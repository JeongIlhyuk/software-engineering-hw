/* ========== 제어 로직 함수 ========== */

#include <stdio.h>
#include "types.h"

// 모든 방향 막힘 확인 (SA PDF p.10 DFD Level 4 "2.1.2.3 All Blocked Handler")
// SRS PDF p.3 FR-3.3 "좌/우 모두 불가 시"
bool all_blocked(SensorData *sensors) {
    return sensors->front && sensors->left && sensors->right;
}

// 회전 우선순위 결정 (SA PDF p.31 Process Spec 2.1.2.2 "Turning Priority Decision")
// SRS PDF p.3 FR-3.2 "Left 우선 규칙을 적용"
// SRS PDF p.3 "좌/우 모두 가용 시 Left 우선"
TurnDirection decide_turn_priority(SensorData *sensors) {
    // 왼쪽 우선 정책
    if (!sensors->left) {//좌측에 장애물이 없으면
        return TURN_LEFT;//좌측으로 회전
    } else if (!sensors->right) {//우측에 장애물이 없으면
        return TURN_RIGHT;//우측으로 회전
    } else {//좌측과 우측에 장애물이 모두 있으면
        return TURN_NONE;//회전하지 않음
    }
}

// FSM 실행기 (SA PDF p.7 "2.0 Control Logic & Command Generation")
// SA PDF p.12 "FSM Version 1: 상태 전이도"
// SRS PDF p.3 "3.3 상태기계 요구사항"
void fsm_executor(RVCContext *ctx) {
    ctx->state_duration++;//현재 상태의 tick 수
    
    switch (ctx->state) {
        case STATE_MOVING:  // SA PDF p.11 "Moving: 정상 전진 및 청소 중"
            // SA PDF p.13 상태 전이 테이블 "Moving" 행
            ctx->motor_cmd = MOTOR_FORWARD;
            ctx->cleaner_cmd = CLEANER_ON;
            
            // 상태 전이 조건
            if (ctx->sensors.dust) {
                // SRS PDF p.3 FR-5.1 "Dust_Exist 시 Boost 모드"
                ctx->state = STATE_DUST_CLEANING;
                ctx->dust_clean_timer = 5;  // 먼지 청소 상태를 5 tick 동안 유지
                ctx->state_duration = 0; // 상태의 tick 수를 0으로 리셋
                printf("[FSM] MOVING -> DUST_CLEANING (dust detected)\n");
            } 
            else if (ctx->sensors.front) {
                // SA PDF p.13 "Moving → Turning (Front Obstacle)"
                ctx->state = STATE_TURNING;
                ctx->state_duration = 0;
                printf("[FSM] MOVING -> TURNING (front obstacle)\n");
            }
            break;
            
        case STATE_TURNING:  // SA PDF p.11 "Turning: 장애물 회피 회전 중"
            ctx->cleaner_cmd = CLEANER_ON;
            
            if (all_blocked(&ctx->sensors)) {
                // SA PDF p.13 "Turning → Backwarding (All Blocked)"
                // SRS PDF p.3 FR-3.3 "좌/우 모두 불가 시 Backward"
                ctx->state = STATE_BACKWARDING;
                ctx->backward_timer = 3;  // 후진 상태를 3 tick 동안 유지
                ctx->state_duration = 0;
                printf("[FSM] TURNING -> BACKWARDING (all blocked)\n");
            } 
            else {
                TurnDirection turn = decide_turn_priority(&ctx->sensors);
                // SA PDF p.31 "2.1.2.2 Turning Priority Decision"
                if (turn == TURN_LEFT) {
                    ctx->motor_cmd = MOTOR_TURN_LEFT;
                    printf("[FSM] Turning LEFT (priority)\n");
                } else if (turn == TURN_RIGHT) {
                    ctx->motor_cmd = MOTOR_TURN_RIGHT;
                    printf("[FSM] Turning RIGHT\n");
                } else {
                    ctx->state = STATE_PAUSE;
                    // SA PDF p.11 "Pause: 일시 정지 (탈출 대기)"
                    ctx->state_duration = 0; // 현재 상태에서 경과한 tick 수
                    printf("[FSM] TURNING -> PAUSE (no turn available)\n");
                }
                
                // 회전 완료 후 이동 상태로 복귀
                if (ctx->state_duration >= 2 && turn != TURN_NONE) {
                    ctx->state = STATE_MOVING;
                    ctx->state_duration = 0;
                    printf("[FSM] TURNING -> MOVING (turn complete)\n");
                }
            }
            break;
            
        case STATE_BACKWARDING:  // SA PDF p.11 "Backwarding: 후진 중 (전방향 막힘)"
            // SRS PDF p.3 FR-3.3 "Backward를 T_back 시간 수행"
            // SRS PDF p.5 "T_back=600 ms"
            ctx->motor_cmd = MOTOR_BACKWARD;
            ctx->cleaner_cmd = CLEANER_ON;
            ctx->backward_timer--;
            
            if (ctx->backward_timer <= 0) {
                ctx->state = STATE_TURNING;
                ctx->state_duration = 0;
                printf("[FSM] BACKWARDING -> TURNING (escape)\n");
            }
            break;
            
        case STATE_DUST_CLEANING:  // SA PDF p.11 "Dust Cleaning: 먼지 집중 청소 (정지)"
            // SRS PDF p.3 FR-5.2 "일정 시간/영역 청소 후 Normal 복귀"
            ctx->motor_cmd = MOTOR_STOP;
            ctx->cleaner_cmd = CLEANER_POWERUP;
            ctx->dust_clean_timer--;
            
            if (ctx->dust_clean_timer <= 0) {
                ctx->state = STATE_MOVING;
                ctx->state_duration = 0;// 현재 상태에서 경과한 tick 수를 0으로 리셋
                printf("[FSM] DUST_CLEANING -> MOVING (clean complete)\n");
            }
            break;
            
        case STATE_PAUSE:  // SA PDF p.3 "Stop 상태 Deadlock → Pause 상태로 분리"
            ctx->motor_cmd = MOTOR_STOP;
            ctx->cleaner_cmd = CLEANER_ON;
            
            // 데드락 탈출: 일시정지 후 후진 시도
            // SRS PDF p.3 FR-4.1 "Deadlock_Suspect로 표식"
            // SRS PDF p.3 FR-4.2 "Backward→Turn→Forward 시퀀스"
            if (ctx->state_duration >= 3) {
                ctx->state = STATE_BACKWARDING;
                ctx->backward_timer = 3;  // 후진 상태를 3 tick 동안 유지
                ctx->state_duration = 0;// 현재 상태에서 경과한 tick 수를 0으로 리셋
                printf("[FSM] PAUSE -> BACKWARDING (deadlock escape)\n");
            }
            break;
    }
}

