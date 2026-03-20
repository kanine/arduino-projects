#!/usr/bin/env bash
# Sync Arduino sketch folders from this repo to a local Arduino IDE directory.
# The destination is read from bash/.env (ARDUINO_IDE_DIR).
# Copy bash/.env.example to bash/.env and set the path before running.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# ── Load .env ─────────────────────────────────────────────────────────────────
ENV_FILE="${SCRIPT_DIR}/.env"
if [[ -f "${ENV_FILE}" ]]; then
  # shellcheck source=/dev/null
  source "${ENV_FILE}"
fi

ARDUINO_IDE_DIR="${ARDUINO_IDE_DIR:-}"

usage() {
  cat <<'EOF'
Usage:
  ./bash/sync_to_ide.sh [options] [sketch_name ...]

Sync Arduino sketch folders from this repo to a local Arduino IDE directory.
Destination is set via ARDUINO_IDE_DIR in bash/.env (see bash/.env.example).

Options:
  --dest DIR    Override ARDUINO_IDE_DIR from .env
  --dry-run     Show what would change without copying files
  --delete      Delete files in destination that no longer exist locally
  -h, --help    Show this help

Examples:
  ./bash/sync_to_ide.sh
  ./bash/sync_to_ide.sh sonic-mm-detect
  ./bash/sync_to_ide.sh --dry-run sonic-mm-detect counter-00-99
  ./bash/sync_to_ide.sh --dest /mnt/d/Scripts/arduino-projects
EOF
}

DRY_RUN=0
DELETE=0
declare -a REQUESTED_SKETCHES=()

while (($# > 0)); do
  case "$1" in
    --dest)
      ARDUINO_IDE_DIR="${2:-}"
      shift 2
      ;;
    --dry-run)
      DRY_RUN=1
      shift
      ;;
    --delete)
      DELETE=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
    *)
      REQUESTED_SKETCHES+=("$1")
      shift
      ;;
  esac
done

if [[ -z "${ARDUINO_IDE_DIR}" ]]; then
  echo "Error: ARDUINO_IDE_DIR is not set." >&2
  echo "  Set it in bash/.env (see bash/.env.example) or pass --dest DIR." >&2
  exit 1
fi

# ── Discover sketches ─────────────────────────────────────────────────────────
# Each sketch is the directory that directly contains a .ino file.
discover_sketches() {
  while IFS= read -r ino_file; do
    dirname "${ino_file}"
  done < <(find "${REPO_ROOT}" \
    -not -path "${REPO_ROOT}/.git/*" \
    -not -path "${REPO_ROOT}/bash/*" \
    -name "*.ino" \
    | sort)
}

declare -A SKETCH_MAP=()   # sketch_name -> relative_path/from/repo_root
while IFS= read -r abs_path; do
  rel_path="${abs_path#"${REPO_ROOT}/"}"
  name="$(basename "${abs_path}")"
  SKETCH_MAP["${name}"]="${rel_path}"
done < <(discover_sketches)

if ((${#SKETCH_MAP[@]} == 0)); then
  echo "No sketches (*.ino) found under ${REPO_ROOT}." >&2
  exit 1
fi

# ── Resolve which sketches to sync ────────────────────────────────────────────
declare -a SKETCHES_TO_SYNC=()

if ((${#REQUESTED_SKETCHES[@]} == 0)); then
  # All discovered sketches
  while IFS= read -r name; do
    SKETCHES_TO_SYNC+=("${name}")
  done < <(printf '%s\n' "${!SKETCH_MAP[@]}" | sort)
else
  for name in "${REQUESTED_SKETCHES[@]}"; do
    if [[ -z "${SKETCH_MAP[${name}]:-}" ]]; then
      echo "Sketch not found locally: ${name}" >&2
      echo "Available sketches:" >&2
      printf '  %s\n' "${!SKETCH_MAP[@]}" | sort >&2
      exit 1
    fi
    SKETCHES_TO_SYNC+=("${name}")
  done
fi

# ── rsync args ────────────────────────────────────────────────────────────────
declare -a RSYNC_ARGS=(
  --archive
  --compress
  --human-readable
  --itemize-changes
  --omit-dir-times
  --exclude=.git
  --exclude=.DS_Store
  --exclude='*.ok'
  --exclude='*.notok'
)

if ((DRY_RUN)); then
  RSYNC_ARGS+=(--dry-run)
fi

if ((DELETE)); then
  RSYNC_ARGS+=(--delete)
fi

# ── Sync ──────────────────────────────────────────────────────────────────────
echo "Arduino IDE dir : ${ARDUINO_IDE_DIR}"
echo "Repo root       : ${REPO_ROOT}"
echo "Sketches to sync: ${SKETCHES_TO_SYNC[*]}"
echo

mkdir -p "${ARDUINO_IDE_DIR}"

for name in "${SKETCHES_TO_SYNC[@]}"; do
  rel_path="${SKETCH_MAP[${name}]}"
  local_path="${REPO_ROOT}/${rel_path}/"
  dest_path="${ARDUINO_IDE_DIR}/${rel_path}/"

  echo "==> Syncing ${rel_path}"
  mkdir -p "$(dirname "${dest_path}")"
  rsync "${RSYNC_ARGS[@]}" "${local_path}" "${dest_path}"
  echo
done

echo "Sync complete."
