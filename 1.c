/*
 * RVC 제어 소프트웨어 - Version 1: 단일 FSM
 * Homework #7 - 구조화 분석 및 설계
 * 
 * 특징:
 * - 5개 상태를 가진 단일 FSM
 * - 장애물 회피 처리 (왼쪽 우선)
 * - 먼지 감지 및 청소
 * - 데드락 회피를 위한 후진 동작
 * - 데드락 방지를 위한 일시정지 상태
 * 
 * 참고 문서:
 * - SA PDF p.11-13: FSM Version 1 상태 정의 및 전이
 * - SRS PDF p.2-4: 요구사항 명세
 * 
 * 주의: 이 파일은 src/ 폴더의 파일들을 자동으로 병합한 것입니다.
 *       수정은 src/ 폴더의 개별 파일에서 하세요.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* ========== 타입 정의 ========== */

// FSM 상태 (SA PDF p.11-12 FSM Version 1 상태 정의)
typedef enum {
    STATE_MOVING,        // SA PDF p.11 "Moving: 정상 전진 및 청소 중"
    STATE_TURNING,       // SA PDF p.11 "Turning: 장애물 회피 회전 중"
    STATE_BACKWARDING,   // SA PDF p.11 "Backwarding: 후진 중"
    STATE_DUST_CLEANING, // SA PDF p.11 "Dust Cleaning: 먼지 집중 청소"
    STATE_PAUSE         // SA PDF p.11 "Pause: 일시 정지"
} SystemState;          // SRS PDF p.3 FR-2.1 "CN1/CN2 별도 FSM"

// 모터 명령 (SRS PDF p.2 "Motor_Cmd -> {Forward,...}")
typedef enum {
    MOTOR_FORWARD,      // SRS PDF p.2 "Motor_Cmd -> {Forward,...}"
    MOTOR_TURN_LEFT,    // SRS PDF p.3 FR-3.2 "좌회전 우선 규칙"
    MOTOR_TURN_RIGHT,   // SRS PDF p.3 FR-3.1 "좌/우 중 가용한 방향"
    MOTOR_BACKWARD,     // SRS PDF p.3 FR-3.3 "Backward를 T_back 시간 수행"
    MOTOR_STOP         // SA PDF p.3 문제점 "Stop 상태 Deadlock"
} MotorCommand;         // SRS PDF p.4 DD "Motor_Cmd"

// 청소기 명령 (SRS PDF p.2 "Clean_Cmd -> {Off, Normal, Boost}")
typedef enum {
    CLEANER_OFF,        // SRS PDF p.2 "Clean_Cmd -> {Off, Normal, Boost}"
    CLEANER_ON,         // SA PDF p.16 CN2 "Normal Cleaning"
    CLEANER_POWERUP     // SA PDF p.16 "Power-Up Cleaning"
} CleanerCommand;       // SRS PDF p.4 DD "Clean_Cmd"

// 회전 방향
typedef enum {
    TURN_LEFT,
    TURN_RIGHT,
    TURN_NONE
} TurnDirection;

// 센서 데이터 구조 (SA PDF p.5 Event List)
typedef struct {
    bool front;         // SRS PDF p.2 "Front_Obs"
    bool left;          // SRS PDF p.2 "Left_Obs"
    bool right;         // SRS PDF p.2 "Right_Obs"
    bool dust;          // SRS PDF p.2 "Dust_Level", p.4 "Dust_Exist"
} SensorData;

// 시스템 컨텍스트
typedef struct {
    SystemState state;
    SensorData sensors;
    MotorCommand motor_cmd;
    CleanerCommand cleaner_cmd;
    int tick_count;         // SRS PDF p.2 "Tick: 제어 주기"
    int state_duration;
    int dust_clean_timer;
    int backward_timer;
} RVCContext;

// 전역 변수
RVCContext rvc;


/* ========== 센서 인터페이스 함수 ========== */




// 실제 하드웨어 센서가 없어 제어 로직 테스트를 위해 랜덤 값 사용
// 20% 확률로 장애물 감지
// 실제 구현 시에는 하드웨어 센서 핀에서 값을 읽어야 함
void read_front_sensor(bool *value) {
    // 시뮬레이션: 랜덤 장애물 감지
    *value = (rand() % 10) < 2; 
}

void read_left_sensor(bool *value) {
    *value = (rand() % 10) < 2;
}

void read_right_sensor(bool *value) {
    *value = (rand() % 10) < 2;
}

void read_dust_sensor(bool *value) {
    *value = (rand() % 10) < 1;  // 10% 확률
}

