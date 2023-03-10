#!/bin/bash

function print_usage_and_die() {
    local MSG="$*"
    echo -e "${MSG}\nUsage: mkdmg.sh <filename> </path/to/bootstrap> </path/to/config> </path/to/pkg>"
    exit 1
}

function print_error_and_die() {
    echo "Error: $*"
    exit 1
}

function check_success() {
    if [ $? -ne 0 ]; then
        print_error_and_die "$*"
    fi
}

function cleanup() {
    local err=$?

    if [ -n "$TMPDIR" ] && [ -d "$TMPDIR" ]; then
        rm -rf "$TMPDIR"
    fi

    trap '' EXIT
    exit $err
}

function cp_and_exit_on_failure() {
  local SRC="$1"
  local DST="$2"

  echo "copying ${SRC} to ${DST}"
  cp "$SRC" "$DST"
  if [ $? -ne 0 ]; then
    echo "failed to copy \"$SRC\" to \"$DST\""
    exit 1
  fi
}

SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(dirname "$0")

# Input parameters and sanity checks
[ -z "$1" ]                 && print_usage_and_die "Missing filename for DMG"
[ -z "$2" ]                 && print_usage_and_die "Missing bootstrap filename"
[ -z "$3" ]                 && print_usage_and_die "Missing config filename"
[ -z "$4" ]                 && print_usage_and_die "Missing pkg filename"
[ ! -f "$2" ]               && print_usage_and_die "Bootstrap not found"
[ ! -f "$3" ]               && print_usage_and_die "Config not found"
[ ! -f "$4" ]               && print_usage_and_die "Package not found"
[ ! -f "${SCRIPT_DIR}/_DS_Store" ] && print_usage_and_die "${SCRIPT_DIR}/_DS_Store file not found"
[ ! -f "${SCRIPT_DIR}/background.png" ] && print_usage_and_die "${SCRIPT_DIR}/background.png file not found"

DMG_NAME="${1}"
BOOTSTRAP="${2}"
CONFIG="${2}"
PKG="${3}"

# Create temporary working directory to stage source files. Clean up on exit.
TMPDIR=$(mktemp -d -t "${SCRIPT_NAME}"_XXXXX)
trap cleanup EXIT

# Copy source files to staging directory
mkdir "${TMPDIR}/.background"
check_success "Failed to create directory ${TMPDIR}/.background"
cp_and_exit_on_failure "${SCRIPT_DIR}/background.png" "${TMPDIR}/.background"
cp_and_exit_on_failure "${SCRIPT_DIR}/_DS_Store" "${TMPDIR}/.DS_Store"
cp_and_exit_on_failure "${PKG}" "${TMPDIR}/cisco-secureclient-cloudmanagement.pkg"
cp_and_exit_on_failure "${BOOTSTRAP}" "${TMPDIR}/.bs.json"
cp_and_exit_on_failure "${CONFIG}" "${TMPDIR}/.cm_config.json"

# Create DMG file using files and directory structure from staging directory
"${SCRIPT_DIR}/mkdmgfd.sh" "${DMG_NAME}" "${TMPDIR}"

RETVAL=$?
if [ "$RETVAL" -eq 0 ]; then
    echo "Injected dmg \"${DMG_NAME}\" created successfully"
else
    print_error_and_die "Injected dmg \"${DMG_NAME}\" failed to create"
fi
