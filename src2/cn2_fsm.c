/* ========== CN2: 청소기 제어 FSM ========== */

#include <stdio.h>
#include "types.h"

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

