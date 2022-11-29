# Attempts to download the component from Artifactory
# Copyright Cisco Systems, Inc. 2022

function(get_artifactory_url component output)
    execute_process(
        COMMAND ${CM_SCRIPTS_DIR}/get_artifactory_url.sh ${component} ${is_arch_arm}
        OUTPUT_VARIABLE get_artifactory_url_output
    )

    string(REGEX MATCH "https:\/\/.+\.tar\.gz" artifactory_url "${get_artifactory_url_output}")

    if (artifactory_url STREQUAL "")
        set(${output} "" PARENT_SCOPE)
        message("ERROR: download URL for ${component} is not available.")
        #
        # TODO also clean the third-party component to ensure the build
        #      is pristine prior to building and uploading to Artifactory
    else()
        set(${output} ${artifactory_url} PARENT_SCOPE)
        message("-- Have download URL for ${component}: ${artifactory_url}.")
    endif()
endfunction()

function(download_component component_name component_dst_dir)
    get_artifactory_url(${component_name} artifactory_url)
    if (NOT ${artifactory_url} STREQUAL "")

        ExternalProject_Add(
            third-party-${component_name}
            URL ${artifactory_url}
            CONFIGURE_COMMAND echo "Configuration not necessary."
            BUILD_COMMAND echo "Build not necessary."
            INSTALL_COMMAND ${COMMAND} cp -aR "${component_dst_dir}/." "${CM_THIRDPARTY_EXPORT}"
        )

    endif()
endfunction()
