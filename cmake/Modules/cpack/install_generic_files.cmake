# Handles the installation of generic files.
# Copyright Cisco Systems, Inc. 2024

# Set rpm-specific variables.
if(RPM_BUILD)
    set(systemd_install_path "/usr/lib/systemd/system")
endif()

install(
    PROGRAMS 
        ${CMAKE_CURRENT_BINARY_DIR}/export/bin/csccloudmanagement
        ${CMAKE_CURRENT_BINARY_DIR}/export/bin/cmpackagemanager
        ${CMAKE_CURRENT_BINARY_DIR}/export/bin/csc_cmid
    DESTINATION ${CPACK_PACKAGING_INSTALL_PREFIX}/bin
    COMPONENT linuxinstall
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/export/lib/libcmidapi.so
    DESTINATION ${CPACK_PACKAGING_INSTALL_PREFIX}/lib
    COMPONENT linuxinstall
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

FILE(MAKE_DIRECTORY ${CPACK_PACKAGING_INSTALL_PREFIX}/etc)

install(
    PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/linux/systemd/csccmservice
    TYPE BIN
    COMPONENT linuxinstall
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/linux/systemd/csccloudmanagement.service
    DESTINATION ${systemd_install_path}
    COMPONENT linuxinstall
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)