#!/bin/bash

set -e

if [ "x$CM_BUILD_VER" = "x" ]; then
    CM_BUILD_VER="1.0.0000"
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

CM_PACKAGE_ID=com.cisco.secureclient.cloudmanagement
CM_VOLUME="Cisco Secure Client - Cloud Management ${VER}"
CM_PKG="cisco-secure-client-macos-cloudmanagement.pkg"
CM_INSTALLER="cisco-secure-client-macos-cloudmanagement-${VER}.pkg"
CM_DMG="cisco-secure-client-macos-cloudmanagement-${VER}.dmg"

STAGING="../${BUILD_TYPE}"
SCRIPTS_STAGING="cm_pkg_scripts"
PAYLOAD_STAGING="cm_pkg_payload"
DMG_STAGING="cm_dmg"

echo "creating CM package Payload Staging Area"

rm -rf ${PAYLOAD_STAGING}
mkdir -p ${PAYLOAD_STAGING}${LIBDIR}
mkdir -p ${PAYLOAD_STAGING}${BINDIR}
mkdir -p ${PAYLOAD_STAGING}${CONFIGDIR}
mkdir -p ${PAYLOAD_STAGING}${LAUNCHD_DIR}

cp -f ${STAGING}/client/csccloudmanagement ${PAYLOAD_STAGING}${BINDIR}
cp -f ${STAGING}/export/bin/csc_cmid ${PAYLOAD_STAGING}${BINDIR}

cp -f ${STAGING}/export/lib/libcmidapi.dylib ${PAYLOAD_STAGING}${LIBDIR}

cp -f cm_uninstall.sh ${PAYLOAD_STAGING}${BINDIR}

cp -f com.cisco.secureclient.cloudmanagement.plist ${PAYLOAD_STAGING}${LAUNCHD_DIR}

for script in ${SCRIPTS_STAGING}/*; do
    chmod 755 "${script}"
done

pkgbuild --root ${PAYLOAD_STAGING} --scripts ${SCRIPTS_STAGING} --identifier ${CM_PACKAGE_ID} --version ${VER} --install-location "/" ${CM_PKG} --ownership recommended
productbuild --distribution cm_distribution.xml --package-path "./" "unsigned ${CM_PKG}"

rm -rf ${DMG_STAGING}
rm -f ${CM_DMG}
mkdir -p ${DMG_STAGING}

#TODO : Signing

mv "unsigned ${CM_PKG}" "${CM_INSTALLER}"

cp -f ../client/config/bs.json ${DMG_STAGING}/.bs.json
cp -f ../client/config/cm_config.json ${DMG_STAGING}/.cm_config.json

cp -f ${CM_INSTALLER} ${DMG_STAGING}
hdiutil create -ov -format UDZO -srcfolder ${DMG_STAGING} ${CM_DMG} -layout NONE -fs HFS+ -volname "${CM_VOLUME}"

rm -rf ${PAYLOAD_STAGING}
rm -rf ${DMG_STAGING}
rm -f ${CM_PKG}
rm -f ${CM_INSTALLER}