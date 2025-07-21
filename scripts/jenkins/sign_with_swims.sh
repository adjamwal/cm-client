#!/bin/bash -e
# 
# Expected environment variables
#
# CM_CLIENT_TAG - branch name
# VERSION_MAJOR
# VERSION_MINOR
# VERSION_PATCH
# VERSION_BUILD
#
# Usage:
# 
# ./sign_with_swims.sh <swims_signing_tools_root> <build auth token> <path_to_installer>
#
# NOTE: swims_signing_tools_root is the path to the SWIMS rpm_deb_signing repository of SWIMS tools used for RPM and Debian package signing

if [ $# -ne 3 ]; then
    echo "Usage sign_with_swims.sh <swims_signing_tools_root> <build_auth_token> <path_to_installer>"
    exit 1
fi

# Check to be sure we have GPG as it's required to do the signing
if ! command -v gpg >/dev/null 2>&1; then
    echo "gpg could not be found"
    exit 1
fi
echo "**Using GPG found in $(command -v gpg)**"

SWIMS_OPENPGP_TOOLS_PATH="${1}/Linux-64/swims-openpgp"

BRANCH_NAME=${CM_CLIENT_TAG}
if [ -z "${BRANCH_NAME}" ]; then
    BRANCH_NAME="main"
fi

PROJECT_NAME="https://code.engine.sourcefire.com/UnifiedConnector/cm-client"
ARTIFACT_NAME=SecureClientCloudManagement
ARTIFACT_VERSION="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}"

# Hardcoded to the author of this script for now
BUILD_INIT=louilee

export VAULT_ADDR=https://keeper.cisco.com
export VAULT_NAMESPACE=swims-prod/GROUP_2
export BUILD_ATTESTATION_KEY_NAME=SecureClientCloudManagement

pushd "${SWIMS_OPENPGP_TOOLS_PATH}"
    rm -f BuildAuth.tkn
    echo -n "${2}" > BuildAuth.tkn

    # Create Build Metadata
    rm -f BuildMetadataFile.json ArtifactMetadataFile.json release-session-request-payload.b64 release-build-data.sig release-build-session.tkn
    ./code_sign.8.x86_64 build-data buildMetadata encode -buildInitiator "${BUILD_INIT}" -buildTimestamp "$(date +%s)" -branchName "${BRANCH_NAME}" -projectName "${PROJECT_NAME}" -pipelineUrl "${JOB_URL}" -pipelineId "Release-CM-Client-Linux" -jobId "${BUILD_NUMBER}" -out BuildMetadataFile.json
    cat BuildMetadataFile.json 

    # Create Artifact Metadata
    ./code_sign.8.x86_64 build-data artifactMetadata encode -artifactName "${ARTIFACT_NAME}" -releaseVersion "${ARTIFACT_VERSION}" -releaseDate "$(date +%m-%d-%Y)" -out ArtifactMetadataFile.json

    # Create Signed Request Payload for use to create the Session Token
    ./code_sign.8.x86_64 swims build session encodePayload -buildAuthToken BuildAuth.tkn -buildInitiator "${BUILD_INIT}" -branchName "${BRANCH_NAME}" -projectName "${PROJECT_NAME}" -buildMetadata BuildMetadataFile.json -out release-session-request-payload.b64
    ./code_sign.8.x86_64 swims utils vault signWithAttestationKey -input release-session-request-payload.b64 -out release-build-data.sig
 
    # Creates the Session Token
    ./code_sign.8.x86_64 swims build session create -requestPayload release-session-request-payload.b64 -requestSignature release-build-data.sig -out release-build-session.tkn -logLevel DEBUG

    echo "**Session Token Created**"

    cloud_management_rpm="${3}/cisco-secure-client-cloudmanagement-${ARTIFACT_VERSION}.$(arch).rpm"

    # Source the environment values to setup the build
    source secure_client_cm_key_setup.sh

    echo "**Signing RPM ${cloud_management_rpm}, sig_type=${sig_type}**"
    ls -al ${cloud_management_rpm}

    status=0

    # Note to sign locally you would use `rpm --addsign`, in Jenkins, we use the provided Python/expect scripts
    #rpm --define '_gpg_name _' --define '__gpg_check_password_cmd /bin/true' --define '__gpg_sign_cmd %(echo $PWD)/run-extsign.exp    run-extsign %{__plaintext_filename} %{__signature_filename}' --addsign "${cloud_management_rpm}" || status=$?

    echo "${cloud_management_rpm}" > rpmlist.csv
    python3 rpm_sign_batchmode.py3 -f rpmlist.csv -r ./rpm-sign-pipeline.exp || status=$?
    rm rpmlist.csv
    if [ ${status} -ne 0 ]; then
        echo "ERROR: RPM signing failed"
        echo
        cat swims_client.log
        exit 1
    fi
popd

cp "${SWIMS_OPENPGP_TOOLS_PATH}/cloud_management_rpm_import_gpg" cm_gpg_pub.key
