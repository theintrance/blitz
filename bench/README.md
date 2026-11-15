# bench

함수 하나를 여러 번 실행하면서 호출마다 걸린 시간을 모으고, 그 분포(p50/p90/p99/p99.9, 히스토그램)와 처리량을 출력하는 측정 도구입니다.

## 사용법

```cpp
#include "blitz/bench/histogram.hpp"
#include "blitz/bench/report.hpp"
#include "blitz/bench/runner.hpp"

const blitz::bench::RunResult result = blitz::bench::run("parse", blitz::bench::RunConfig{}, [&] { return parse(message); });
blitz::bench::print_report(result, std::cout);
blitz::bench::print_histogram(result.recorder.samples(), std::cout);
```

- `run`은 warm-up 횟수만큼 실행하고(기록 안 함), 반복 횟수만큼 호출 앞뒤를 `now_ns()`로 재서 기록한 뒤 요약을 돌려줍니다.
- 기본값은 warm-up 1만 회, 측정 100만 회입니다.
- 함수가 값을 반환하면 `do_not_optimize`로 붙잡아 두므로 계산이 지워지지 않습니다. 반환값이 없는 함수라면 결과를 직접 `do_not_optimize`에 넘깁니다.

| 헤더 | 내용 |
| --- | --- |
| `blitz/bench/timer.hpp` | `now_ns()` (`std::chrono::steady_clock`), `measure_timer_overhead()`, `summarize_timer_deltas()` |
| `blitz/bench/recorder.hpp` | `LatencyRecorder` |
| `blitz/bench/stats.hpp` | `percentile()`, `summarize()` |
| `blitz/bench/histogram.hpp` | 2의 거듭제곱 구간 히스토그램 |
| `blitz/bench/runner.hpp` | `run()`, `RunConfig`, `RunResult`, `do_not_optimize()` |
| `blitz/bench/report.hpp` | 사람이 읽는 출력, CSV |

## 백분위수 정의

nearest-rank 방식입니다. 정렬된 N개에서 순위 = ⌈N × p⌉이고, 값은 그 순위의 원소(1부터 셈)입니다.

- p를 분수 num/den으로 받아 순위를 정수로 계산합니다: `rank = (N * num + den - 1) / den`
- p50 = 1/2, p90 = 9/10, p99 = 99/100, p99.9 = 999/1000
- 표본이 비었거나 p가 (0, 1] 밖이면 `std::invalid_argument`를 던집니다.
- 표본이 1,000개 미만이면 출력에서 p99.9 옆에 경고를 붙입니다.

## 히스토그램

구간 k는 2^(k-1) ns 이상 2^k ns 미만이고, 구간 번호는 `std::bit_width(값)`입니다. 0 ns는 구간 0에 따로 모입니다. 비어 있지 않은 가장 낮은 구간부터 가장 높은 구간까지 출력하고, 그 사이의 빈 구간도 0으로 표시해 분포가 두 봉우리로 갈라지는 것을 볼 수 있게 했습니다.

## bench_selftest

```bash
cmake --preset bench
cmake --build --preset bench
taskset -c 2 ./build/bench/bin/bench_selftest --csv bench_selftest.csv
```

| 대상 | 설정 | 확인하는 것 |
| --- | --- | --- |
| `timer_overhead` | 100만 회 | 이 도구로 잴 수 있는 하한 |
| `empty` | warm-up 1만, 측정 100만 | `timer_overhead`와 비슷해야 함 |
| `sleep_100us` | warm-up 10, 측정 1,000 | 모든 값이 100,000 ns 이상. 아니면 종료 코드 1 |
| `sort_1024` | warm-up 1만, 측정 100만 | p50 대비 p99.9가 벌어지는 정도 |

