/* ========== 메인 제어 루프 ========== */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "types.h"

// 전역 변수 정의
RVCSystem rvc;

// 함수 선언
void sensor_interface(SensorData *sensors);
void control_logic(RVCSystem *sys);
void actuator_interface(RVCSystem *sys);

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

