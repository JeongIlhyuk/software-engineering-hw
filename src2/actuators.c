/* ========== 액추에이터 인터페이스 함수 ========== */

#include <stdio.h>
#include "types.h"

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

