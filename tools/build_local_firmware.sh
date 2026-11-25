#!/usr/bin/env bash
# Build with the isolated environment described in docs/wireless-disconnect.md.
set -euo pipefail

repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
environment="$repo_dir/build/local-zephyr/env.sh"
if [[ ! -f "$environment" ]]; then
    echo "Missing local environment: $environment" >&2
    exit 1
fi
source "$environment"

target="${1:-analytics}"
args=(-b seeeduino_xiao_ble)
config_args=("-DZMK_CONFIG=$repo_dir/config")
case "$target" in
    analytics)
        output=aroundforty-r-analytics
        args+=(-S studio-rpc-usb-uart)
        config_args+=("-DSHIELD=AroundForty-RB_R rgbled_adapter"
            "-DZMK_EXTRA_MODULES=$repo_dir;$repo_dir/analytics"
            "-DEXTRA_CONF_FILE=$repo_dir/analytics/usb-poc.conf;$repo_dir/analytics/persistence.conf"
            "-DEXTRA_DTC_OVERLAY_FILE=$repo_dir/analytics/usb-poc.overlay")
        ;;
    left)
        output=aroundforty-l
        config_args+=("-DSHIELD=AroundForty-RB_L rgbled_adapter" "-DZMK_EXTRA_MODULES=$repo_dir")
        ;;
    right)
        output=aroundforty-r
        args+=(-S studio-rpc-usb-uart)
        config_args+=("-DSHIELD=AroundForty-RB_R rgbled_adapter" "-DZMK_EXTRA_MODULES=$repo_dir")
        ;;
    *)
        echo "Usage: bash tools/build_local_firmware.sh [analytics|left|right]" >&2
        exit 2
        ;;
esac

cd "$repo_dir/build/local-zephyr/workspace"
exec west build -s zmk/app -d "$repo_dir/build/firmware/$output" "${args[@]}" \
    -- "${config_args[@]}"
