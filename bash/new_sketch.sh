#!/usr/bin/env bash
# Create a new Arduino sketch with the correct folder structure.
# Prompts interactively for any argument not supplied on the command line.
#
# Usage:
#   ./bash/new_sketch.sh [options]
#   ./bash/new_sketch.sh --name mysketch --board esp32 --ota
#   ./bash/new_sketch.sh --name counter  --board mega
#
# Options:
#   --name NAME       Sketch folder/file name (lowercase, no spaces)
#   --board BOARD     Target board: esp32 | mega
#   --ota             Include ota_core.h (ESP32 only, default for esp32)
#   --no-ota          Skip OTA even for ESP32
#   --category CAT    Subfolder under Mega_2560/ (e.g. 4digits); ignored for ESP32
#   -h, --help        Show this help

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

OTA_CORE_SRC="${REPO_ROOT}/ESP32_dev/otacore/ota_core.h"
SECRETS_SRC_ESP32="${REPO_ROOT}/ESP32_dev/otacore/secrets.h.example"

# ── Parse arguments ────────────────────────────────────────────────────────────
SKETCH_NAME=""
BOARD=""
OTA=""           # "yes" | "no" | "" (unset = ask for esp32)
CATEGORY=""

usage() {
  cat <<'EOF'
Usage:
  ./bash/new_sketch.sh [options]

Options:
  --name NAME       Sketch name (lowercase, no spaces)
  --board BOARD     esp32 | mega
  --ota             Include OTA support (ESP32 only)
  --no-ota          Skip OTA
  --category CAT    Subfolder under Mega_2560/ (e.g. 4digits)
  -h, --help        Show this help
EOF
}

while (($# > 0)); do
  case "$1" in
    --name)      SKETCH_NAME="${2:-}"; shift 2 ;;
    --board)     BOARD="${2:-}";       shift 2 ;;
    --ota)       OTA="yes";            shift   ;;
    --no-ota)    OTA="no";             shift   ;;
    --category)  CATEGORY="${2:-}";    shift 2 ;;
    -h|--help)   usage; exit 0 ;;
    -*)          echo "Unknown option: $1" >&2; usage >&2; exit 1 ;;
    *)           echo "Unexpected argument: $1" >&2; usage >&2; exit 1 ;;
  esac
done

# ── Interactive prompts for missing args ───────────────────────────────────────
prompt() {
  local var_name="$1"
  local prompt_text="$2"
  local default="${3:-}"
  local value
  if [[ -n "${default}" ]]; then
    read -rp "${prompt_text} [${default}]: " value
    value="${value:-${default}}"
  else
    read -rp "${prompt_text}: " value
  fi
  printf -v "${var_name}" '%s' "${value}"
}

if [[ -z "${SKETCH_NAME}" ]]; then
  prompt SKETCH_NAME "Sketch name (lowercase, no spaces)"
fi

# Normalise: lowercase, spaces to hyphens
SKETCH_NAME="${SKETCH_NAME,,}"
SKETCH_NAME="${SKETCH_NAME// /-}"

if [[ -z "${BOARD}" ]]; then
  prompt BOARD "Board (esp32 / mega)" "esp32"
fi

BOARD="${BOARD,,}"
if [[ "${BOARD}" != "esp32" && "${BOARD}" != "mega" ]]; then
  echo "Error: board must be esp32 or mega (got: ${BOARD})" >&2
  exit 1
fi

if [[ "${BOARD}" == "esp32" && -z "${OTA}" ]]; then
  read -rp "Include OTA support? [Y/n]: " ota_ans
  ota_ans="${ota_ans:-Y}"
  if [[ "${ota_ans,,}" == "y" ]]; then OTA="yes"; else OTA="no"; fi
fi

if [[ "${BOARD}" == "mega" && -z "${CATEGORY}" ]]; then
  prompt CATEGORY "Subfolder under Mega_2560/ (leave blank for none)" ""
fi

# ── Resolve destination path ───────────────────────────────────────────────────
if [[ "${BOARD}" == "esp32" ]]; then
  SKETCH_DIR="${REPO_ROOT}/ESP32_dev/${SKETCH_NAME}"
else
  if [[ -n "${CATEGORY}" ]]; then
    SKETCH_DIR="${REPO_ROOT}/Mega_2560/${CATEGORY}/${SKETCH_NAME}"
  else
    SKETCH_DIR="${REPO_ROOT}/Mega_2560/${SKETCH_NAME}"
  fi
