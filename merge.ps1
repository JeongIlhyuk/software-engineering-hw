# 파일들을 하나로 합치는 스크립트
# Git hook에서 사용

$header = @"
/*
 * RVC 제어 소프트웨어 - Version 1: 단일 FSM
 * Homework #7 - 구조화 분석 및 설계
 * 
 * 특징:
 * - 5개 상태를 가진 단일 FSM
 * - 장애물 회피 처리 (왼쪽 우선)
 * - 먼지 감지 및 청소
 * - 데드락 회피를 위한 후진 동작
 * - 데드락 방지를 위한 일시정지 상태
 * 
 * 참고 문서:
 * - SA PDF p.11-13: FSM Version 1 상태 정의 및 전이
 * - SRS PDF p.2-4: 요구사항 명세
 * 
 * 주의: 이 파일은 src/ 폴더의 파일들을 자동으로 병합한 것입니다.
 *       수정은 src/ 폴더의 개별 파일에서 하세요.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

"@

$outputFile = "1.c"
$header | Out-File -FilePath $outputFile -Encoding UTF8

$typesContent = Get-Content "src\types.h" -Raw
$typesContent = $typesContent -replace 'extern RVCContext rvc;', 'RVCContext rvc;'
$typesContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$sensorsContent = Get-Content "src\sensors.c" -Raw
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+<stdlib.h>\s*$', ''
$sensorsContent = $sensorsContent -replace '(?m)^#include\s+<stdbool.h>\s*$', ''
$sensorsContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$fsmContent = Get-Content "src\fsm.c" -Raw
$fsmContent = $fsmContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$fsmContent = $fsmContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$fsmContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$actuatorsContent = Get-Content "src\actuators.c" -Raw
$actuatorsContent = $actuatorsContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$actuatorsContent = $actuatorsContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$actuatorsContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

$mainContent = Get-Content "src\main.c" -Raw
$mainContent = $mainContent -replace '(?m)^#include\s+"types.h"\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<stdio.h>\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<stdlib.h>\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<stdbool.h>\s*$', ''
$mainContent = $mainContent -replace '(?m)^#include\s+<time.h>\s*$', ''
$mainContent = $mainContent -replace '(?s)// 함수 선언.*?void actuator_interface\(RVCContext \*ctx\);\s*\r?\n', ''
$mainContent | Out-File -FilePath $outputFile -Append -Encoding UTF8

