# Defines CPack variables and logic flow for generating a package.
# Copyright Cisco Systems, Inc. 2024

include(helper/determine_platform_derivatives)

# Set CPack metadata variables.
set(CPACK_PACKAGE_NAME "cisco-secure-client-cloudmanagement")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cisco Cloud Management for Linux")
set(CPACK_PACKAGE_VENDOR "Cisco")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.cisco.com/site/us/en/products/security/secure-client/index.html")
set(CPACK_PACKAGE_VERSION $ENV{CM_BUILD_VER})
set(CPACK_PACKAGE_ARCHITECTURE ${arch})
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Installer)
set(CPACK_PACKAGE_LICENSE "Proprietary")
set(CPACK_PACKAGE_GROUP "Applications/Security")

# Given 0 CPack will try to use all available CPU cores.
set(CPACK_THREADS 0)
# Ensures that contents written to the CPack configuration files is escaped properly.
set(CPACK_VERBATIM_VARIABLES YES)

set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/cisco/secureclient/cloudmanagement")
set(CPACK_COMPONENTS_ALL client)

# Will NOT override umask settings.
set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
)

# HAVE_RPM
find_program(have_rpm rpm)
if(have_rpm)
    set(RPM_BUILD true)
    set(CPACK_GENERATOR "RPM")
endif()

# HAVE_DPKG
find_program(have_dpkg dpkg)
if(have_dpkg)
    set(DEB_BUILD true)
    set(CPACK_GENERATOR "DEB")
endif()

include(helper/determine_architecture)
include(helper/determine_platform)

set(CPACK_DEFAULT_DIR_PERMISSIONS ${CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS})
set(CPACK_COMPONENT_INSTALL ON)
set(CPACK_PACKAGE_AUTOREQPROV NO)

# CPack RPM specific variables
set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CPACK_PACKAGE_ARCHITECTURE}.rpm")

# Will override umask settings for RPM packages.
set(CPACK_RPM_PACKAGE_REQUIRES "systemd")
if(is_suse_based)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, cron")
else()
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, crontabs")
endif()

# CPack DEB specific variables
set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CPACK_PACKAGE_ARCHITECTURE}.deb")

# Will override umask settings for DEB packages.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, cron")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "maintainer@cisco.com")

include(cpack/install_generic_files)

set(INSTALL_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/linux/installer)
set(CPACK_POST_INSTALL_SCRIPT_FILE ${INSTALL_SCRIPT_DIR}/postinstall)
set(CPACK_PRE_UNINSTALL_SCRIPT_FILE ${INSTALL_SCRIPT_DIR}/preuninstall)
set(CPACK_USER_FILELIST
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cms.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/cmpackagemanager.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cmid.exe.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cmid_control_plugin.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csccloudmanagement_cmidapi.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/cmpackagemanager_cmidapi.log"
)

include(CPack)