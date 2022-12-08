# Attempts to download the component from Artifactory
# Copyright Cisco Systems, Inc. 2022

function(upload_component component_name output)
    if(DEFINED ENV{ARTIFACTORY_TOKEN})
        ExternalProject_Add_Step(
            third-party-${component_name}
            artifactory_upload
            COMMAND ARTIFACTORY_TOKEN=$ENV{ARTIFACTORY_TOKEN} ${CM_SCRIPTS_DIR}/upload_artifact_to_artifactory.sh ${component_name}
            COMMAND ${COMMAND} cp -aR "${component_install_prefix}/." "${CM_THIRDPARTY_EXPORT}"
            DEPENDEES install
        )
        set(${output} artifactory_upload PARENT_SCOPE)
    else()
        ExternalProject_Add_Step(
            third-party-${component_name}
            copy_exports
            COMMENT "-- Artifactory upload skipped for ${component_name}, no ARTIFACTORY_TOKEN available"
            COMMAND ${COMMAND} cp -aR "${component_install_prefix}/." "${CM_THIRDPARTY_EXPORT}"
            DEPENDEES install
        )
        set(${output} copy_exports PARENT_SCOPE)
    endif()
endfunction()
