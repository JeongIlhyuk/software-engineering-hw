/* ========== 센서 인터페이스 함수 ========== */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "types.h"

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

