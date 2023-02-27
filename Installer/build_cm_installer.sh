#!/bin/bash

set -e

if [ "x$CM_BUILD_VER" = "x" ]; then
    echo "CM_BUILD_VER not set. exiting..."
    exit 1
fi

VER=${CM_BUILD_VER}
if [ "x$1" = "xdebug" ]; then
    BUILD_TYPE="debug"
else
    BUILD_TYPE="release"
fi

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

CM_PACKAGE_ID="com.cisco.secureclient.cloudmanagement"
CM_VOLUME="Cisco Secure Client - Cloud Management ${VER}"
CM_PKG="cisco-secure-client-macos-cloudmanagement.pkg"
CM_PKG_UNSIGNED="unsigned-${CM_PKG}"
CM_INSTALLER="cisco-secure-client-macos-cloudmanagement-${VER}.pkg"
CM_DMG="cisco-secure-client-macos-cloudmanagement-${VER}.dmg"

STAGING="../${BUILD_TYPE}"
SCRIPTS_STAGING="cm_pkg_scripts"
PAYLOAD_STAGING="cm_pkg_payload"
DMG_STAGING="cm_dmg"
BUILD_STAGING_DIR="Staging"

DSYM_STAGING="cisco-secure-client-macos-cloudmanagement-${VER}-symbols"
DSYM_TAR="cisco-secure-client-macos-cloudmanagement-${VER}-symbols.tar.gz"

echo "creating CM package Payload Staging Area"

rm -rf "${PAYLOAD_STAGING}"
mkdir -p "${PAYLOAD_STAGING}${LIB_DIR}"
mkdir -p "${PAYLOAD_STAGING}${BIN_DIR}"
mkdir -p "${PAYLOAD_STAGING}${CONFIG_DIR}"
mkdir -p "${PAYLOAD_STAGING}${LAUNCHD_DIR}"

cp -f "${STAGING}/client/${CM_BINARY}" "${PAYLOAD_STAGING}${BIN_DIR}"
cp -f "${STAGING}/export/bin/${CMID_BINARY}" "${PAYLOAD_STAGING}${BIN_DIR}"
cp -f "${STAGING}/OSPackageManager/${PM_BINARY}" "${PAYLOAD_STAGING}${BIN_DIR}"

cp -f "${STAGING}/export/lib/${CMID_LIBRARY}" "${PAYLOAD_STAGING}${LIB_DIR}"

cp -f "${UNINSTALL_SCRIPT}" "${PAYLOAD_STAGING}${BIN_DIR}"

cp -f "${CM_PLIST}" "${PAYLOAD_STAGING}${LAUNCHD_DIR}"

for script in ${SCRIPTS_STAGING}/*; do
    chmod 755 "${script}"
done

#TODO : change "skip_release" to "release" when we get to the stage when we don't require debug info about potential crashes
if [ "$BUILD_TYPE" = "skip_release" ]; then
    rm -rf "${DSYM_STAGING}"
    mkdir -p "${DSYM_STAGING}"
    rm -f "../${DSYM_TAR}"

    dsymutil "${STAGING}/client/${CM_BINARY}" -o "${DSYM_STAGING}/${CM_BINARY}.dSYM"
    dsymutil "${STAGING}/export/bin/${CMID_BINARY}" -o "${DSYM_STAGING}/${CM_BINARY}.dSYM"
    dsymutil "${STAGING}/OSPackageManager/${PM_BINARY}" -o "${DSYM_STAGING}/${PM_BINARY}.dSYM"
    dsymutil "${STAGING}/export/lib/${CMID_LIBRARY}" -o "${DSYM_STAGING}/${CMID_LIBRARY}.dSYM"

    strip "${STAGING}/client/${CM_BINARY}"
    strip "${STAGING}/export/bin/${CMID_BINARY}"
    strip "${STAGING}/OSPackageManager/${PM_BINARY}"
    strip "${STAGING}/export/lib/${CMID_LIBRARY}" -x 

    cp "${STAGING}/client/${CM_BINARY}" "${STAGING}/export/bin/${CMID_BINARY}" "${STAGING}/OSPackageManager/${PM_BINARY}" "${STAGING}/export/lib/${CMID_LIBRARY}" "${DSYM_STAGING}"

    tar czf "../${BUILD_STAGING_DIR}/${DSYM_TAR}" "${DSYM_STAGING}"
fi

install_name_tool -change "@rpath/${CMID_LIBRARY}" "@executable_path/../lib/${CMID_LIBRARY}" "${PAYLOAD_STAGING}${BIN_DIR}/${PM_BINARY}"

pkgbuild    --root "${PAYLOAD_STAGING}" \
            --scripts "${SCRIPTS_STAGING}" \
            --identifier "${CM_PACKAGE_ID}" \
            --version "${VER}" \
            --install-location "/" \
            --ownership recommended \
            "${CM_PKG}"

productbuild    --distribution "${CM_DISTRIBUTION}" \
                --package-path "./" \
                "${CM_PKG_UNSIGNED}"

rm -rf "${DMG_STAGING}"
rm -f "${CM_DMG}"
mkdir -p "${DMG_STAGING}"

#TODO : Signing the package

mv "${CM_PKG_UNSIGNED}" "${CM_INSTALLER}"

cp -f "../client/config/${BOOTSTRAP_FILE}" "${DMG_STAGING}/.${BOOTSTRAP_FILE}"
cp -f "../client/config/${CONFIG_FILE}" "${DMG_STAGING}/.${CONFIG_FILE}"

cp -f "${CM_INSTALLER}" "${DMG_STAGING}"
hdiutil create  -ov \
                -format UDZO \
                -srcfolder "${DMG_STAGING}"\
                -layout NONE \
                -fs HFS+ \
                -volname "${CM_VOLUME}" \
                "${CM_DMG}" 

rm -rf "${PAYLOAD_STAGING}"
rm -rf "${DSYM_STAGING}"
rm -rf "${DMG_STAGING}"
rm -f "${CM_PKG}"
rm -f "${CM_INSTALLER}"

echo "Copying CM Installer to Staging"
cp "${CM_DMG}" "../${BUILD_STAGING_DIR}"
