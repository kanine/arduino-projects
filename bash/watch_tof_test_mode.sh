#!/usr/bin/env bash
# Watch Time-of-Flight webhook request logs and optionally serial output.
# Useful for test mode validation where firmware should confirm success=true.

set -euo pipefail

LOGS_DIR="/home/kanine/ourSites/processcontrol/logs/json"
SERIAL_PORT=""
BAUD="115200"
FQBN="esp32:esp32:esp32"
POLL_SECS="1"

usage() {
  cat <<'EOF'
Usage:
  ./bash/watch_tof_test_mode.sh [options]

Options:
  --logs-dir DIR       JSON log directory to watch
  --serial-port PORT   Serial device for arduino-cli monitor (optional)
  --baud RATE          Serial baud rate (default: 115200)
  --fqbn FQBN          Board FQBN for monitor (default: esp32:esp32:esp32)
  --poll-secs N        Poll interval for new log files (default: 1)
  -h, --help           Show this help

Examples:
  ./bash/watch_tof_test_mode.sh
  ./bash/watch_tof_test_mode.sh --serial-port /dev/ttyUSB0
EOF
}

while (($# > 0)); do
  case "$1" in
    --logs-dir)
      LOGS_DIR="${2:-}"
      shift 2
      ;;
    --serial-port)
      SERIAL_PORT="${2:-}"
      shift 2
      ;;
    --baud)
      BAUD="${2:-}"
      shift 2
      ;;
    --fqbn)
      FQBN="${2:-}"
      shift 2
      ;;
    --poll-secs)
      POLL_SECS="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ ! -d "$LOGS_DIR" ]]; then
  echo "[watch] ERROR: logs directory not found: $LOGS_DIR" >&2
  exit 1
fi

if ! command -v arduino-cli >/dev/null 2>&1; then
  if [[ -x "/home/kanine/arduino-local/bin/arduino-cli" ]]; then
    export PATH="$PATH:/home/kanine/arduino-local/bin"
  fi
fi

LOG_PID=""
SERIAL_PID=""

cleanup() {
  if [[ -n "$LOG_PID" ]]; then
    kill "$LOG_PID" >/dev/null 2>&1 || true
  fi
  if [[ -n "$SERIAL_PID" ]]; then
    kill "$SERIAL_PID" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT INT TERM

watch_logs() {
  local last_file=""
  echo "[watch] log watcher started: $LOGS_DIR"
  while true; do
    local latest
    latest=$(ls -t "$LOGS_DIR"/*.json 2>/dev/null | head -n 1 || true)
    if [[ -n "$latest" && "$latest" != "$last_file" ]]; then
      last_file="$latest"
      echo "[logs] new file: $(basename "$latest")"
      awk '
        /"batch_id"/ { batch=$0 }
        /"host"/ { host=$0 }
        /"success"/ { success=$0 }
        END {
          if (host != "") print "[logs] " host
          if (batch != "") print "[logs] " batch
          if (success != "") print "[logs] " success
        }
      ' "$latest"
    fi
    sleep "$POLL_SECS"
  done
}

watch_serial() {
  echo "[watch] serial watcher started: port=$SERIAL_PORT baud=$BAUD"
  arduino-cli monitor -p "$SERIAL_PORT" -c "baudrate=$BAUD" --fqbn "$FQBN" 2>&1 |
    sed -u 's/^/[serial] /'
}

watch_logs &
LOG_PID=$!

if [[ -n "$SERIAL_PORT" ]]; then
  watch_serial &
  SERIAL_PID=$!
else
  echo "[watch] serial disabled (pass --serial-port /dev/ttyUSB0 to enable)"
fi

echo "[watch] running. Press Ctrl+C to stop."
wait
