# Copyright Cisco Systems, Inc. 2025

find_file(GPGME_LIB gpgme.h HINTS /usr/include)
add_subdirectory("src")
