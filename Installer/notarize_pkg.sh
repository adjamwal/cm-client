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

MAX_WAIT_TIME=28800

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
# Notarize a given installer pkg
#
# $1 - Notarization UUID to poll an update for
#
function notarize_pkg_in_dir() {
  local SIGNED_PKG="${1}"
  local PRIMARY_BUNDLE_ID="${2}"

  log_msg "Notarize pkg with args $*"

  log_msg "Uploading ${SIGNED_PKG} to be notarized..."
  STATUS=$(xcrun notarytool submit --wait --timeout "${MAX_WAIT_TIME}" --apple-id "${NOTARIZATION_USER}" --password "${NOTARIZATION_PASS}" --team-id "${TEAM_ID}" "${SIGNED_PKG}" 2>&1)
  RETVAL=$?
  
  if [[ ${STATUS} = *"Timeout"* ]]; then log_error_and_die "Notarization Timed out, waited ${MAX_WAIT_TIME}s"; fi

  if [ $RETVAL -eq 0 ]; then
    if [[ ${STATUS} = *"status: Accepted"* ]]; then log_msg "Notarization Passed"; fi
    if [[ ${STATUS} = *"status: Invalid"* ]]; then log_error_and_die "${STATUS}"; fi
     
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
