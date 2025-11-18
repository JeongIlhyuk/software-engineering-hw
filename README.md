# RVC 제어 소프트웨어

이 프로젝트는 **모듈화된 개발**과 **단일 파일 제출**을 모두 지원합니다.

## 프로젝트 구조

```
프로젝트/
├── src/              # Version 1 개발용 모듈 파일들
│   ├── types.h       # 타입 정의
│   ├── sensors.c     # 센서 인터페이스
│   ├── fsm.c         # FSM 제어 로직
│   ├── actuators.c   # 액추에이터 인터페이스
│   └── main.c        # 메인 함수
├── src2/             # Version 2 개발용 모듈 파일들
│   ├── types.h       # 타입 정의
│   ├── sensors.c     # 센서 인터페이스
│   ├── cn1_fsm.c     # CN1 모터 FSM
│   ├── cn2_fsm.c     # CN2 청소기 FSM
│   ├── control.c     # 제어 로직 조율
│   ├── actuators.c   # 액추에이터 인터페이스
│   └── main.c        # 메인 함수
├── 1.c               # Version 1 제출용 단일 파일 (자동 생성)
└── 2.c               # Version 2 제출용 단일 파일 (자동 생성)
```

---

## 사용 방법

### 개발 시

`src/` 또는 `src2/` 폴더의 개별 파일을 수정하세요.

**Version 1 (src/):**
- `src/types.h` - 타입 정의
- `src/sensors.c` - 센서 관련 코드
- `src/fsm.c` - FSM 로직
- `src/actuators.c` - 액추에이터 제어
- `src/main.c` - 메인 함수

**Version 2 (src2/):**
- `src2/types.h` - 타입 정의
- `src2/sensors.c` - 센서 관련 코드
- `src2/cn1_fsm.c` - CN1 모터 FSM
- `src2/cn2_fsm.c` - CN2 청소기 FSM
- `src2/control.c` - 제어 로직 조율
- `src2/actuators.c` - 액추에이터 제어
- `src2/main.c` - 메인 함수

### 제출용 파일 생성

제출하기 직전에 "1.c 반영해줘" 또는 "2.c 반영해줘"라고 요청하시면, 
분할된 파일들을 자동으로 합쳐서 `1.c` 또는 `2.c`를 생성해드립니다.

### 컴파일 및 실행

```powershell
$env:Path += ";C:\msys64\mingw64\bin"
gcc 1.c -o 1.exe
.\1.exe

# 또는 Version 2
gcc 2.c -o 2.exe
.\2.exe
```

## 워크플로우

1. **개발**: `src/` 또는 `src2/` 폴더의 개별 파일에서 작업
2. **제출용 파일 생성**: "1.c 반영해줘" 또는 "2.c 반영해줘" 요청
3. **테스트**: `1.c` 또는 `2.c` 컴파일 및 실행
4. **제출**: `1.c` 또는 `2.c` 파일 제출

## 주의사항

- **`1.c`와 `2.c`는 자동 생성된 파일입니다.** 직접 수정하지 마세요!
- 모든 수정은 `src/` 또는 `src2/` 폴더의 개별 파일에서 하세요.
- 제출용 파일 생성 시 해당 파일이 덮어씌워집니다.

## 파일 설명

### Version 1 (src/)

#### src/types.h
- 모든 타입 정의 (enum, struct)
- 전역 변수 선언

#### src/sensors.c
- 센서 읽기 함수
- 센서 인터페이스

#### src/fsm.c
- FSM 실행기
- 상태 전이 로직
- 회전 우선순위 결정

#### src/actuators.c
- 모터 제어
- 청소기 제어
- 액추에이터 인터페이스

#### src/main.c
- 메인 함수
- 시스템 초기화
- 제어 루프

### Version 2 (src2/)

#### src2/types.h
- 모든 타입 정의 (CN1, CN2 상태 및 컨텍스트)
- 전역 변수 선언

#### src2/sensors.c
- 센서 읽기 함수
- 센서 인터페이스

#### src2/cn1_fsm.c
- CN1 모터 FSM 실행기
- 모터 상태 전이 로직
- 회전 우선순위 결정

#### src2/cn2_fsm.c
- CN2 청소기 FSM 실행기
- 청소기 상태 전이 로직

#### src2/control.c
- CN1과 CN2 간 제어 로직 조율
- Cleaner_Trigger 및 Motor_Status 관리

#### src2/actuators.c
- 모터 제어
- 청소기 제어
- 액추에이터 인터페이스

#### src2/main.c
- 메인 함수
- 시스템 초기화
- 제어 루프

