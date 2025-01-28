# Determine the operating system derivative.
# Copyright Cisco Systems, Inc. 2024

set(RHEL_VARIANTS
    "centos"
    "redhat"
    "rocky"
    "almalinux"
    "oracle"
    "rhel")

cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)

if(${DISTRO_ID} IN_LIST RHEL_VARIANTS)
    set(is_rhel_based TRUE)
elseif(${DISTRO_ID} MATCHES "suse")
    set(is_suse_based TRUE)
elseif(${DISTRO_ID} MATCHES "sles")
    set(is_suse_based TRUE)
elseif(${DISTRO_ID} MATCHES "ubuntu")
    set(is_debian_based TRUE)
elseif(${DISTRO_ID} MATCHES "amzn")
    set(is_amazon_linux TRUE)
endif()
