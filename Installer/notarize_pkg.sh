#!/bin/bash -e
#
# User Environment Variables (All must be present)
# ================================================
#
# NOTARIZATION_USER             The username used for notarization
#                               of signed pkg files
# NOTARIZATION_PASS             The password used for notarization
#                               of signed pkg files
#

#
# Log an error to stderr and quit.
#
# $@ - Message
#
function log_error_and_die() {
    local MSG="$*"
    printf "%s\n" "${MSG}" >&2
    exit 1
}

#
# Log a message to stdout.
#
# $@ - Message
#
function log_msg() {
    local MSG="$*"
    printf "%s\n" "${MSG}"
}

#
# Wait for notarization to complete.
# Sleeps for 90 seconds, then polls for an update every 30 seconds. Function
# will abort on error or exit on success.
#
# $1 - Notarization UUID to poll an update for
#
function wait_for_notarization_complete() {
  local UUID="${1}"

  # Time represented in seconds (8 hours)
  local MAX_WAIT_TIME=28800
  local TIME_WAITED=0

  log_msg "UUID=${UUID}"

  # Wait for the UUID to update on Apple's side before getting notarization info
  sleep 90
  TIME_WAITED=90

  while true; do
    STATUS=$(xcrun altool --notarization-info "${UUID}" -u "${NOTARIZATION_USER}" -p "${NOTARIZATION_PASS}" 2>&1)
    if [[ ${STATUS} = *"Status: success"* ]]; then log_msg "Notarization Passed"; break; fi
    if [[ ${STATUS} = *"Status: invalid"* ]]; then log_error_and_die "${STATUS}"; fi
    if [ ${TIME_WAITED} -ge ${MAX_WAIT_TIME} ]; then log_error_and_die "Notarization Timed out, waited ${MAX_WAIT_TIME}s"; fi

    log_msg "Waiting for notarization confirmation, waited ${TIME_WAITED}s, remaining $((MAX_WAIT_TIME - TIME_WAITED))s for ${UUID}..."

    TIME_WAITED=$((TIME_WAITED + 30))
    sleep 30
  done
  log_msg "Notarization completed, elapsed time ${TIME_WAITED}s"
}

#
# Notarize a given installer pkg
#
# $1 - Notarization UUID to poll an update for
#
function notarize_pkg_in_dir() {
  local SIGNED_PKG="${1}"
  local PRIMARY_BUNDLE_ID="${2}"

  log_msg "Notarize pkg with args $*"

  log_msg "Uploading ${SIGNED_PKG} to be notarized..."
  STATUS=$( xcrun altool --notarize-app -f "${SIGNED_PKG}" --primary-bundle-id "${PRIMARY_BUNDLE_ID}" --asc-provider Cisco -u "${NOTARIZATION_USER}" -p "${NOTARIZATION_PASS}" 2>&1)
  RETVAL=$?

  if [ $RETVAL -eq 0 ]; then
    wait_for_notarization_complete ${STATUS#*RequestUUID = }

    STATUS=$( xcrun stapler staple "${SIGNED_PKG}" 2>&1)
    RETVAL=$?
    if [ $RETVAL -eq 0 ]; then
      log_msg "Stapling Complete"
      log_msg "${SIGNED_PKG} is notarized by Apple"
      # Recalculate SHA256 after notarization ticket has been stapled
      openssl dgst -sha256 "${SIGNED_PKG}" > "${SIGNED_PKG}.sha256"
    else
      log_error_and_die "${STATUS}"
    fi
  else
    log_error_and_die "${STATUS}"
  fi
}

if [ $# -ne 2 ]; then
    log_msg "Usage: notarize_pkg.sh <signed pkg> <primary bundle id>"
    exit 1
fi

if [ ! -f "${1}" ]; then
    log_msg "Could not find signed install package ${1}"
    exit 1
fi

notarize_pkg_in_dir "${1}" "${2}"
