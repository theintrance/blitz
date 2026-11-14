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
