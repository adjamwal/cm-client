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
# HAVE_DPKG
find_program(have_dpkg dpkg)

if(have_rpm)
    set(RPM_BUILD true)
    set(CPACK_GENERATOR "RPM")
elseif(have_dpkg)
    set(DEB_BUILD true)
    set(CPACK_GENERATOR "DEB")
endif()

include(helper/determine_architecture)
include(helper/determine_platform)

# CPack RPM specific variables
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

# CPack DEB specific variables
# Explicit setting the architecture to amd64 when `arch` is x86_64 to avoid architecture mismatch while installation.
if( ${arch} STREQUAL "x86_64" )
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${arch}")
endif()
set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Cisco Systems, Inc.")
set(CPACK_PACKAGE_CONTACT "support@cisco.com")

# Will override umask settings for DEB packages.
set(CPACK_DEBIAN_DEFAULT_DIR_PERMISSIONS ${CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS})
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_PACKAGE_SECTION "non-free/utils")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "systemd , cron")

include(cpack/install_generic_files)

set(INSTALL_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/linux/installer)

set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${INSTALL_SCRIPT_DIR}/postinstall)
set(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE ${INSTALL_SCRIPT_DIR}/preinstall)
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE ${INSTALL_SCRIPT_DIR}/preuninstall)

set(CPACK_RPM_USER_FILELIST
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cms.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/cmpackagemanager.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cmid.exe.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csc_cmid_control_plugin.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/csccloudmanagement_cmidapi.log"
"%ghost %{_localstatedir}/log/cisco/secureclient/cloudmanagement/cmpackagemanager_cmidapi.log"
)

# Create symlinks for postinstall and preuninstall scripts
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${INSTALL_SCRIPT_DIR}/postinstall ${INSTALL_SCRIPT_DIR}/postinst)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${INSTALL_SCRIPT_DIR}/preuninstall ${INSTALL_SCRIPT_DIR}/prerm)

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${INSTALL_SCRIPT_DIR}/postinst"
    "${INSTALL_SCRIPT_DIR}/prerm"
)

include(CPack)