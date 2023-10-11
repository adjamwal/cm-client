/bin/bash -e
#
# Usage: artifactory_upload.sh cisco-secure-client-macos-cloudmanagement-1.2.3.4.pkg
# Usage: artifactory_upload.sh cisco-secure-client-macos-cloudmanagement-1.2.3.4-symbols.tgz

ARTIFACTORY_UPLOAD_URL=https://engci-maven.cisco.com/artifactory

REL_FILENAME=$1
# SEARCH_KEY will get everything after last dot appears
SEARCH_KEY=${REL_FILENAME%.*}

if [ "${ARTIFACTORY_UPLOAD}" != "YES" ]; then
  echo "NOTE: Skipping Artifactory upload..."
  exit 0
fi

if [ ! -f "${REL_FILENAME}" ]; then
  echo "ERROR: ${REL_FILENAME} file not found"
  exit 1
else
  echo "Uploading ${REL_FILENAME} to Artifactory"
  ls -l ${REL_FILENAME}
fi

curl_result=0
curl --fail -H "X-JFrog-Art-Api: ${ARTIFACTORY_TOKEN}" -X PUT "${ARTIFACTORY_UPLOAD_URL}/ampcx-generic-thirdparty/sccm-cloudmanagement/${REL_FILENAME};key=${SEARCH_KEY}" -T "${REL_FILENAME}" || curl_result=$?
if [ $curl_result -ne 0 ]; then
  echo "ERROR: Failed to upload ${prereq}"
  exit 1
fi
exit 0