// 센서 인터페이스 (SA PDF p.7 DFD Level 1 "1.0 Sensor Interface & Preprocessing")
// SA PDF p.18-19 Process Spec 1.0
// SRS PDF p.2 FR-1.1 "Raw 센서값을 필터링"
void sensor_interface(SensorData *sensors) {
    read_front_sensor(&sensors->front);
    read_left_sensor(&sensors->left);
    read_right_sensor(&sensors->right);
    read_dust_sensor(&sensors->dust);
}


/* ========== 제어 로직 함수 ========== */


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


/* ========== 액추에이터 인터페이스 함수 ========== */


// 모터 제어 (SA PDF p.7 "3.0 Actuator Interface")
// SA PDF p.22-23 Process Spec 3.0
// SRS PDF p.2 "Motor: Motor_Cmd -> {Forward, TurnLeft...}"
void motor_control(MotorCommand cmd) {
    const char *direction;
    switch (cmd) {
        case MOTOR_FORWARD:   direction = "MOVE_FORWARD"; break;
        case MOTOR_TURN_LEFT: direction = "TURN_LEFT_45"; break;  // 45도 회전
        case MOTOR_TURN_RIGHT: direction = "TURN_RIGHT_45"; break;
        case MOTOR_BACKWARD:  direction = "MOVE_BACKWARD"; break;
        case MOTOR_STOP:      direction = "STOP"; break;
        default:              direction = "UNKNOWN"; break;
    }
    printf("  [MOTOR] %s\n", direction);
}

// 청소기 제어 (SA PDF p.22-23 Process Spec 3.0 "Actuator Interface")
// SRS PDF p.2 "Cleaner: Clean_Cmd -> {Off, Normal, Boost}"
void cleaner_control(CleanerCommand cmd) {
    const char *clean_cmd;
    switch (cmd) {
        case CLEANER_OFF:     clean_cmd = "VACUUM_OFF"; break;
        case CLEANER_ON:      clean_cmd = "VACUUM_NORMAL"; break;
        case CLEANER_POWERUP: clean_cmd = "VACUUM_TURBO"; break;
        default:              clean_cmd = "UNKNOWN"; break;
    }
    printf("  [CLEANER] %s\n", clean_cmd);
}

void actuator_interface(RVCContext *ctx) {
    motor_control(ctx->motor_cmd);
    cleaner_control(ctx->cleaner_cmd);
}


/* ========== 메인 제어 루프 ========== */





// 전역 변수 정의
RVCContext rvc;

// 시스템 초기화 (SA PDF p.20-21 Process Spec 2.0 "INITIALIZE CN1_State")
void initialize_system() {
    srand(time(NULL));
    rvc.state = STATE_MOVING;
    rvc.tick_count = 0;  // SRS PDF p.2 "Tick: 제어 주기"
    rvc.state_duration = 0;
    rvc.dust_clean_timer = 0;
    rvc.backward_timer = 0;
    rvc.motor_cmd = MOTOR_FORWARD;
    rvc.cleaner_cmd = CLEANER_ON;
    printf("=== RVC Control System V1 (Single FSM) Started ===\n\n");
}

void print_status(RVCContext *ctx) {
    const char *state_names[] = {
        "MOVING", "TURNING", "BACKWARDING", "DUST_CLEANING", "PAUSE"
    };
    
    printf("\n--- Tick %d ---\n", ctx->tick_count);
    printf("State: %s (duration: %d)\n", 
           state_names[ctx->state], ctx->state_duration);
    printf("Sensors: F=%d L=%d R=%d D=%d\n",
           ctx->sensors.front, ctx->sensors.left, 
           ctx->sensors.right, ctx->sensors.dust);
}

// 메인 함수 (SA PDF p.6 "RVC Control (0)" 전체 시스템)
// SRS PDF p.3-4 "P-1 제어주기: 50–100 ms"
int main(void) {
    initialize_system();
    
    // 시뮬레이션 루프: 50 ticks
    for (int i = 0; i < 50; i++) {
        rvc.tick_count = i;
        
        // 1. 센서 인터페이스 (SA PDF p.18-19 Process 1.0)
        sensor_interface(&rvc.sensors);
        
        // 2. 제어 로직 (FSM) (SA PDF p.20-21 Process 2.0)
        fsm_executor(&rvc);
        
        // 3. 액추에이터 인터페이스 (SA PDF p.22-23 Process 3.0)
        actuator_interface(&rvc);
        
        // 4. 상태 표시
        print_status(&rvc);
        
        // Tick 지연 시뮬레이션
        #ifndef _WIN32
        usleep(200000);  // 200ms - SRS PDF p.3 "P-2 반응시간 ≤ 150 ms"
        #endif
    }
    
    printf("\n=== Simulation Complete ===\n");
    return 0;
}


