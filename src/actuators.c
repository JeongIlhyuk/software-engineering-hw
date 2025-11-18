/* ========== 액추에이터 인터페이스 함수 ========== */

#include <stdio.h>
#include "types.h"

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