fi

if [[ -e "${SKETCH_DIR}" ]]; then
  echo "Error: '${SKETCH_DIR}' already exists." >&2
  exit 1
fi

# ── Create directory structure ─────────────────────────────────────────────────
echo ""
echo "Creating sketch: ${SKETCH_DIR#"${REPO_ROOT}/"}"
mkdir -p "${SKETCH_DIR}/docs"

# ── Write .ino ─────────────────────────────────────────────────────────────────
if [[ "${BOARD}" == "esp32" && "${OTA}" == "yes" ]]; then
  cat > "${SKETCH_DIR}/${SKETCH_NAME}.ino" <<EOF
// ${SKETCH_NAME} – ESP32
//
// Configuration lives in secrets.h (copy from secrets.h.example).

// ── Optional OTA config ────────────────────────────────────────────────────────
#define OTA_LED_PIN  2          // onboard blue LED; remove if no LED wired

#include "ota_core.h"

// ── User parameters ───────────────────────────────────────────────────────────

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(OTA_LED_PIN, OUTPUT);
  digitalWrite(OTA_LED_PIN, LOW);

  otaCoreSetup();
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  otaCoreHandle();
  if (!otaCoreReady()) return;

  // ── Sketch logic ──────────────────────────────────────────────────────────
}
EOF

elif [[ "${BOARD}" == "esp32" ]]; then
  cat > "${SKETCH_DIR}/${SKETCH_NAME}.ino" <<EOF
// ${SKETCH_NAME} – ESP32
//
// Configuration lives in secrets.h (copy from secrets.h.example).

#include <WiFi.h>
#include "secrets.h"

// ── Constants ─────────────────────────────────────────────────────────────────
static const unsigned long WIFI_TIMEOUT_MS = 15000;

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  // ── Sketch logic ──────────────────────────────────────────────────────────
}
EOF

else
  # Mega
  cat > "${SKETCH_DIR}/${SKETCH_NAME}.ino" <<EOF
// ${SKETCH_NAME} – Arduino Mega 2560

// ── User parameters ───────────────────────────────────────────────────────────

// ── Constants ─────────────────────────────────────────────────────────────────

// ── State ─────────────────────────────────────────────────────────────────────

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(19200);
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  // ── Sketch logic ──────────────────────────────────────────────────────────
}
EOF
fi

# ── Copy ota_core.h if requested ──────────────────────────────────────────────
if [[ "${OTA}" == "yes" ]]; then
  if [[ ! -f "${OTA_CORE_SRC}" ]]; then
    echo "Warning: ${OTA_CORE_SRC} not found — skipping ota_core.h copy." >&2
  else
    cp "${OTA_CORE_SRC}" "${SKETCH_DIR}/ota_core.h"
    echo "  + ota_core.h"
  fi
fi

# ── Write secrets files (ESP32 only) ──────────────────────────────────────────
if [[ "${BOARD}" == "esp32" ]]; then
  if [[ -f "${SECRETS_SRC_ESP32}" ]]; then
    cp "${SECRETS_SRC_ESP32}" "${SKETCH_DIR}/secrets.h.example"
    cp "${SECRETS_SRC_ESP32}" "${SKETCH_DIR}/secrets.h"
    echo "  + secrets.h.example"
    echo "  + secrets.h  (fill in your credentials)"
  fi
fi

# ── Write docs/wiring.md stub ─────────────────────────────────────────────────
BOARD_TYPE="ESP32 WROOM"
BOARD_LOGIC="3.3V"
if [[ "${BOARD}" == "mega" ]]; then
  BOARD_TYPE="Arduino Mega 2560"
  BOARD_LOGIC="5V"
fi

cat > "${SKETCH_DIR}/docs/wiring.md" <<EOF
ATN-IO v3
Project: ${SKETCH_NAME} — <brief description>

[BOARD]
TYPE  = ${BOARD_TYPE}
LOGIC = ${BOARD_LOGIC}

[INPUTS]

[OUTPUTS]

[COMPONENTS]

[WIRING]

[NOTES]
EOF

echo "  + docs/wiring.md"
echo "  + ${SKETCH_NAME}.ino"
echo ""
echo "Sketch created: ${SKETCH_DIR#"${REPO_ROOT}/"}"

if [[ "${BOARD}" == "esp32" ]]; then
  echo "Next: edit secrets.h with your WiFi credentials, then:"
  echo "  bash bash/deploy.sh ${SKETCH_NAME}"
fi
