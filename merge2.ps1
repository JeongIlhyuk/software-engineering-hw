# 파일들을 하나로 합치는 스크립트 (Version 2)
# Git hook에서 사용

$header = @"
/*
 * RVC 제어 소프트웨어 - Version 2: 이중 FSM (CN1 + CN2)
 * Homework #7 - 구조화 분석 및 설계
 * 
 * 특징:
 * - CN1 (모터 FSM): Idle, Moving, Turning, Backwarding, Paused
 * - CN2 (청소기 FSM): Off, Normal_Cleaning, PowerUp_Cleaning
 * - Cleaner_Trigger와 Motor_Status를 통한 FSM 간 통신
 * - 향상된 모듈성 및 유지보수성
 * 
 * 참고 문서:
 * - SA PDF p.14-16: FSM Version 2 (CN1/CN2 분리 구조)
 * - SRS PDF p.3 FR-2.1: "CN1(이동)과 CN2(청소) 별도 FSM"
 * 
 * 주의: 이 파일은 src2/ 폴더의 파일들을 자동으로 병합한 것입니다.
 *       수정은 src2/ 폴더의 개별 파일에서 하세요.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

"@

$outputFile = "2.c"
$header | Out-File -FilePath $outputFile -Encoding UTF8

$typesContent = Get-Content "src2\types.h" -Raw
$typesContent = $typesContent -replace 'extern RVCSystem rvc;', 'RVCSystem rvc;'
$typesContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$sensorsContent = Get-Content "src2\sensors.c" -Raw
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+<stdlib.h>\s*$', ''
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+<stdbool.h>\s*$', ''
$sensorsContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$cn1Content = Get-Content "src2\cn1_fsm.c" -Raw
$cn1Content = $cn1Content -replace '(?m)^#include\s+"types.h"\s*$', ''
$cn1Content = $cn1Content -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$cn1Content | Out-File -FilePath $outputFile -Append -Encoding UTF8

$cn2Content = Get-Content "src2\cn2_fsm.c" -Raw
$cn2Content = $cn2Content -replace '(?m)^#include\s+"types.h"\s*$', ''
$cn2Content = $cn2Content -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$cn2Content | Out-File -FilePath $outputFile -Append -Encoding UTF8

$controlContent = Get-Content "src2\control.c" -Raw
$controlContent = $controlContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$controlContent = $controlContent -replace '(?s)// 함수 선언.*?void cn2_cleaner_fsm\(CN2_Context \*cn2, bool dust_detected, bool motor_moving\);\s*\r?\n', ''
$controlContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$actuatorsContent = Get-Content "src2\actuators.c" -Raw
$actuatorsContent = $actuatorsContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$actuatorsContent = $actuatorsContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$actuatorsContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$mainContent = Get-Content "src2\main.c" -Raw
$mainContent = $mainContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<stdlib.h>\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<stdbool.h>\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<time.h>\s*$', ''
$mainContent = $mainContent -replace '(?s)// 함수 선언.*?void actuator_interface\(RVCSystem \*sys\);\s*\r?\n', ''
$mainContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

