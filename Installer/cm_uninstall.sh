#!/bin/bash

if [ $(id -u) -ne 0 ]; then
    echo "You need super user privileges to run this script."
    exit 1
fi

CM_DIR="/opt/cisco/secureclient/cloudmanagement"
BIN_DIR="${CM_DIR}/bin"
LAUNCHD_DIR="/Library/LaunchDaemons"
LAUNCHD_FILE="com.cisco.secureclient.cloudmanagement.plist"
CM_PACKAGE_ID="com.cisco.secureclient.cloudmanagement"

echo "Uninstalling Cisco Secure Client CloudManagement ..."

echo "Stopping csccloudmanagement ... "
launchctl bootout system ${LAUNCHD_DIR}/${LAUNCHD_FILE}

# ensure that CM is not running
CMPROC=`ps -A -o pid,command | grep '(${BIN_DIR}/csccloudmanagement)' | egrep -v 'grep|cm_uninstall' | awk '{print $1}'`
if [ ! "x${CMPROC}" = "x" ] ; then
    echo Killing `ps -A -o pid,command -p ${CMPROC} | grep ${CMPROC} | egrep -v 'ps|grep'`
    kill -KILL ${CMPROC}
fi

echo "Removing files ..."

#Remove launchd file if exists
if [ -e "${LAUNCHD_DIR}/${LAUNCHD_FILE}" ] ; then
    rm -rf "${LAUNCHD_DIR}/${LAUNCHD_FILE}"
fi

rm -rf "${CM_DIR}" || exit

pkgutil --forget "${CM_PACKAGE_ID}"

echo "Successfully uninstalled Cisco Secure Client CloudManagement."

exit 0