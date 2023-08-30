#!/bin/bash -e
#
# Usage: build_cm_installer.sh [debug|release] [staging_dir]
#
# Optional User Environment Variables
# ===================================
#
# RETAIN_SYMBOLS	Set to "yes" to retain symbols when building a release build
# DEV_ID_APP_CERT	Developer ID Application Certificate
# DEV_ID_INSTALL_CERT	Developer ID Installer Certificate
#
# Required User Environment Variables Needed by Notarization Script
# =================================================================
#
# NOTARIZATION_USER             The username used for notarization
#                               of signed pkg files
# NOTARIZATION_PASS             The password used for notarization
#                               of signed pkg files
# Note, 'cisco-secure-client-macos-cloudmanagement' in SAMPLE_PREFIX is vital
# at the moment, as wildcard is used on Jenkins job to upload artefacts.
# When we are ready to upgrade the naming, scripts/jenkins/Release.Jenksinsfile
# has to be updated also to support that.

SAMPLE_PREFIX="cisco-secure-client-macos-cloudmanagement-testpackage1"
SAMPLE_PKG_NAME="${SAMPLE_PREFIX}.pkg"

echo "Creating ${SAMPLE_PKG_NAME}"

if [ $# -ne 2 ]; then
    echo "Usage: build_sample_installer.sh [debug|release] [staging_dir]"
fi

VER=${CM_BUILD_VER}
if [ "x$1" = "xdebug" ]; then
    BUILD_TYPE="debug"
else
    BUILD_TYPE="release"
fi

BUILD_STAGING_DIR="${2}"
echo "Creating build Staging directory"
mkdir -p "${BUILD_STAGING_DIR}"

#
# ---------------------------------------

CM_DIR="/opt/cisco/secureclient/cloudmanagement_testpackage1"
SAMPLE_BIN_DIR="${CM_DIR}/sample_bin"

SAMPLE_BINARY="csccloudmanagement"
CM_DISTRIBUTION="sample_distribution.xml"

SAMPLE_PACKAGE_ID="com.cisco.secureclient.testpackage1"
SAMPLE_PKG_PATH="smp_pkg_tmp"
SAMPLE_PKG="${SAMPLE_PKG_PATH}/${SAMPLE_PKG_NAME}"
SAMPLE_PKG_UNSIGNED="unsigned-${SAMPLE_PKG_NAME}"
SAMPLE_INSTALLER="${SAMPLE_PREFIX}-${VER}.pkg"

STAGING="../${BUILD_TYPE}"
STAGING_EXPORT="${STAGING}/export"
STAGING_EXPORT_BIN="${STAGING_EXPORT}/bin"
PAYLOAD_STAGING="sample_pkg_payload"

CM_BINARIES=(
    "${SAMPLE_BINARY}"
)

#
# ---------------------------------------

clear_and_recreate_payload_staging()
{
    echo "creating Sample package Payload Staging Area"

    rm -rf "${PAYLOAD_STAGING}"
    mkdir -p "${PAYLOAD_STAGING}${SAMPLE_BIN_DIR}"
}

copy_and_prepare_staging()
{
    clear_and_recreate_payload_staging

    if [ -z "${DEV_ID_APP_CERT}" ]; then
        echo "Developer ID Application certificate is not specified, code will not be signed"
    fi

    for exe in "${CM_BINARIES[@]}"; do
        cp -f "${STAGING_EXPORT_BIN}/${exe}" "${PAYLOAD_STAGING}${SAMPLE_BIN_DIR}"
        if [ -n "${DEV_ID_APP_CERT}" ]; then
            echo "codesigning ${exe} with ${DEV_ID_APP_CERT}"
            codesign --timestamp --verbose --force --deep --options runtime --sign "${DEV_ID_APP_CERT}" ${PAYLOAD_STAGING}/${SAMPLE_BIN_DIR}/${exe}
        fi
    done
}

create_pkg_from_staging()
{
    rm -fr "${SAMPLE_PKG_PATH}"
    mkdir -p "${SAMPLE_PKG_PATH}"

    pkgbuild    --root "${PAYLOAD_STAGING}" \
                --identifier "${SAMPLE_PACKAGE_ID}" \
                --version "${VER}" \
                --install-location "/" \
                --ownership recommended \
                "${SAMPLE_PKG}"

    productbuild    --distribution "${CM_DISTRIBUTION}" \
                    --package-path "${SAMPLE_PKG_PATH}" \
                    --version "${VER}" \
                    "${SAMPLE_PKG_UNSIGNED}"

    rm -rf "${PAYLOAD_STAGING}"
    rm -rf "${SAMPLE_PKG_PATH}"
    # Resides in SAMPLE_PKG_PATH (should be removed by line above)
    rm -f "${SAMPLE_PKG}"

    if [ -n "${DEV_ID_INSTALL_CERT}" ]; then
        productsign --sign "${DEV_ID_INSTALL_CERT}" \
                           "${SAMPLE_PKG_UNSIGNED}" \
                           "${SAMPLE_INSTALLER}"
        echo "** Package ${SAMPLE_INSTALLER} is signed ${DEV_ID_INSTALL_CERT}"
        rm "${SAMPLE_PKG_UNSIGNED}"

        # Expectation is this script is run relative to this directory
        ./notarize_pkg.sh "${SAMPLE_INSTALLER}" "${SAMPLE_PACKAGE_ID}"

        # Keep a copy of the notarized PKG
        cp "${SAMPLE_INSTALLER}" "${BUILD_STAGING_DIR}"
    else
        echo "Installer signing certificate is not defined, package is not signed"
        mv "${SAMPLE_PKG_UNSIGNED}" "${SAMPLE_INSTALLER}"
    fi
}

create_dmg_from_pkg_and_config()
{
    rm -f "${SAMPLE_INSTALLER}"
}

#
# ---------------------------------------

copy_and_prepare_staging

create_pkg_from_staging
