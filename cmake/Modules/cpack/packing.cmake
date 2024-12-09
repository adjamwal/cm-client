# Defines CPack variables and logic flow for generating a package.
# Copyright Cisco Systems, Inc. 2024

include(helper/determine_platform_derivatives)

# Set CPack metadata variables.
set(CPACK_PACKAGE_NAME "cisco-secure-client-cloudmanagement")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cisco Cloud Management for Linux")
set(CPACK_PACKAGE_VENDOR "Cisco")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.cisco.com/site/us/en/products/security/secure-client/index.html")
set(CPACK_PACKAGE_VERSION $ENV{CM_BUILD_VER})
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Installer)

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

# CPack RPM specific variables
include(helper/determine_architecture)
include(helper/determine_platform)
set(CPACK_RPM_PACKAGE_ARCHITECTURE "${arch}")
set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")
set(CPACK_RPM_PACKAGE_LICENSE "Proprietary")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Security")


# Will override umask settings for RPM packages.
set(CPACK_RPM_DEFAULT_DIR_PERMISSIONS ${CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS})
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_RPM_PACKAGE_AUTOREQPROV NO)

set(CPACK_RPM_PACKAGE_REQUIRES "systemd")
if(is_suse_based)
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, cron")
else()
    set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, crontabs")
endif()

include(cpack/install_generic_files)

set(CPACK_RPM_USER_FILELIST
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cms.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/cmpackagemanager.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cmid.exe.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cmid_control_plugin.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csccloudmanagement_cmidapi.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/cmpackagemanager_cmidapi.log"
)

include(CPack)