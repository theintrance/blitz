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

echo "## Code"
echo
echo "- commit: \`$(git -C "$root" rev-parse --short HEAD)\`"
changes=$(git -C "$root" status --short)
if [ -z "$changes" ]; then
    echo "- uncommitted changes: none"
else
    echo "- uncommitted changes: yes"
    echo
    echo '```'
    echo "$changes"
    echo '```'
fi
echo

echo "## Input data"
echo
if [ $# -eq 0 ]; then
    echo "- input files: none"
else
    for file in "$@"; do
        size=$(wc -c < "$file" | tr -d ' ')
        echo "- \`$file\`: $size bytes, sha256 \`$(sha256 "$file")\`"
    done
fi
echo

echo "## Build"
echo
if [ -f "$cache" ]; then
    compiler=$(cache_value CMAKE_CXX_COMPILER)
    build_type=$(cache_value CMAKE_BUILD_TYPE)
    build_type_upper=$(printf '%s' "$build_type" | tr '[:lower:]' '[:upper:]')
    flags=$(squeeze "$(cache_value CMAKE_CXX_FLAGS) $(cache_value "CMAKE_CXX_FLAGS_$build_type_upper")")
    march=$(printf '%s' "$flags" | grep -o -- '-march=[^ ]*' || true)
    lto=$(cache_value CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    cmake_command=$(cache_value CMAKE_COMMAND)
    echo "- compiler: \`$("$compiler" --version | head -1)\` (\`$compiler\`)"
    echo "- cmake: \`$("$cmake_command" --version | head -1)\`"
    echo "- preset: \`$(basename "$build_dir")\`, build type: \`${build_type:-none}\`"
    echo "- flags: \`$flags\`"
    echo "- march: ${march:-not set}"
    echo "- lto: ${lto:-off}"
    echo "- sanitizer: \`BLITZ_SANITIZE=$(cache_value BLITZ_SANITIZE)\`"
else
    echo "- \`$cache\` not found, run \`cmake --preset bench\` first"
fi
echo

echo "## Machine"
echo
if [ "$os" = "Linux" ]; then
    echo "- cpu: $(lscpu_value 'Model name')"
    echo "- sockets $(lscpu_value 'Socket(s)'), cores per socket $(lscpu_value 'Core(s) per socket'), threads per core $(lscpu_value 'Thread(s) per core'), numa nodes $(lscpu_value 'NUMA node(s)')"
    echo "- cache: L1d $(lscpu_value 'L1d cache'), L2 $(lscpu_value 'L2 cache'), L3 $(lscpu_value 'L3 cache')"
    echo "- memory: $(free -h | awk '/^Mem:/ {print $2}')"
elif [ "$os" = "Darwin" ]; then
    performance=$(sysctl_value hw.perflevel0.physicalcpu)
    efficiency=$(sysctl_value hw.perflevel1.physicalcpu)
    l1d=$(sysctl_value hw.perflevel0.l1dcachesize)
    l2=$(sysctl_value hw.perflevel0.l2cachesize)
    echo "- cpu: $(sysctl_value machdep.cpu.brand_string)"
    echo "- cores: physical $(sysctl_value hw.physicalcpu), logical $(sysctl_value hw.logicalcpu)${performance:+ (performance $performance, efficiency ${efficiency:-0})}"
    echo "- cache (performance cores): L1d $(( ${l1d:-0} / 1024 )) KiB, L2 $(( ${l2:-0} / 1024 / 1024 )) MiB"
    echo "- memory: $(( $(sysctl_value hw.memsize) / 1024 / 1024 / 1024 )) GiB"
else
    echo "- unsupported os: $os"
fi
echo

echo "## OS"
echo
if [ "$os" = "Linux" ]; then
    distro=$(sed -n 's/^PRETTY_NAME="\{0,1\}\([^"]*\)"\{0,1\}$/\1/p' /etc/os-release 2> /dev/null || true)
    virt=$(systemd-detect-virt 2> /dev/null || true)
    echo "- distribution: ${distro:-unknown}"
    echo "- kernel: $(uname -r)"
    echo "- virtualization: ${virt:-unknown}"
elif [ "$os" = "Darwin" ]; then
    vmm=$(sysctl_value kern.hv_vmm_present)
    echo "- distribution: $(sw_vers -productName) $(sw_vers -productVersion) ($(sw_vers -buildVersion))"
    echo "- kernel: Darwin $(uname -r)"
    if [ "$vmm" = "1" ]; then
        echo "- virtualization: virtual machine"
    else
        echo "- virtualization: none"
    fi
fi
echo

echo "## CPU settings"
echo
if [ "$os" = "Linux" ]; then
    governors=$(cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2> /dev/null | sort -u | paste -sd ',' - || true)
    smt=$(cat /sys/devices/system/cpu/smt/control 2> /dev/null || true)
    isolated=$(cat /sys/devices/system/cpu/isolated 2> /dev/null || true)
    echo "- governor: ${governors:-not available}"
    echo "- smt: ${smt:-not available}"
    echo "- isolated cpus: ${isolated:-none}"
elif [ "$os" = "Darwin" ]; then
    power=$(pmset -g batt 2> /dev/null | sed -n "s/^Now drawing from '\(.*\)'$/\1/p")
    low_power=$(pmset -g 2> /dev/null | awk '$1 == "lowpowermode" {print $2}')
    echo "- governor, smt: not available on macOS"
    echo "- power: ${power:-unknown}, low power mode: ${low_power:-unknown}"
fi
echo

echo "## Run conditions"
echo
echo "- load (uptime): \`$(uptime | sed 's/^ *//')\`"
users=$(who | awk '{print $1}' | sort | uniq -c | awk '{printf "%s%s %s", sep, $2, $1; sep=", "}')
echo "- logged-in users (who, name and session count): ${users:-none}"