- 정렬 입력은 `std::mt19937`(시드 42)의 출력값을 그대로 씁니다. 엔진 출력은 표준에 정해져 있어서 libstdc++와 libc++에서 같은 입력이 나옵니다. 분포 객체(`uniform_int_distribution`)는 구현마다 결과가 달라서 쓰지 않았습니다.
- 매 호출마다 같은 입력을 미리 잡아 둔 버퍼에 복사한 뒤 정렬하므로, 측정 구간에 할당이 없습니다.
- CSV는 `--csv PATH`(기본값 `bench_selftest.csv`)에 한 줄씩 이어 쓰고, 파일이 비어 있을 때만 헤더를 씁니다. 열은 `name,count,min_ns,p50_ns,p90_ns,p99_ns,p999_ns,max_ns,mean_ns,throughput_ops_per_sec`입니다.

## 타이머 오버헤드

`now_ns()`를 연속으로 두 번 부른 차이를 100만 번 모은 값입니다. 프로그램을 3회 실행했습니다.

| 장비 | min | median | mean | 분해능 |
| --- | --- | --- | --- | --- |
| AMD EPYC 7532 (VMware 게스트, vCPU 8), Ubuntu 24.04.5, g++ 13.3, `bench` 프리셋, `taskset -c 2` | 21–30 ns | 50 ns | 50.4–52.6 ns | n/a |
| Apple M2, macOS 27.0.1, Apple Clang 21, `bench` 프리셋 (참고용) | 0 ns | 0 ns | 14.6–17.1 ns | 41 ns |

- 분해능(`resolution`)은 시계의 한 눈금입니다. 차이 중에 0이 있으면 두 호출이 같은 눈금에 들어간 것이므로, 0이 아닌 가장 작은 차이를 한 눈금으로 봅니다. 차이가 모두 0보다 크면 가장 작은 차이는 호출 비용이고 한 눈금은 min 이하라는 것만 알 수 있어서 `n/a`로 찍습니다.
- 서버 수치의 측정 조건은 [bench/results/2026-11-14-sort-1024-epyc7532.md](results/2026-11-14-sort-1024-epyc7532.md)에 있습니다.
- 서버에서는 `now_ns()` 두 번에 약 50 ns가 듭니다. 10배 규칙에 따르면 이 서버에서는 약 500 ns보다 짧은 함수의 호출별 지연을 믿기 어렵습니다.
- 서버는 VMware 가상머신이라 시계와 vCPU 배치가 하이퍼바이저를 거칩니다. 물리 서버에서 재면 값이 달라질 수 있습니다.
- Apple Silicon의 `steady_clock`은 24 MHz로 증가해서 한 눈금이 41.67 ns입니다(출력에서는 41 또는 42). 연속 호출 비용이 한 눈금보다 작아서 min과 median이 0으로 나오고, 이때는 평균이 실제 비용의 추정치입니다.

## 한계

- 호출마다 타이머를 두 번 부르므로, 타이머 오버헤드의 10배보다 짧은 함수는 측정값을 믿기 어렵습니다. 그런 함수는 여러 번 묶어 재야 하지만, 묶으면 호출별 꼬리 지연은 볼 수 없습니다. 묶음 측정은 구현하지 않았습니다.
- 처리량에는 타이머 호출과 기록 비용이 포함되므로 짧은 함수일수록 실제보다 낮게 나옵니다.
- 같은 함수를 빈틈없이 반복하면 캐시와 분기 예측이 실제 상황보다 유리해집니다(Carl Cook 노트 16장의 마이크로벤치마크 함정).
- 기록기는 호출마다 8바이트를 순서대로 쓰므로(100만 회에 8 MB), 측정 중에 캐시 일부를 차지합니다.
- macOS에서는 분해능이 41 ns라 짧은 구간의 분포가 0과 41/42 ns로만 나뉩니다.

## 구현 메모

- `LatencyRecorder`는 생성할 때 용량만큼 0으로 채운 메모리를 확보합니다. 0을 쓰는 동안 페이지가 실제로 잡히므로 측정 중에 page fault가 나지 않습니다. `record`는 할당하지 않고, 용량이 차면 버린 개수만 셉니다.
- `run`은 기록기를 warm-up 전에 만듭니다. 8 MB를 0으로 채우면 캐시가 밀려나므로 그 뒤에 warm-up으로 다시 데웁니다.
- 처리량 = 측정 횟수 ÷ 측정 루프 전체를 따로 잰 경과 시간(초)입니다.
