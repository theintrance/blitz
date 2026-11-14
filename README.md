# blitz

## 요구 사항

- CMake 3.25 이상
- C++20을 지원하는 GCC 또는 Clang (macOS는 Apple Clang)
- 첫 테스트 빌드 때 GoogleTest를 내려받으므로 네트워크 연결

## 빌드와 테스트

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

| 프리셋 | 빌드 종류 | sanitizer | 용도 |
| --- | --- | --- | --- |
| `debug` | Debug | ASan + UBSan | 개발·테스트 |
| `bench` | RelWithDebInfo (`-O2 -g`), `-fno-omit-frame-pointer` | 끔 | 측정 전용 |

빌드 결과는 `build/<프리셋>/`, 실행 파일은 `build/<프리셋>/bin/`에 생깁니다.

## 벤치마크 실행

```bash
cmake --preset bench
cmake --build --preset bench
./build/bench/bin/bench_selftest
```

리눅스 서버에서는 코어를 고정해 실행합니다.

```bash
taskset -c 2 ./build/bench/bin/bench_selftest
```

- 측정 도구 사용법, 백분위수 정의, 타이머 오버헤드, 한계는 [bench/README.md](bench/README.md)에 있습니다.
- 측정 수치는 `bench` 프리셋으로 빌드한 실행 파일에서만 얻습니다. Debug·sanitizer 빌드의 지연시간은 기록하지 않습니다.
- `-march=native`는 기본으로 켜지 않습니다. 켜서 측정했다면 측정 기록에 적습니다.
- `bench_selftest`는 출력 맨 위 `[build]`에 컴파일러, 빌드 종류, 플래그, sanitizer 여부를 찍습니다. 측정용 빌드가 아니면 경고를 붙입니다.

## 측정 기록

측정할 때마다 [양식](docs/templates/measurement-record.md)을 복사해 `bench/results/YYYY-MM-DD-이름.md`로 저장하고 커밋합니다.

- 기록은 고치지 않고 새 파일로 추가합니다. 느려진 결과도 지우지 않습니다.
- 프로그램을 3회 이상 실행해 실행별 수치를 모두 적습니다. 한 번만 실행한 결과는 기록하지 않습니다.
- 개발 장비(macOS, 가상머신)에서 잰 기록은 제목에 "참고용"을 붙입니다. 서로 다른 장비의 절대 수치는 비교하지 않습니다.
- 서버에서는 코어를 고정해 실행합니다. 소켓이 두 개면 CPU와 메모리를 같은 쪽에 묶습니다.
- 측정 전에 `uptime`으로 다른 사용자의 부하를 확인해 적습니다.
- 코드를 모두 커밋한 상태에서 빌드하고 잽니다. 기록의 커밋 해시가 측정한 코드를 가리켜야 합니다.

```bash
numactl --cpunodebind=0 --membind=0 taskset -c 2 ./build/bench/bin/bench_selftest --csv run.csv
```

양식의 코드, 입력 데이터, 빌드, 장비, OS, CPU 설정, 실행 조건 항목은 스크립트로 모읍니다. 출력은 영어이고 절 순서는 양식과 같습니다(Code, Input data, Build, Machine, OS, CPU settings, Run conditions). 빌드 디렉터리 기본값은 `build/bench`이고 `-b`로 바꿀 수 있습니다. 입력 파일을 넘기면 크기와 sha256을 함께 적습니다.

```bash
./scripts/collect-env.sh
./scripts/collect-env.sh -b build/bench data/sample.itch
```

## 디렉터리

| 경로 | 내용 |
| --- | --- |
| `cmake/` | 경고 옵션(`blitz_warnings`), sanitizer 옵션(`BLITZ_SANITIZE`) |
| `bench/include/`, `bench/src/` | 측정 도구 라이브러리 `blitz_bench` |
| `bench/selftest/` | 측정 도구 자체 점검 벤치마크 `bench_selftest` |
| `bench/results/` | 측정 기록 |
| `tests/` | 단위 테스트 `blitz_tests` (GoogleTest) |
| `simulator/`, `itch/`, `book/`, `pipeline/` | 2주차 이후 모듈 |
| `docs/templates/` | 측정 조건 기록 양식 |
| `scripts/` | 개발 보조 스크립트 |
| `data/` | ITCH 샘플 (git에 올리지 않음) |

## 소스 추가

타깃은 디렉터리 안의 소스 파일(`*.cpp`, `*.cc`)로 만들어집니다. 파일을 넣고 다시 빌드하면 CMake가 자동으로 다시 구성되므로 `CMakeLists.txt`를 고칠 필요가 없습니다.

| 소스 위치 | 타깃 |
| --- | --- |
| `bench/src/` | `blitz_bench` 정적 라이브러리. 소스가 없으면 헤더 전용 라이브러리 |
| `bench/selftest/` | `bench_selftest` 실행 파일 |
| `tests/` | `blitz_tests` 실행 파일. 테스트 케이스마다 `ctest`에 등록 |

- `blitz_bench`를 링크한 타깃은 `bench/include/`를 include 경로로 씁니다.
- 경고 옵션 `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`는 `blitz_warnings`를 링크한 타깃에만 붙습니다.
- 새 모듈은 코드가 생기는 주차에 최상위 `CMakeLists.txt`에 `add_subdirectory`로 추가합니다.

## 코드 스타일

`.clang-format`을 따릅니다 (arboris와 같은 설정).

cpplint 검사는 `CPPLINT.cfg` 설정으로 실행합니다. [uv](https://docs.astral.sh/uv/)가 필요합니다.

```bash
./scripts/run-cpplint.sh
```
