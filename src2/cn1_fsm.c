/* ========== CN1: 모터 제어 FSM ========== */

#include <stdio.h>
#include "types.h"

// 모든 방향 막힘 확인
bool all_blocked(SensorData *sensors) {
    return sensors->front && sensors->left && sensors->right;
}

// 회전 우선순위 결정
// SRS PDF p.3 FR-3.2 "좌/우 모두 가용 시 Left 우선"
TurnDirection decide_turn_priority(SensorData *sensors) {
    // 왼쪽 우선 정책
    if (!sensors->left) {
        return TURN_LEFT;
    } else if (!sensors->right) {
        return TURN_RIGHT;
    } else {
        return TURN_NONE;
    }
}

// CN1 모터 FSM (SA PDF p.24-25 Process Spec 2.1 "Motor State Management (CN1)")
// SRS PDF p.3 "3.3.1 CN1: Motor Control FSM"
void cn1_motor_fsm(CN1_Context *cn1, SensorData *sensors, bool cleaner_trigger) {
    cn1->state_duration++;
    cn1->cleaner_trigger_received = cleaner_trigger;
    
    switch (cn1->state) {
        case MOTOR_IDLE:  // SA PDF p.15 "Idle → Moving (시작)"
            cn1->command = CMD_STOP;
            // 자동 시작 (시뮬레이션)
            if (cn1->state_duration >= 2) {
                cn1->state = MOTOR_MOVING;
                cn1->state_duration = 0;
                printf("[CN1] IDLE -> MOVING (start)\n");
            }
            break;
            
        case MOTOR_MOVING:  // SA PDF p.15 "Moving → Turning (장애물)"
            cn1->command = CMD_FORWARD;
            
            // 우선순위 1: 청소기 트리거 (먼지 청소를 위한 일시정지)
            // SRS PDF p.3 FR-2.2 "Cleaner_Trigger를 CN1에 전달"
            // SRS PDF p.3 FR-2.3 "Trigger 수신 시 Pause 상태로 전이"
            if (cleaner_trigger) {
                cn1->state = MOTOR_PAUSED;
                cn1->state_duration = 0;
                printf("[CN1] MOVING -> PAUSED (cleaner trigger)\n");
            }
            // 우선순위 2: 장애물 회피
            else if (sensors->front) {
                cn1->state = MOTOR_TURNING;
                cn1->state_duration = 0;
                printf("[CN1] MOVING -> TURNING (front obstacle)\n");
            }
            break;
            
        case MOTOR_TURNING:  // SA PDF p.9 "2.1.2 Collision Avoidance Logic"
            // SA PDF p.31 "2.1.2.2 Turning Priority Decision"
            if (all_blocked(sensors)) {
                // SA PDF p.15 "Turning → Backwarding (전방향 막힘)"
                cn1->state = MOTOR_BACKWARDING;
                cn1->backward_timer = 3;  // 3 ticks
                cn1->state_duration = 0;
                printf("[CN1] TURNING -> BACKWARDING (all blocked)\n");
            } 
            else {
                TurnDirection turn = decide_turn_priority(sensors);
                // SRS PDF p.3 FR-3.2 "좌/우 모두 가용 시 Left 우선"
                if (turn == TURN_LEFT) {
                    cn1->command = CMD_TURN_LEFT;
                    printf("[CN1] Executing TURN_LEFT\n");
                } else if (turn == TURN_RIGHT) {
                    cn1->command = CMD_TURN_RIGHT;
                    printf("[CN1] Executing TURN_RIGHT\n");
                } else {
                    cn1->state = MOTOR_PAUSED;
                    cn1->state_duration = 0;
                    printf("[CN1] TURNING -> PAUSED (no path)\n");
                }
                
                // 회전 완료
                if (cn1->state_duration >= 2 && turn != TURN_NONE) {
                    cn1->state = MOTOR_MOVING;
                    cn1->state_duration = 0;
                    printf("[CN1] TURNING -> MOVING (turn complete)\n");
                }
            }
            break;
            
        case MOTOR_BACKWARDING:  // SA PDF p.15 CN1 "Backwarding"
            // SRS PDF p.5 "T_back=600 ms"
            cn1->command = CMD_BACKWARD;
            cn1->backward_timer--;
            
            if (cn1->backward_timer <= 0) {
                cn1->state = MOTOR_TURNING;
                cn1->state_duration = 0;
                printf("[CN1] BACKWARDING -> TURNING (escape)\n");
            }
            break;
            
        case MOTOR_PAUSED:  // SA PDF p.15 "Paused ← Cleaner_Trigger (CN2 요청)"
            // SRS PDF p.3 FR-2.3 "다음 Tick에서 Pause 상태로 전이"
            // SRS PDF p.5 "Pause: 안전 정지(Stop과 달리 Deadlock 회피)"
            cn1->command = CMD_STOP;
            
            // 청소 완료 시 재개 또는 데드락 탈출 타임아웃
            if (!cleaner_trigger && cn1->state_duration >= 1) {
                cn1->state = MOTOR_MOVING;
                cn1->state_duration = 0;
                printf("[CN1] PAUSED -> MOVING (resume)\n");
            }
            // 긴 일시정지 후 데드락 탈출
            // SRS PDF p.3 FR-4.2 "N번 실패 시 사용자 알림"
            else if (cn1->state_duration >= 5) {
                cn1->state = MOTOR_BACKWARDING;
                cn1->backward_timer = 3;
                cn1->state_duration = 0;
                printf("[CN1] PAUSED -> BACKWARDING (deadlock escape)\n");
            }
            break;
    }
}

