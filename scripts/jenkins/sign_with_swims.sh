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
# ./sign_with_swims.sh <swims_signing_tools_root> <build auth token> <path_to_installer> <rpm|deb>
#
# NOTE: swims_signing_tools_root is the path to the SWIMS rpm_deb_signing repository of SWIMS tools used for RPM and Debian package signing

if [ $# -ne 4 ]; then
    echo "Usage sign_with_swims.sh <swims_signing_tools_root> <build_auth_token> <path_to_installer> <rpm|deb>"
    exit 1
fi

installer_pkg_type="${4}"
if [ "${installer_pkg_type}" = "rpm" ]; then
    echo "Using SWIMS to sign a RPM installer package"
elif [ "${installer_pkg_type}" = "deb" ]; then
    echo "Using SWIMS to sign a Debian installer package"
else
    echo "Unrecognized installer type ${installer_pkg_type}"
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

function sign_rpm_installer() {
    local installer_path="${1}"
    local cloud_management_rpm="${installer_path}/cisco-secure-client-cloudmanagement-${ARTIFACT_VERSION}.$(arch).rpm"
    local status=0

    echo "**Signing RPM ${cloud_management_rpm}, sig_type=${sig_type}**"
    ls -al "${cloud_management_rpm}"

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
}

#
# A Debian installer file is made up of the following files
# * debian-binary
# * control archive (control.{tar,xz,gz}
# * data archive (data.{tar,xz,gz,bz2,lzma}
#
# Our aim to produce an origin type compatible signature that can be verified using debsig-verify.
# This type of signature can be produced using debsigs but it can only produce a signature using
# local keys.  There is a patch that the SWIMS team provides to patch the debsigs tools. The
# patch is specific to two versions of the tool.  The issue I have with patching a system executable
# is that it has problems with reliability such as version incompatibilities with the patches
# and the fact that a apt upgrade of the package will wipe out the changes applied by the patch.
#
# Producing an origin type signature is quite simple though.  It's basically to include a signature
# of the combination of the Debian installer files as listed above (debian-binary, control archive
# and the data archive).  The name of this signature is `_gpgorigin`.
#
# The function below will unpack the Debian installer, use the SWIMS `run-extsign` tool to produce
# the `_gpgorigin` signature and then repackage the Debian installer to includes this new signature
# file.
#
function sign_deb_installer() {
    local installer_path="${1}"

    local pkg_arch=$(arch)
    if [ ${pkg_arch} = "x86_64" ]; then
        pkg_arch=amd64
    fi

    local installer_name="cisco-secure-client-cloudmanagement-${ARTIFACT_VERSION}.${pkg_arch}.deb"
    local cloud_management_deb="${installer_path}/${installer_name}"
    local status=0

    echo "**Signing Debian installer ${cloud_management_deb}, sig_type=${sig_type}**"

    cp "${cloud_management_deb}" .

    # Clean up in case files were left behind from a previous build
    rm -f debian-binary control.tar.* data.tar.*

    ar x "${installer_name}"

    member_control=$(find . -regex ".*/control\.tar\(\.xz\|\.gz\)?")
    member_data=$(find . -regex ".*/data\.tar\(\.gz\|\.xz\|\.bz2\|\.lzma\)?")
    cat "debian-binary" "${member_control}" "${member_data}" > ".combined"
    ./run-extsign.exp ".combined" _gpgorigin
    rm -f "${installer_name}"
    ls -al debian-binary "${member_control}" "${member_data}" _gpgorigin
    ar rc "${installer_name}" debian-binary "${member_control}" "${member_data}" _gpgorigin
    rm -f debian-binary "${member_control}" "${member_data}" _gpgorigin

    # Overwrite the non-signed installer with the signed version
    mv "${installer_name}" "${cloud_management_deb}"
}

function create_debsig_policy() {
  cat << EOF > "${1}"
<?xml version="1.0"?>
<!DOCTYPE Policy SYSTEM "https://www.debian.org/debsig/1.0/policy.dtd">
<Policy xmlns="https://www.debian.org/debsig/1.0/">

<Origin Name="Debsig" id="55CA54ACFEDC4C8F" Description="Cisco Secure Client Cloud Management"/>
  <Selection>
    <Required Type="origin" File="debsig.gpg" id="55CA54ACFEDC4C8F"/>
  </Selection>
  <Verification MinOptional="0">
    <Required Type="origin" File="debsig.gpg" id="55CA54ACFEDC4C8F"/>
  </Verification>
</Policy>
EOF
}

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

    # Source the environment values to setup the build
    source secure_client_cm_key_setup.sh

    if [ "${installer_pkg_type}" = "rpm" ]; then
        sign_rpm_installer "${3}"
    else
        sign_deb_installer "${3}"
    fi
popd

if [ "${4}" = "rpm" ]; then
  cp "${SWIMS_OPENPGP_TOOLS_PATH}/cloud_management_rpm_import_gpg" cm_gpg_pub.key
else
  cp "${SWIMS_OPENPGP_TOOLS_PATH}/rel.gpg" debsig.gpg
  create_debsig_policy "secureclientcloudmanagement.pol"
fi
