/* ========== 제어 로직 조율 ========== */

#include "types.h"

// 함수 선언
void cn1_motor_fsm(CN1_Context *cn1, SensorData *sensors, bool cleaner_trigger);
void cn2_cleaner_fsm(CN2_Context *cn2, bool dust_detected, bool motor_moving);

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

