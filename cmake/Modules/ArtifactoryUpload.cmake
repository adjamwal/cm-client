# Attempts to download the component from Artifactory
# Copyright Cisco Systems, Inc. 2022

function(upload_component component_name output)
    if(DEFINED ENV{ARTIFACTORY_TOKEN})
        ExternalProject_Add_Step(
            third-party-${component_name}
            artifactory_upload
            COMMAND PLATFORM=$ENV{PLATFORM} ARTIFACTORY_TOKEN=$ENV{ARTIFACTORY_TOKEN} ${CM_SCRIPTS_DIR}/upload_artifact_to_artifactory.sh ${component_name}
            DEPENDEES install
        )
        set(${output} artifactory_upload PARENT_SCOPE)
    endif()

    ExternalProject_Add_Step(
        third-party-${component_name}
        copy_exports
        COMMENT "-- Copying exports for ${component_name} to common export directory"
        COMMAND ${COMMAND} cp -aR "${component_install_prefix}/." "${CM_THIRDPARTY_EXPORT}"
        DEPENDEES install
    )
    set(${output} copy_exports PARENT_SCOPE)
endfunction()
