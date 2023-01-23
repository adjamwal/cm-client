#!/bin/bash

if [ "x$CM_BUILD_VER" = "x" ]; then
	CM_BUILD_VER="1.0.0000"
fi

VER=${CM_BUILD_VER}
if [ "x$1" = "debug" ]; then
	BUILD_TYPE="debug"
else
	BUILD_TYPE="release"
fi

PREFIX="/opt/cisco/secureclient"
CM_DIR="${PREFIX}/cloudmanagement"
BINDIR="${CM_DIR}/bin"
LIBDIR="${CM_DIR}/lib"
CONFIGDIR="${CM_DIR}/etc"
LAUNCHD_DIR="/Library/LaunchDaemons"

CM_PACKAGE_ID=com.cisco.secureclient.cloudmanagement
CM_VOLUME="Cisco Secure Client - Cloud Management ${VER}"
CM_INSTALLER="cisco-secure-client-macos-cloudmanagement-${VER}.pkg"
CM_DMG="cisco-secure-client-macos-cloudmanagement-${VER}.dmg"

STAGING="../${BUILD_TYPE}"
SCRIPTS_STAGING="./cm_pkg_scripts"
PAYLOAD_STAGING="./cm_pkg_payload"
DMG_STAGING="./cm_dmg"

echo "creating CM package Payload Staging Area"

rm -rf ${PAYLOAD_STAGING}
mkdir -p ${PAYLOAD_STAGING}${LIBDIR}
mkdir -p ${PAYLOAD_STAGING}${BINDIR}
mkdir -p ${PAYLOAD_STAGING}${CONFIGDIR}
mkdir -p ${PAYLOAD_STAGING}${LAUNCHD_DIR}

cp -f ${STAGING}/client/csccloudmanagement ${PAYLOAD_STAGING}${BINDIR}/ || exit 1
cp -f ${STAGING}/export/bin/csc_cmid ${PAYLOAD_STAGING}${BINDIR}/ || exit 1

cp -f ${STAGING}/export/lib/libcmidapi.dylib ${PAYLOAD_STAGING}${LIBDIR}/ || exit 1

cp -f ../client/config/bs.json ${PAYLOAD_STAGING}${CONFIGDIR}/ || exit 1
cp -f ../client/config/cm_config.json ${PAYLOAD_STAGING}${CONFIGDIR}/ || exit 1

cp -f cm_uninstall.sh ${PAYLOAD_STAGING}${BINDIR} || exit 1

cp -f com.cisco.secureclient.cloudmanagement.plist ${PAYLOAD_STAGING}${LAUNCHD_DIR}/ || exit 1

for script in ${SCRIPTS_STAGING}/*; do
	chmod 755 "${script}" || exit 1
done

pkgbuild --root ${PAYLOAD_STAGING} --scripts ${SCRIPTS_STAGING} --identifier ${CM_PACKAGE_ID} --version ${VER} --install-location "/" ${CM_INSTALLER} --ownership recommended || exit 1
productbuild --distribution cm_distribution.xml --package-path "./" "unsigned ${CM_INSTALLER}" || exit 1

mkdir -p ${DMG_STAGING}

#TODO : Signing

cp -f "unsigned ${CM_INSTALLER}" ${DMG_STAGING}/ || exit 1
hdiutil create -ov -format UDZO -srcfolder ${DMG_STAGING} ${CM_DMG} -layout NONE -fs HFS+ -volname "${CM_VOLUME}" || exit 1

rm -rf ${PAYLOAD_STAGING}