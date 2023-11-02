#!/bin/bash -e

#
# ci_coverity_analyze.sh
# =============
#
# Build AMPKIT under coverity and report unhandled issues.
#
# Usage
# =====
#
# ci_coverity_analyze.sh
#
# User Environment Variables (All must be present)
# ================================================
#
# COV_PLATFORM_URL                    Coverity web platform URL: coverity-platform-server.clg5.amp.ciscolabs.com
# COV_PLATFORM_USERNAME               Coverity web platform username.
# COV_PLATFORM_PASSWORD               Coverity web platform password.
# COV_PLATFORM_STREAM                 Coverity stream to record results: cloudmanagement-${platform}
#
# User Environment Variables (Optional)
# ================================================
#
# COV_PLATFORM_PUBLISH                Set to "yes" to publish results to coverity.
#
# Jenkins Environment Variables
# =============================
#
# WORKSPACE                           Root directory of the current job.
#

COVERITY_INSTALL_DIR=/Applications/cov-analysis-macosx-2022.12.1
COVERITY_BIN_DIR=${COVERITY_INSTALL_DIR}/bin

BUILD_ROOT=${WORKSPACE}/cm-client
WORKSPACE_ROOT=${WORKSPACE}
COVERITY_REPORT_ARCHIVE=${WORKSPACE}/coverity_report.tar.bz2

COVERITY_INTERMEDIATE_DIR=${WORKSPACE_ROOT}/coverity/macos_coverity_intermediate
COVERITY_HTML_DIR=${WORKSPACE_ROOT}/coverity/macos_coverity_html_output

COV_PLATFORM_URL="rtp-cov-cim-03.cisco.com"
COV_PLATFORM_STREAM="cloudmanagement-macos"

COV_PLATFORM_USERNAME="${COV_USER}"
COV_PLATFORM_PASSWORD="${COV_PASSWORD}"
COV_PLATFORM_PUBLISH=yes

function log_msg() {
    local MSG="$*"
    printf "%s\n" "${MSG}"
}

function log_error_exit() {
    local MSG="$*"
    echo "[error] ${MSG}" >&2
    exit 1
}

function log_info() {
    local MSG="$*"
    log_msg "[info] ${MSG}"
}

function cov_configure() {
    ${COVERITY_BIN_DIR}/cov-configure --clang --xml-option=skip_file:"/Applications/" --xml-option=skip_file:"/tests/" --xml-option=skip_file:"/xcode_debug/" --config coverity/cov_config.xml
}

function cov_build() {
    mkdir -p "${COVERITY_INTERMEDIATE_DIR}"
    pushd ${BUILD_ROOT}
	./scripts/build.sh -x -d
        ${COVERITY_BIN_DIR}/cov-build --config coverity/cov_config.xml --dir "${COVERITY_INTERMEDIATE_DIR}" xcodebuild build -project ${BUILD_ROOT}/xcode_debug/cm-client.xcodeproj -configuration "Debug" -scheme ALL_BUILD
    popd
}

function cov_analyze() {
    ${COVERITY_BIN_DIR}/cov-analyze --dir "${COVERITY_INTERMEDIATE_DIR}" \
                                        --strip-path "${WORKSPACE_ROOT}"
}

function cov_html() {
    ${COVERITY_BIN_DIR}/cov-format-errors --dir "${COVERITY_INTERMEDIATE_DIR}" \
                                              --strip-path "${WORKSPACE_ROOT}" \
                                              --html-output "${COVERITY_HTML_DIR}"
}

function generate_html_archive() {
    # Produce HTML output
    mkdir -p "${COVERITY_HTML_DIR}"
    cov_html
    # Compress HTML output for archiving
    pushd "${WORKSPACE}"
    tar cvfj "${COVERITY_REPORT_ARCHIVE}" "${COVERITY_HTML_DIR}"
    popd
}

function publish_results() {
    if [ "${COV_PLATFORM_PUBLISH}" = "yes" ]; then
        log_info "Publishing coverity results to ${COV_PLATFORM_URL}"
        # Publish the report to coverity host
        ${COVERITY_BIN_DIR}/cov-commit-defects --url "https://${COV_PLATFORM_URL}:6010" \
                                                --stream "${COV_PLATFORM_STREAM}" \
                                                --user "${COV_PLATFORM_USERNAME}" \
                                                --password "${COV_PLATFORM_PASSWORD}" \
                                                --dir "${COVERITY_INTERMEDIATE_DIR}" \
                                                --exclude-files "/Applications/"
    fi
}

# Sanity checks
[ -z "${COV_PLATFORM_URL}" ] && log_error_exit "Missing COV_PLATFORM_URL env var"
[ -z "${COV_PLATFORM_USERNAME}" ] && log_error_exit "Missing COV_PLATFORM_USERNAME env var"
[ -z "${COV_PLATFORM_PASSWORD}" ] && log_error_exit "Missing COV_PLATFORM_PASSWORD env var"
[ -z "${COV_PLATFORM_STREAM}" ] && log_error_exit "Missing COV_PLATFORM_STREAM env var"
[ -z "${WORKSPACE}" ] && log_error_exit "Missing WORKSPACE env var"

# we don't need to configure it each time cov_configure (once per each build node)
cov_configure
cov_build
cov_analyze

# we won't need generate_html_archive, results can be found on the portal
#generate_html_archive

# Results published by Jenkins through the Coverity plug-in
# - Uncomment to explicitly publish through this script ($COV_PLATFORM_PUBLISH must be set to "yes")
publish_results

# cleanup intermediate directory after commit was made (optional space saving step)
# rm -rf ${COVERITY_INTERMEDIATE_DIR}
