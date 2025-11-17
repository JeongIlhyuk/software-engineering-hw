/*
 * RVC 제어 소프트웨어 - Version 2: 이중 FSM (CN1 + CN2)
 * Homework #7 - 구조화 분석 및 설계
 * 
 * 특징:
 * - CN1 (모터 FSM): Idle, Moving, Turning, Backwarding, Paused
 * - CN2 (청소기 FSM): Off, Normal_Cleaning, PowerUp_Cleaning
 * - Cleaner_Trigger와 Motor_Status를 통한 FSM 간 통신
 * - 향상된 모듈성 및 유지보수성
 * 
 * 참고 문서:
 * - SA PDF p.14-16: FSM Version 2 (CN1/CN2 분리 구조)
 * - SRS PDF p.3 FR-2.1: "CN1(이동)과 CN2(청소) 별도 FSM"
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* ========== 타입 정의 ========== */

// CN1: 모터 FSM 상태 (SA PDF p.15 CN1)
// SRS PDF p.3 FR-2.1 "CN1(이동)과 CN2(청소) 별도 FSM"
typedef enum {
    MOTOR_IDLE,         // SA PDF p.15 CN1 "Idle"
    MOTOR_MOVING,       // SA PDF p.15 "Moving"
    MOTOR_TURNING,      // SA PDF p.15 "Turning"
    MOTOR_BACKWARDING,  // SA PDF p.15 "Backwarding"
    MOTOR_PAUSED        // SA PDF p.15 "Paused"
} MotorState;

// CN2: 청소기 FSM 상태 (SA PDF p.16 CN2)
// SA PDF p.14 "CN2: Cleaner Control FSM"
typedef enum {
    CLEANER_OFF,        // SA PDF p.16 CN2 "Off"
    CLEANER_NORMAL,     // SA PDF p.16 "Normal Cleaning"
    CLEANER_POWERUP     // SA PDF p.16 "Power-Up Cleaning"
} CleanerState;

// Motor Commands
typedef enum {
    CMD_FORWARD,
    CMD_TURN_LEFT,
    CMD_TURN_RIGHT,
    CMD_BACKWARD,
    CMD_STOP
} MotorCommand;

// Cleaner Commands
typedef enum {
    CMD_OFF,
    CMD_NORMAL,
    CMD_TURBO
} CleanerCommand;

// Turn Direction
typedef enum {
    TURN_LEFT,
    TURN_RIGHT,
    TURN_NONE
} TurnDirection;

// 센서 데이터
typedef struct {
    bool front;
    bool left;
    bool right;
    bool dust;
} SensorData;

// CN1 컨텍스트 (SA PDF p.8 "2.1 Motor State Management (CN1)")
typedef struct {
    MotorState state;
    MotorCommand command;
    int state_duration;
    int backward_timer;
    bool cleaner_trigger_received;  // SRS PDF p.3 FR-2.2 "Cleaner_Trigger"
} CN1_Context;

// CN2 컨텍스트 (SA PDF p.8 "2.2 Cleaner State Management (CN2)")
typedef struct {
    CleanerState state;
    CleanerCommand command;
    int powerup_timer;
    bool motor_is_moving;  // SRS PDF p.4 DD "Motor_Status"
} CN2_Context;

// 시스템 컨텍스트 (SRS PDF p.2 FR-2 "제어노드 구조(CN1/CN2)")
typedef struct {
    CN1_Context cn1;
    CN2_Context cn2;
    SensorData sensors;
    int tick_count;         // SRS PDF p.2 "Tick: 제어 주기"
    bool cleaner_trigger;    // SA PDF p.8 "CN2 → CN1: Cleaner_Trigger"
    bool motor_status_moving; // SA PDF p.8 "CN1 → CN2: Motor_Status"
} RVCSystem;

/* ========== 전역 변수 ========== */
RVCSystem rvc;

/* ========== 센서 인터페이스 함수 ========== */

void read_front_sensor(bool *value) {
    *value = (rand() % 10) < 2;  // 20% 장애물 확률
}

void read_left_sensor(bool *value) {
    *value = (rand() % 10) < 2;
}

void read_right_sensor(bool *value) {
    *value = (rand() % 10) < 2;
}

void read_dust_sensor(bool *value) {
    *value = (rand() % 10) < 1;  // 10% 먼지 확률
}

