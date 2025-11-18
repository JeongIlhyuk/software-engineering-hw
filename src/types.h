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
extern RVCContext rvc;

