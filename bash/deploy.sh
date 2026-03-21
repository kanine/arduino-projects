#!/usr/bin/env bash
# Deploy an Arduino sketch: sync to IDE, compile, and upload.
# Board family is inferred from the sketch path (Mega_2560 vs ESP32_dev).
# Upload method is inferred from the board family (serial vs OTA network).
# Configuration lives in bash/.env (see bash/.env.example).
#
# Usage:
#   ./bash/deploy.sh <sketch_name> [options]
#
# Examples:
#   ./bash/deploy.sh otabasic
#   ./bash/deploy.sh otabasic --no-sync
#   ./bash/deploy.sh sonic-mm-detect --port /dev/ttyUSB1

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# ── Load .env ─────────────────────────────────────────────────────────────────
ENV_FILE="${SCRIPT_DIR}/.env"
if [[ -f "${ENV_FILE}" ]]; then
  # shellcheck source=/dev/null
  source "${ENV_FILE}"
fi

# ── Defaults from .env (overridable via CLI) ───────────────────────────────────
ESP32_FQBN="${ESP32_FQBN:-esp32:esp32:esp32}"
ESP32_OTA_PORT="${ESP32_OTA_PORT:-}"
ESP32_OTA_PASSWORD="${ESP32_OTA_PASSWORD:-}"
MEGA_FQBN="${MEGA_FQBN:-arduino:avr:mega}"
MEGA_SERIAL_PORT="${MEGA_SERIAL_PORT:-/dev/ttyUSB0}"

# ── Logging ────────────────────────────────────────────────────────────────────
LOG_LEVEL="${LOG_LEVEL:-info}"   # debug | info | warn | error

log()  { echo "[deploy] $*"; }
info() { [[ "${LOG_LEVEL}" != "error" ]] && echo "[deploy] $*" || true; }
warn() { echo "[deploy] WARN: $*" >&2; }
err()  { echo "[deploy] ERROR: $*" >&2; }

# ── Usage ─────────────────────────────────────────────────────────────────────
usage() {
  cat <<'EOF'
Usage:
  ./bash/deploy.sh <sketch_name> [options]

Sync, compile, and upload an Arduino sketch.
Board family and upload method are inferred from the sketch path.

Options:
  --no-sync         Skip syncing to Arduino IDE directory
  --no-upload       Compile only, do not upload
  --port PORT       Override the upload port (OTA IP or serial device)
  -h, --help        Show this help

Environment (.env):
  ESP32_FQBN            FQBN for ESP32 boards   (default: esp32:esp32:esp32)
  ESP32_OTA_PORT        IP address for OTA upload
  ESP32_OTA_PASSWORD    OTA password (leave empty if not set)
  MEGA_FQBN             FQBN for Mega boards    (default: arduino:avr:mega)
  MEGA_SERIAL_PORT      Serial port for Mega    (default: /dev/ttyUSB0)
  LOG_LEVEL             debug | info | warn | error (default: info)

Examples:
  ./bash/deploy.sh otabasic
  ./bash/deploy.sh otabasic --no-sync
  ./bash/deploy.sh otabasic --port 192.168.1.100
  ./bash/deploy.sh sonic-mm-detect --port /dev/ttyUSB1
EOF
}

# ── Parse arguments ────────────────────────────────────────────────────────────
SKETCH_NAME=""
DO_SYNC=1
DO_UPLOAD=1
PORT_OVERRIDE=""

while (($# > 0)); do
  case "$1" in
    --no-sync)    DO_SYNC=0;    shift ;;
    --no-upload)  DO_UPLOAD=0;  shift ;;
    --port)       PORT_OVERRIDE="${2:-}"; shift 2 ;;
    -h|--help)    usage; exit 0 ;;
    -*)           err "Unknown option: $1"; usage >&2; exit 1 ;;
    *)
      if [[ -z "${SKETCH_NAME}" ]]; then
        SKETCH_NAME="$1"
      else
        err "Unexpected argument: $1"; usage >&2; exit 1
      fi
      shift
      ;;
  esac
done

if [[ -z "${SKETCH_NAME}" ]]; then
  err "No sketch name given."
  usage >&2
  exit 1
fi