// 센서 인터페이스 (SA PDF p.7 "1.0 Sensor Interface & Preprocessing")
void sensor_interface(SensorData *sensors) {
    read_front_sensor(&sensors->front);
    read_left_sensor(&sensors->left);
    read_right_sensor(&sensors->right);
    read_dust_sensor(&sensors->dust);
}

/* ========== CN1: 모터 제어 FSM ========== */

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

/* ========== CN2: 청소기 제어 FSM ========== */

// CN2 청소기 FSM (SA PDF p.26-27 Process Spec 2.2 "Cleaner State Management (CN2)")
// SRS PDF p.3 "3.3.2 CN2: Cleaner Control FSM"
void cn2_cleaner_fsm(CN2_Context *cn2, bool dust_detected, bool motor_moving) {
    cn2->motor_is_moving = motor_moving;
    
    switch (cn2->state) {
        case CLEANER_OFF:  // SA PDF p.16 "Off → Normal Cleaning (시작)"
            cn2->command = CMD_OFF;
            // 시스템과 함께 자동 시작
            cn2->state = CLEANER_NORMAL;
            printf("[CN2] OFF -> NORMAL (start)\n");
            break;
            
        case CLEANER_NORMAL:  // SA PDF p.16 "Normal → Power-Up (먼지 감지)"
            cn2->command = CMD_NORMAL;
            
            // 이동 중 먼지 감지 → 일시정지 요청 및 파워업
            // SRS PDF p.3 FR-5.1 "Dust_Exist 시 Boost 또는 Spot"
            if (dust_detected && motor_moving) {
                cn2->state = CLEANER_POWERUP;
                cn2->powerup_timer = 5;  // 5 ticks 집중 청소
                printf("[CN2] NORMAL -> POWERUP (dust detected)\n");
            }
            break;
            
        case CLEANER_POWERUP:  // SA PDF p.16 "Power-Up → Normal (청소 완료)"
            // SRS PDF p.3 FR-5.2 "일정 시간/영역 청소 후 Normal 복귀"
            cn2->command = CMD_TURBO;
            cn2->powerup_timer--;
            
            if (cn2->powerup_timer <= 0) {
                cn2->state = CLEANER_NORMAL;
                printf("[CN2] POWERUP -> NORMAL (clean complete)\n");
            }
            break;
    }
}

/* ========== 제어 로직 조율 ========== */

// 제어 로직 (SA PDF p.8 "CN 간 상호작용")
// SRS PDF p.3 FR-2.2 "상호 인터페이스는 Cleaner_Trigger와 Motor_Status"
void control_logic(RVCSystem *sys) {
    // 청소기 트리거 신호 결정
    sys->cleaner_trigger = (sys->cn2.state == CLEANER_POWERUP);
    // SA PDF p.16 "Normal → Power-Up → Paused Trigger to CN1"
    
    // 모터 상태 결정
    sys->motor_status_moving = (sys->cn1.state == MOTOR_MOVING);
    // SRS PDF p.4 DD "Motor_Status"
    
    // CN1 (모터 FSM) 실행 (SA PDF p.24-25 Process 2.1)
    cn1_motor_fsm(&sys->cn1, &sys->sensors, sys->cleaner_trigger);
    
    // CN2 (청소기 FSM) 실행 (SA PDF p.26-27 Process 2.2)
    cn2_cleaner_fsm(&sys->cn2, sys->sensors.dust, sys->motor_status_moving);
}

/* ========== 액추에이터 인터페이스 함수 ========== */

// 모터 제어 (SA PDF p.7 "3.0 Actuator Interface")
void motor_control(MotorCommand cmd) {
    const char *direction;
    switch (cmd) {
        case CMD_FORWARD:   direction = "MOVE_FORWARD"; break;
        case CMD_TURN_LEFT: direction = "TURN_LEFT_45"; break;
        case CMD_TURN_RIGHT: direction = "TURN_RIGHT_45"; break;
        case CMD_BACKWARD:  direction = "MOVE_BACKWARD"; break;
        case CMD_STOP:      direction = "STOP"; break;
        default:            direction = "UNKNOWN"; break;
    }
    printf("  [MOTOR] %s\n", direction);
}

