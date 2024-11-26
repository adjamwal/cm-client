# Handles the installation of generic files.
# Copyright Cisco Systems, Inc. 2024

install(
    PROGRAMS 
        ${CMAKE_CURRENT_BINARY_DIR}/export/bin/csccloudmanagement
        ${CMAKE_CURRENT_BINARY_DIR}/export/bin/cmpackagemanager
        ${CMAKE_CURRENT_BINARY_DIR}/export/bin/csc_cmid
    DESTINATION ${CPACK_PACKAGING_INSTALL_PREFIX}/bin
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/export/lib/libcmidapi.so
    DESTINATION ${CPACK_PACKAGING_INSTALL_PREFIX}/lib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)

FILE(MAKE_DIRECTORY ${CPACK_PACKAGING_INSTALL_PREFIX}/etc)
