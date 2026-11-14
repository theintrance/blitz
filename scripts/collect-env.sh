#!/bin/bash

set -euo pipefail

usage() {
    echo "usage: $0 [-b BUILD_DIR] [INPUT_FILE ...]"
}

root=$(git rev-parse --show-toplevel)
build_dir="$root/build/bench"

while getopts "b:h" option; do
    case "$option" in
        b) build_dir="$OPTARG" ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

os=$(uname -s)
cache="$build_dir/CMakeCache.txt"

cache_value() {
    sed -n "s/^$1:[A-Z]*=//p" "$cache" | head -1
}

squeeze() {
    printf '%s' "$1" | tr -s ' ' | sed 's/^ //; s/ $//'
}

sha256() {
    if command -v sha256sum > /dev/null; then
        sha256sum "$1" | cut -d ' ' -f 1
    else
        shasum -a 256 "$1" | cut -d ' ' -f 1
    fi
}

lscpu_value() {
    LC_ALL=C lscpu | sed -n "s/^$1:[[:space:]]*//p" | head -1
}

sysctl_value() {
    sysctl -n "$1" 2> /dev/null || true
}

echo "## 코드"
echo
echo "- 커밋: \`$(git -C "$root" rev-parse --short HEAD)\`"
changes=$(git -C "$root" status --short)
if [ -z "$changes" ]; then
    echo "- 커밋되지 않은 변경: 없음"
else
    echo "- 커밋되지 않은 변경: 있음"
    echo
    echo '```'
    echo "$changes"
    echo '```'
fi
echo

echo "## 입력 데이터"
echo
if [ $# -eq 0 ]; then
    echo "- 입력 파일: 없음"
else
    for file in "$@"; do
        size=$(wc -c < "$file" | tr -d ' ')
        echo "- \`$file\`: $size bytes, sha256 \`$(sha256 "$file")\`"
    done
fi
echo

echo "## 빌드"
echo
if [ -f "$cache" ]; then
    compiler=$(cache_value CMAKE_CXX_COMPILER)
    build_type=$(cache_value CMAKE_BUILD_TYPE)
    build_type_upper=$(printf '%s' "$build_type" | tr '[:lower:]' '[:upper:]')
    flags=$(squeeze "$(cache_value CMAKE_CXX_FLAGS) $(cache_value "CMAKE_CXX_FLAGS_$build_type_upper")")
    march=$(printf '%s' "$flags" | grep -o -- '-march=[^ ]*' || true)
    lto=$(cache_value CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    echo "- 컴파일러: \`$("$compiler" --version | head -1)\` (\`$compiler\`)"
    echo "- 프리셋: \`$(basename "$build_dir")\`, 빌드 종류: \`${build_type:-none}\`"
    echo "- 최적화 플래그: \`$flags\`"
    echo "- -march: ${march:-지정 안 함}"
    echo "- LTO: ${lto:-꺼짐}"
    echo "- sanitizer: \`BLITZ_SANITIZE=$(cache_value BLITZ_SANITIZE)\`"
else
    echo "- \`$cache\`가 없습니다. 먼저 \`cmake --preset bench\`로 구성합니다."
fi
echo

echo "## 장비"
echo
if [ "$os" = "Linux" ]; then
    echo "- CPU: $(lscpu_value 'Model name')"
    echo "- 소켓 $(lscpu_value 'Socket(s)'), 소켓당 코어 $(lscpu_value 'Core(s) per socket'), 코어당 스레드 $(lscpu_value 'Thread(s) per core'), NUMA 노드 $(lscpu_value 'NUMA node(s)')"
    echo "- 캐시: L1d $(lscpu_value 'L1d cache'), L2 $(lscpu_value 'L2 cache'), L3 $(lscpu_value 'L3 cache')"
    echo "- 메모리: $(free -h | awk '/^Mem:/ {print $2}')"
elif [ "$os" = "Darwin" ]; then
    performance=$(sysctl_value hw.perflevel0.physicalcpu)
    efficiency=$(sysctl_value hw.perflevel1.physicalcpu)
    l1d=$(sysctl_value hw.perflevel0.l1dcachesize)
    l2=$(sysctl_value hw.perflevel0.l2cachesize)
    echo "- CPU: $(sysctl_value machdep.cpu.brand_string)"
    echo "- 코어: 물리 $(sysctl_value hw.physicalcpu), 논리 $(sysctl_value hw.logicalcpu)${performance:+ (성능 코어 $performance, 효율 코어 ${efficiency:-0})}"
    echo "- 캐시 (성능 코어): L1d $(( ${l1d:-0} / 1024 )) KiB, L2 $(( ${l2:-0} / 1024 / 1024 )) MiB"
    echo "- 메모리: $(( $(sysctl_value hw.memsize) / 1024 / 1024 / 1024 )) GiB"
else
    echo "- 지원하지 않는 OS: $os"
fi
echo

echo "## OS"
echo
if [ "$os" = "Linux" ]; then
    distro=$(sed -n 's/^PRETTY_NAME="\{0,1\}\([^"]*\)"\{0,1\}$/\1/p' /etc/os-release 2> /dev/null || true)
    virt=$(systemd-detect-virt 2> /dev/null || true)
    echo "- 배포판: ${distro:-확인 불가}"
    echo "- 커널: $(uname -r)"
    echo "- 가상화: ${virt:-확인 불가}"
elif [ "$os" = "Darwin" ]; then
    vmm=$(sysctl_value kern.hv_vmm_present)
    echo "- 배포판: $(sw_vers -productName) $(sw_vers -productVersion) ($(sw_vers -buildVersion))"
    echo "- 커널: Darwin $(uname -r)"
    if [ "$vmm" = "1" ]; then
        echo "- 가상화: 가상머신"
    else
        echo "- 가상화: 없음"
    fi
fi
echo

echo "## CPU 설정"
echo
if [ "$os" = "Linux" ]; then
    governors=$(cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2> /dev/null | sort -u | paste -sd ',' - || true)
    smt=$(cat /sys/devices/system/cpu/smt/control 2> /dev/null || true)
    echo "- 주파수 거버너: ${governors:-확인 불가}"
    echo "- SMT: ${smt:-확인 불가}"
elif [ "$os" = "Darwin" ]; then
    power=$(pmset -g batt 2> /dev/null | sed -n "s/^Now drawing from '\(.*\)'$/\1/p")
    low_power=$(pmset -g 2> /dev/null | awk '$1 == "lowpowermode" {print $2}')
    echo "- 주파수 거버너, SMT: macOS에서는 확인·설정할 수 없음"
    echo "- 전원: ${power:-확인 불가}, 저전력 모드: ${low_power:-확인 불가}"
fi
echo

echo "## 실행 조건"
echo
echo "- 부하 (uptime): \`$(uptime | sed 's/^ *//')\`"
users=$(who | awk '{print $1}' | sort | uniq -c | awk '{printf "%s%s %s", sep, $2, $1; sep=", "}')
echo "- 접속 사용자 (who, 이름과 세션 수): ${users:-없음}"