// 청소기 제어 (SA PDF p.7 "3.0 Actuator Interface")
void cleaner_control(CleanerCommand cmd) {
    const char *clean_cmd;
    switch (cmd) {
        case CMD_OFF:    clean_cmd = "VACUUM_OFF"; break;
        case CMD_NORMAL: clean_cmd = "VACUUM_NORMAL"; break;
        case CMD_TURBO:  clean_cmd = "VACUUM_TURBO"; break;
        default:         clean_cmd = "UNKNOWN"; break;
    }
    printf("  [CLEANER] %s\n", clean_cmd);
}

void actuator_interface(RVCSystem *sys) {
    motor_control(sys->cn1.command);
    cleaner_control(sys->cn2.command);
}

/* ========== 메인 제어 루프 ========== */

// 시스템 초기화 (SA PDF p.20 "INITIALIZE CN1_State := Idle, CN2_State := Off")
void initialize_system() {
    srand(time(NULL));
    
    // CN1 초기화 (SA PDF p.15 CN1 초기 상태)
    rvc.cn1.state = MOTOR_IDLE;
    rvc.cn1.command = CMD_STOP;
    rvc.cn1.state_duration = 0;
    rvc.cn1.backward_timer = 0;
    rvc.cn1.cleaner_trigger_received = false;
    
    // CN2 초기화 (SA PDF p.16 CN2 초기 상태)
    rvc.cn2.state = CLEANER_OFF;
    rvc.cn2.command = CMD_OFF;
    rvc.cn2.powerup_timer = 0;
    rvc.cn2.motor_is_moving = false;
    
    // 시스템
    rvc.tick_count = 0;  // SRS PDF p.2 "Tick: 제어 주기"
    rvc.cleaner_trigger = false;
    rvc.motor_status_moving = false;
    
    printf("=== RVC Control System V2 (Dual FSM: CN1+CN2) Started ===\n\n");
}

// 상태 출력 (SA PDF p.36 "제어 흐름 (Control Flows)")
// SA PDF p.8 "Cleaner_Trigger ↔ Motor_Status (상호작용)"
void print_status(RVCSystem *sys) {
    const char *motor_states[] = {
        "IDLE", "MOVING", "TURNING", "BACKWARDING", "PAUSED"
    };
    const char *cleaner_states[] = {
        "OFF", "NORMAL", "POWERUP"
    };
    
    printf("\n--- Tick %d ---\n", sys->tick_count);
    printf("CN1 State: %s (duration: %d)\n", 
           motor_states[sys->cn1.state], sys->cn1.state_duration);
    printf("CN2 State: %s\n", cleaner_states[sys->cn2.state]);
    printf("Sensors: F=%d L=%d R=%d D=%d\n",
           sys->sensors.front, sys->sensors.left, 
           sys->sensors.right, sys->sensors.dust);
    printf("Trigger: Cleaner->Motor=%d, Motor->Cleaner=%d\n",
           sys->cleaner_trigger, sys->motor_status_moving);
}

// 메인 함수 (SA PDF p.6 DFD Level 0 "RVC Control (0)")
int main(void) {
    initialize_system();
    
    // 시뮬레이션 루프: 50 ticks
    for (int i = 0; i < 50; i++) {
        rvc.tick_count = i;
        
        // 1. 센서 인터페이스 (SA PDF p.7 "1.0 Sensor Interface & Preprocessing")
        sensor_interface(&rvc.sensors);
        
        // 2. 제어 로직 (CN1 + CN2) (SA PDF p.8 "2.0 Control Logic (2개 CN)")
        control_logic(&rvc);
        
        // 3. 액추에이터 인터페이스 (SA PDF p.7 "3.0 Actuator Interface")
        actuator_interface(&rvc);
        
        // 4. 상태 표시
        print_status(&rvc);
        
        // Tick 지연 시뮬레이션
        #ifndef _WIN32
        usleep(200000);  // 200ms
        #endif
    }
    
    printf("\n=== Simulation Complete ===\n");
    // SA PDF p.38 "문제점 해결 검증"
    printf("\nVersion 2 Benefits:\n");
    // SA PDF p.14 "설계 개선 목표: 일관성 및 유지보수성 향상"
    printf("- Better separation of concerns (Motor vs Cleaner)\n");
    printf("- Easier to maintain and extend\n");
    printf("- Clear inter-module communication\n");
    // SA PDF p.38 "Interface 불일치 → 일관성 확보"
    printf("- No interface inconsistency\n");
    
    return 0;
}
