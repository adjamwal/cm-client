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
BINDIR="${CM_DIR}/bin"
LIBDIR="${CM_DIR}/lib"
CONFIGDIR="${CM_DIR}/etc"
LAUNCHD_DIR="/Library/LaunchDaemons"

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
mkdir -p "${PAYLOAD_STAGING}${LIBDIR}"
mkdir -p "${PAYLOAD_STAGING}${BINDIR}"
mkdir -p "${PAYLOAD_STAGING}${CONFIGDIR}"
mkdir -p "${PAYLOAD_STAGING}${LAUNCHD_DIR}"

cp -f "${STAGING}/client/csccloudmanagement" "${PAYLOAD_STAGING}${BINDIR}"
cp -f "${STAGING}/export/bin/csc_cmid" "${PAYLOAD_STAGING}${BINDIR}"
cp -f "${STAGING}/OSPackageManager/cmpackagemanager" "${PAYLOAD_STAGING}${BINDIR}"

cp -f "${STAGING}/export/lib/libcmidapi.dylib" "${PAYLOAD_STAGING}${LIBDIR}"

cp -f "cm_uninstall.sh" "${PAYLOAD_STAGING}${BINDIR}"

cp -f "com.cisco.secureclient.cloudmanagement.plist" "${PAYLOAD_STAGING}${LAUNCHD_DIR}"

for script in ${SCRIPTS_STAGING}/*; do
    chmod 755 "${script}"
done

#TODO : change "skip_release" to "release" when we get to the stage when we don't require debug info about potential crashes
if [ "$BUILD_TYPE" = "skip_release" ]; then
    rm -rf "${DSYM_STAGING}"
    mkdir -p "${DSYM_STAGING}"
    rm -f "../${DSYM_TAR}"

    dsymutil "${STAGING}/client/csccloudmanagement" -o "${DSYM_STAGING}/csccloudmanagement.dSYM"
    dsymutil "${STAGING}/export/bin/csc_cmid" -o "${DSYM_STAGING}/csc_cmid.dSYM"
    dsymutil "${STAGING}/OSPackageManager/cmpackagemanager" -o "${DSYM_STAGING}/cmpackagemanager.dSYM"
    dsymutil "${STAGING}/export/lib/libcmidapi.dylib" -o "${DSYM_STAGING}/libcmidapi.dylib.dSYM"

    strip "${STAGING}/client/csccloudmanagement"
    strip "${STAGING}/export/bin/csc_cmid"
    strip "${STAGING}/OSPackageManager/cmpackagemanager"
    strip "${STAGING}/export/lib/libcmidapi.dylib" -x 

    cp "${STAGING}/client/csccloudmanagement" "${STAGING}/export/bin/csc_cmid" "${STAGING}/OSPackageManager/cmpackagemanager" "${STAGING}/export/lib/libcmidapi.dylib" "${DSYM_STAGING}"

    tar czf "../${DSYM_TAR}" "${DSYM_STAGING}"
    cp "${DSYM_STAGING}/${DSYM_TAR}" "../${BUILD_STAGING_DIR}"
fi

install_name_tool -change "@rpath/libcmidapi.dylib" "@executable_path/../lib/libcmidapi.dylib" "${PAYLOAD_STAGING}${BINDIR}/cmpackagemanager"

pkgbuild    --root "${PAYLOAD_STAGING}" \
            --scripts "${SCRIPTS_STAGING}" \
            --identifier "${CM_PACKAGE_ID}" \
            --version "${VER}" \
            --install-location "/" \
            --ownership recommended \
            "${CM_PKG}"

productbuild    --distribution "cm_distribution.xml" \
                --package-path "./" \
                "${CM_PKG_UNSIGNED}"

rm -rf "${DMG_STAGING}"
rm -f "${CM_DMG}"
mkdir -p "${DMG_STAGING}"

#TODO : Signing the package

mv "${CM_PKG_UNSIGNED}" "${CM_INSTALLER}"

cp -f "../client/config/bs.json" "${DMG_STAGING}/.bs.json"
cp -f "../client/config/cm_config.json" "${DMG_STAGING}/.cm_config.json"

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
