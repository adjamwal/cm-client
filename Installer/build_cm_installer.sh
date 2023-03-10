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

if [ "x$CM_BUILD_VER" = "x" ]; then
    echo "CM_BUILD_VER not set. exiting..."
    exit 1
fi

if [ $# -ne 2 ]; then
    echo "Usage: build_cm_installer.sh [debug|release] [staging_dir]"
fi

VER=${CM_BUILD_VER}
if [ "x$1" = "xdebug" ]; then
    BUILD_TYPE="debug"
else
    BUILD_TYPE="release"
fi

BUILD_STAGING_DIR="${2}"
if [ -d "${BUILD_STAGING_DIR}" ]; then
    echo "Removing existing build Staging directory"
    rm -rf "${BUILD_STAGING_DIR}"
fi
echo "Creating build Staging directory"
mkdir -p "${BUILD_STAGING_DIR}"

#
# ---------------------------------------

CM_DIR="/opt/cisco/secureclient/cloudmanagement"
BIN_DIR="${CM_DIR}/bin"
LIB_DIR="${CM_DIR}/lib"
CONFIG_DIR="${CM_DIR}/etc"
LAUNCHD_DIR="/Library/LaunchDaemons"

CM_BINARY="csccloudmanagement"
CMID_BINARY="csc_cmid"
CMID_LIBRARY="libcmidapi.dylib"
PM_BINARY="cmpackagemanager"
UNINSTALL_SCRIPT="cm_uninstall.sh"
CM_PLIST="com.cisco.secureclient.cloudmanagement.plist"
CM_DISTRIBUTION="cm_distribution.xml"
BOOTSTRAP_FILE="bs.json"
CONFIG_FILE="cm_config.json"

CM_PREFIX="cisco-secure-client-macos-cloudmanagement"
CM_PACKAGE_ID="com.cisco.secureclient.cloudmanagement"
CM_PKG_PATH="cm_pkg_tmp"
CM_PKG_NAME="${CM_PREFIX}.pkg"
CM_PKG="${CM_PKG_PATH}/${CM_PKG_NAME}"
CM_PKG_UNSIGNED="unsigned-${CM_PKG_NAME}"
CM_INSTALLER="${CM_PREFIX}-${VER}.pkg"
CM_DMG="${CM_PREFIX}-${VER}.dmg"

STAGING="../${BUILD_TYPE}"
STAGING_EXPORT="${STAGING}/export"
STAGING_EXPORT_BIN="${STAGING_EXPORT}/bin"
STAGING_EXPORT_LIB="${STAGING_EXPORT}/lib"
SCRIPTS_STAGING="cm_pkg_scripts"
PAYLOAD_STAGING="cm_pkg_payload"

CONFIG_RELATIVE_PATH="../client/config"

DSYM_STAGING="${CM_PREFIX}-${VER}-symbols"
DSYM_TAR="${CM_PREFIX}-${VER}-symbols.tgz"

CM_BINARIES=(
    "${CM_BINARY}"
    "${CMID_BINARY}"
    "${PM_BINARY}"
)

#
# ---------------------------------------

clear_and_recreate_payload_staging()
{
    echo "creating CM package Payload Staging Area"

    rm -rf "${PAYLOAD_STAGING}"
    mkdir -p "${PAYLOAD_STAGING}${LIB_DIR}"
    mkdir -p "${PAYLOAD_STAGING}${BIN_DIR}"
    mkdir -p "${PAYLOAD_STAGING}${CONFIG_DIR}"
    mkdir -p "${PAYLOAD_STAGING}${LAUNCHD_DIR}"
}

strip_and_archive_symbols()
{
    #
    # Retaining symbols is not strictly necessary as you can load the generated
    # dsym into the debugger or use it to symbolicate crashes.  However, if
    # for some reason one wishes to avoid the extra step, then defining
    # RETAIN_SYMBOLS allows us to do that
    if [ "${RETAIN_SYMBOLS}" != "yes" ] && [ "$BUILD_TYPE" = "release" ]; then
        rm -rf "${DSYM_STAGING}"
        mkdir -p "${DSYM_STAGING}"
        rm -f "../${DSYM_TAR}"

        for exe in "${CM_BINARIES[@]}"; do
            if [ "${exe}" = "${CMID_BINARY}" ]; then
                echo "----------- Skipping ${exe} (already stripped...)"
            else
                echo "----------- Stripping ${exe}"
                dsymutil "${STAGING_EXPORT_BIN}/${exe}" -o "${DSYM_STAGING}/${exe}.dSYM"
                strip "${STAGING_EXPORT_BIN}/${exe}"
            fi
        done
        #
        # CMID Library already stripped, skipping...
        #echo "----------- Stripping ${CMID_LIBRARY}"
        #dsymutil "${STAGING_EXPORT_LIB}/${CMID_LIBRARY}" -o "${DSYM_STAGING}/${CMID_LIBRARY}.dSYM"
        #strip -x "${STAGING_EXPORT_LIB}/${CMID_LIBRARY}"

        tar czf "${BUILD_STAGING_DIR}/${DSYM_TAR}" "${DSYM_STAGING}"

        rm -rf "${DSYM_STAGING}"
    fi
}

add_fingerprint() {
    local REPO_PATH="${1}"
    local GIT_FP_FILE="${2}"
    local REPO_NAME
    REPO_NAME=$(basename "${REPO_PATH}")

    pushd "${REPO_PATH}" > /dev/null
        echo "${REPO_NAME} =======FP=============" >> "${GIT_FP_FILE}"
        COMMIT=$(git log | head -1)
        echo "${COMMIT}" >> "${GIT_FP_FILE}"
    popd > /dev/null
}

write_git_fingerprints() {
    local GIT_FP_FNAME="GIT_FP.txt"
    local GIT_FP_FILE="${BUILD_STAGING_DIR}/${GIT_FP_FNAME}"

    rm -f "${GIT_FP_FILE}"

    #
    # This script is run relative to the Installer directory
    add_fingerprint "../../cm-client" "/${GIT_FP_FILE}"
    add_fingerprint "../EndpointIdentity" "/${GIT_FP_FILE}"
    add_fingerprint "../PackageManager" "/${GIT_FP_FILE}"
    add_fingerprint "../third-party/ciscossl" "/${GIT_FP_FILE}"
    add_fingerprint "../third-party/curl" "/${GIT_FP_FILE}"
    add_fingerprint "../third-party/jsoncpp" "/${GIT_FP_FILE}"
    add_fingerprint "../third-party/spdlog" "/${GIT_FP_FILE}"
}

copy_and_prepare_staging()
{
    clear_and_recreate_payload_staging

    if [ -z "${DEV_ID_APP_CERT}" ]; then
        echo "Developer ID Application certificate is not specified, code will not be signed"
    fi

    for exe in "${CM_BINARIES[@]}"; do
        cp -f "${STAGING_EXPORT_BIN}/${exe}" "${PAYLOAD_STAGING}${BIN_DIR}"
        if [ "${exe}" = "${PM_BINARY}" ]; then
            install_name_tool -change "@rpath/${CMID_LIBRARY}" "@executable_path/../lib/${CMID_LIBRARY}" "${PAYLOAD_STAGING}/${BIN_DIR}/${exe}"
        fi
        if [ -n "${DEV_ID_APP_CERT}" ]; then
            echo "codesigning ${exe} with ${DEV_ID_APP_CERT}"
            codesign --timestamp --verbose --force --deep --options runtime --sign "${DEV_ID_APP_CERT}" ${PAYLOAD_STAGING}/${BIN_DIR}/${exe}
        fi
    done
    cp -f "${STAGING_EXPORT_LIB}/${CMID_LIBRARY}" "${PAYLOAD_STAGING}/${LIB_DIR}"
    if [ -n "${DEV_ID_APP_CERT}" ]; then
        echo "codesigning ${CMID_LIBRARY} with ${DEV_ID_APP_CERT}"
        codesign --timestamp --verbose --force --deep --options runtime --sign "${DEV_ID_APP_CERT}" "${PAYLOAD_STAGING}/${LIB_DIR}/${CMID_LIBRARY}"
    fi
    cp -f "${UNINSTALL_SCRIPT}" "${PAYLOAD_STAGING}${BIN_DIR}"

    cp -f "${CM_PLIST}" "${PAYLOAD_STAGING}${LAUNCHD_DIR}"

    chmod 755 "${SCRIPTS_STAGING}"/{pre,post}install
}

create_pkg_from_staging()
{
    rm -fr "${CM_PKG_PATH}"
    mkdir -p "${CM_PKG_PATH}"

    pkgbuild    --root "${PAYLOAD_STAGING}" \
                --scripts "${SCRIPTS_STAGING}" \
                --identifier "${CM_PACKAGE_ID}" \
                --version "${VER}" \
                --install-location "/" \
                --ownership recommended \
                "${CM_PKG}"

    productbuild    --distribution "${CM_DISTRIBUTION}" \
                    --package-path "${CM_PKG_PATH}" \
                    --version "${VER}" \
                    "${CM_PKG_UNSIGNED}"

    rm -rf "${PAYLOAD_STAGING}"
    rm -rf "${CM_PKG_PATH}"
    # Resides in CM_PKG_PATH (should be removed by line above)
    #rm -f "${CM_PKG}"

    if [ -n "${DEV_ID_INSTALL_CERT}" ]; then
        productsign --sign "${DEV_ID_INSTALL_CERT}" \
                           "${CM_PKG_UNSIGNED}" \
                           "${CM_INSTALLER}"
        echo "** Package ${CM_INSTALLER} is signed ${DEV_ID_INSTALL_CERT}"
        rm "${CM_PKG_UNSIGNED}"

        # Expectation is this script is run relative to this directory
        ./notarize_pkg.sh "${CM_INSTALLER}" "${CM_PACKAGE_ID}"

        # Keep a copy of the notarized PKG
        cp "${CM_INSTALLER}" "${BUILD_STAGING_DIR}"
    else
        echo "Installer signing certificate is not defined, package is not signed"
        mv "${CM_PKG_UNSIGNED}" "${CM_INSTALLER}"
    fi
}

create_dmg_from_pkg_and_config()
{
    rm -f "${CM_DMG}"

    ./dmg_resources/mkdmg.sh "${CM_DMG}" \
             "${CONFIG_RELATIVE_PATH}/${BOOTSTRAP_FILE}" \
             "${CONFIG_RELATIVE_PATH}/${CONFIG_FILE}" \
             "${CM_INSTALLER}"

    rm -f "${CM_INSTALLER}"
}

#
# ---------------------------------------

strip_and_archive_symbols

copy_and_prepare_staging

create_pkg_from_staging

create_dmg_from_pkg_and_config

write_git_fingerprints

echo "Created installer ${BUILD_STAGING_DIR}/${CM_DMG}"
mv "${CM_DMG}" "${BUILD_STAGING_DIR}"
