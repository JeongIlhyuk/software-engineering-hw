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

// 전역 변수
extern RVCSystem rvc;

