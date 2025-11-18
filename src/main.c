/* ========== 메인 제어 루프 ========== */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "types.h"

// 전역 변수 정의
RVCContext rvc;

// 함수 선언
void sensor_interface(SensorData *sensors);
void fsm_executor(RVCContext *ctx);
void actuator_interface(RVCContext *ctx);

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