# ── Discover sketch path ───────────────────────────────────────────────────────
SKETCH_PATH=""
while IFS= read -r ino_file; do
  dir="$(dirname "${ino_file}")"
  name="$(basename "${dir}")"
  if [[ "${name}" == "${SKETCH_NAME}" ]]; then
    SKETCH_PATH="${dir}"
    break
  fi
done < <(find "${REPO_ROOT}" \
  -not -path "${REPO_ROOT}/.git/*" \
  -not -path "${REPO_ROOT}/bash/*" \
  -name "*.ino" | sort)

if [[ -z "${SKETCH_PATH}" ]]; then
  err "Sketch not found: ${SKETCH_NAME}"
  echo "Available sketches:" >&2
  find "${REPO_ROOT}" \
    -not -path "${REPO_ROOT}/.git/*" \
    -not -path "${REPO_ROOT}/bash/*" \
    -name "*.ino" \
    -exec dirname {} \; | xargs -I{} basename {} | sort | sed 's/^/  /' >&2
  exit 1
fi

REL_PATH="${SKETCH_PATH#"${REPO_ROOT}/"}"

# ── Detect board family ────────────────────────────────────────────────────────
BOARD_FAMILY=""
case "${REL_PATH}" in
  ESP32_dev/*)  BOARD_FAMILY="esp32" ;;
  Mega_2560/*)  BOARD_FAMILY="mega"  ;;
  *)
    err "Cannot infer board family from path: ${REL_PATH}"
    err "Expected path to start with ESP32_dev/ or Mega_2560/"
    exit 1
    ;;
esac

# ── Resolve FQBN and upload params ────────────────────────────────────────────
if [[ "${BOARD_FAMILY}" == "esp32" ]]; then
  FQBN="${ESP32_FQBN}"
  UPLOAD_PORT="${PORT_OVERRIDE:-${ESP32_OTA_PORT}}"
  UPLOAD_PROTOCOL="network"
  UPLOAD_PASSWORD="${ESP32_OTA_PASSWORD}"
else
  FQBN="${MEGA_FQBN}"
  UPLOAD_PORT="${PORT_OVERRIDE:-${MEGA_SERIAL_PORT}}"
  UPLOAD_PROTOCOL="serial"
  UPLOAD_PASSWORD=""
fi

# ── Print plan ─────────────────────────────────────────────────────────────────
log "────────────────────────────────────────────"
log "Sketch   : ${REL_PATH}"
log "Board    : ${FQBN}"
log "Upload   : ${UPLOAD_PROTOCOL} → ${UPLOAD_PORT:-<not set>}"
log "────────────────────────────────────────────"

# ── Step 1: Sync to IDE ────────────────────────────────────────────────────────
if ((DO_SYNC)); then
  info "Syncing to Arduino IDE..."
  bash "${SCRIPT_DIR}/sync_to_ide.sh" "${SKETCH_NAME}"
  echo
fi

# ── Step 2: Compile ────────────────────────────────────────────────────────────
info "Compiling ${SKETCH_NAME} (${FQBN})..."
arduino-cli compile --fqbn "${FQBN}" "${SKETCH_PATH}"
echo

# ── Step 3: Upload ─────────────────────────────────────────────────────────────
if ((DO_UPLOAD)); then
  if [[ -z "${UPLOAD_PORT}" ]]; then
    err "No upload port configured."
    if [[ "${BOARD_FAMILY}" == "esp32" ]]; then
      err "Set ESP32_OTA_PORT in bash/.env or pass --port <ip>"
    else
      err "Set MEGA_SERIAL_PORT in bash/.env or pass --port /dev/ttyUSBx"
    fi
    exit 1
  fi

  info "Uploading to ${UPLOAD_PORT} via ${UPLOAD_PROTOCOL}..."

  declare -a UPLOAD_ARGS=(
    --fqbn "${FQBN}"
    --port "${UPLOAD_PORT}"
    --protocol "${UPLOAD_PROTOCOL}"
  )

  if [[ "${UPLOAD_PROTOCOL}" == "network" ]]; then
    UPLOAD_ARGS+=(--upload-field "password=${UPLOAD_PASSWORD}")
  fi

  arduino-cli upload "${UPLOAD_ARGS[@]}" "${SKETCH_PATH}"
  echo
  log "Done."
fi
