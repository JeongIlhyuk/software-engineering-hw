/* ========== 센서 인터페이스 함수 ========== */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "types.h"

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

