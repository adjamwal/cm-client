#!/bin/bash
#
# A collection of bash script functions for uploading and downloading csccm
# pre-built binaries from Artifactory, typically used for build artifacts
# produced by csccm third-party or submodule repositories.
#

if [ -z "${WORKSPACE_ROOT}" ]; then
    #WORKSPACE_ROOT="$(pwd)"
    # readlink -f not always available and symlink is in same
    # directory as script, so we can get by without it.
    #WORKSPACE_ROOT=$(dirname "$(readlink -f "${0}")")
    WORKSPACE_ROOT=$(dirname "${0}")
    WORKSPACE_ROOT=${WORKSPACE_ROOT}/..
    # WORKSPACE_ROOT should now contain the full directory path
    #echo "Using WORKSPACE_ROOT=${WORKSPACE_ROOT}"
fi

is_mac_os() {
    [[ "${OSTYPE}" == "darwin"* ]]
}

is_linux() {
    [[ "${OSTYPE}" == "linux"* ]]
}

if is_mac_os; then
    PLATFORM=macOS
elif is_linux; then
    PLATFORM=linux
else
    echo "Unsupported platform"
    exit 1
fi

# Note that Artifactory web pages indicate that engci-maven.cisco.com be used
# when running searches or downloads from Artifactory but that server doesn't
# reliably return the correct results or downloads. We were directed to instead
# use engci-maven-master.cisco.com in place of engci-maven.cisco.com which has
# proven to work reliably even though we are still unsure why this is necessary.
ARTIFACTORY_DOWNLOAD_URL=https://engci-maven-master.cisco.com/artifactory
ARTIFACTORY_UPLOAD_URL=https://engci-maven.cisco.com/artifactory
ARTIFACTORY_PROPERTY_SEARCH_URL="${ARTIFACTORY_DOWNLOAD_URL}/api/search/prop"
ARTIFACTORY_DIRNAME=".artifactory"
ARTIFACTORY_LOCAL_DIR="${WORKSPACE_ROOT}/${ARTIFACTORY_DIRNAME}"

echoerr() {
  (>&2 echo "$@")
}

get_prereq_dir(){
  local prereq="${1}"
  local prereq_dirs=(
    "EndpointIdentity:EndpointIdentity"
    "PackageManager:PackageManager"
    "ciscossl:third-party/ciscossl"
    "curl:third-party/curl"
    "gtest:third-party/gtest"
    "jsoncpp:third-party/jsoncpp"
    "spdlog:third-party/spdlog"
    "crashpad:third-party/crashpad/crashpad"
  )
  for prereq_dir in "${prereq_dirs[@]}" ; do
    local component="${prereq_dir%%:*}"
    local local_dir="${prereq_dir##*:}"
    if [ "${component}" == "${prereq}" ]; then
      echo "${WORKSPACE_ROOT}/${local_dir}"
    fi
  done
}

get_prereq_cmake_file(){
  local prereq="${1}"
  echo "${WORKSPACE_ROOT}/cmake/Modules/ExternalProject_${prereq}.cmake"
}

get_prereq_cmake_file_last_commit_hash(){
  local prepeq="${1}"
  local prepeq_cmake
  local interested_modules=(
    "crashpad"
  )
  for mdl in "${interested_modules[@]}" ; do
    if [ "${mdl}" == "${prepeq}" ]; then
      prepeq_cmake=$(get_prereq_cmake_file "${prepeq}")
      echo "$(git log -n 1 --pretty=format:%h -- "${prepeq_cmake}")_"
      return
    fi
  done
  echo ""
}

upload_prereq_to_artifactory(){
  local prereq="${1}"
  local prereq_dir
  local head_commit_fullhash

  prereq_dir=$(get_prereq_dir "${prereq}")
  pushd "${prereq_dir}" || exit 1
    head_commit_fullhash=$(git rev-parse HEAD | tr -d '\n')
  popd || exit 1

  artifactory_upload "${prereq}" "${head_commit_fullhash}" "${prereq_dir}"
}

get_prereq_download_url(){
  local prereq="${1}"
  local download_url=""
  local prereq_dir
  local url_result=0
  local selection=""
  local head_commit_fullhash
  local search_url=""
  local cmake_file_commit_hash

  cmake_file_commit_hash=$(get_prereq_cmake_file_last_commit_hash "${prereq}")
  prereq_dir=$(get_prereq_dir "${prereq}")
  if [ -z "${prereq_dir}" ]; then
    echoerr "ERROR: Pre-req ${prereq} not found"
    exit 1
  fi
  pushd "${prereq_dir}" || exit 1

  head_commit_fullhash=$(git rev-parse HEAD | tr -d '\n')

  search_url=$(build_artifactory_search_url "${prereq}" "${head_commit_fullhash}" "${cmake_file_commit_hash}") || url_result=1
  if [[ ${url_result} -ne 0 ]]; then
    # Should never happen
    echoerr "Failed to build search URL for ${prereq} ${head_commit_fullhash}, aborting."
    exit 1
  fi
  download_url=$(get_artifactory_download_url "${search_url}") || url_result=$?
  if [[ ${url_result} -eq 2 ]]; then
    # The download_url contains a failure message when the result is non-zero
    echo $download_url
    exit 1
  elif [[ ${url_result} -ne 0 ]]; then
    # The download_url contains a failure message when the result is non-zero
    printf "\n%s\n" "${download_url}"
    download_url=""
  fi

  if [[ -n "${download_url}" ]]; then
    echo "${download_url}"
  fi

  popd || exit 1
}

get_dependency_string(){
  local prereq="${1}"
  local dependency_string
  local dependency_relations
  local build_script_path
  local build_script_hash

  # NOTE We can use $(uname) to specify different
  #      dependencies for different OS'.  Right now,
  #      we're only concernd about Mac
  dependency_relations=(
    "curl:ciscossl"
    "PackageManager:ciscossl curl jsoncpp gtest"
    "crashpad:ciscossl curl"
  )

  dependency_string=""

  # Determine additional dependencies using dependency relations table
  for dependency_relation in "${dependency_relations[@]}" ; do
    local component="${dependency_relation%%:*}"
    local dependencies=(${dependency_relation##*:})
    if [ "${component}" == "${prereq}" ]; then
      for dependency in "${dependencies[@]}" ; do
        local prereq_dir
        prereq_dir=$(get_prereq_dir "${dependency}")
        pushd "${prereq_dir}" > /dev/null || exit 1
        local hash
        hash=$(git rev-parse HEAD | tr -d '\n')
        if [ "${#hash}" -ne 40 ]; then
          echoerr "ERROR: Expected ${prereq} SHA-1 hash which is 40 characters, got ${hash} which is ${#hash} characters"
          exit 1
        fi
        popd > /dev/null || exit 1
        dependency_string="${dependency_string}.${dependency}-${hash:0:10}"
      done
    fi
  done

  echo "${dependency_string}"
}

get_exclude_patterns(){
  local prereq="${1}"
  local full_exclude_pattern=""
  local exclude_pattern_relations=(
    "ciscossl:*.la"
    "curl:*.la"
    "gtest:*.la"
    "jsoncpp:*.la"
    "spdlog:*.la"
    "PackageManager:*.la"
    "EndpointIdentity:*.la"
    "crashpad:"
  )
  for exclude_pattern_relation in "${exclude_pattern_relations[@]}" ; do
    local component="${exclude_pattern_relation%%:*}"
    local exclude_patterns=(${exclude_pattern_relation##*:})
    if [ "${component}" == "${prereq}" ]; then
      for exclude_pattern in "${exclude_patterns[@]}" ; do
        full_exclude_pattern="${full_exclude_pattern} --exclude=${exclude_pattern} --exclude=./export/src --exclude=./export/tmp"
      done
      echo "${full_exclude_pattern}"
    fi
  done
}

get_prereq_version() {
  local prereq="${1}"
  local head_commit_fullhash="${2}"
  local description
  description="$(get_prereq_description "${prereq}" "${head_commit_fullhash}")"
  # Everything after the first dash in the description is the version.
  # Obtaining the version is done this way because some third party modules
  # include their project name as part of the Git tag read by `git describe`.
  echo "${description#*-}"
}

num_commit_on_base_branch() {
  local num_commits
  local base_branches="main master"
  for branch in ${base_branches}; do
    num_commits=$(git rev-list "${branch}".. --count 2>/dev/null | tr -d '\n')
    if [[ -n $num_commits ]]; then
      echo "${num_commits}"
      break
    fi
  done
}

get_prereq_description(){
  local prereq="${1}"
  local head_commit_fullhash="${2}"
  local prereq_description
  prereq_description=''
  if [ -f "version.txt" ]; then
    local local_version
    local num_commits
    local_version=$(<version.txt)
    num_commits=$(num_commit_on_base_branch)
    #Git describe adds a "g" to the start of the hash, adding it here for consistency
    prereq_description="${prereq}-${local_version}-${num_commits}-g${head_commit_fullhash:0:10}"
  #else
    # Describe gives a human readable name based on git tags
    # The submodule repositories are currently tagged for use with describe
    # Can uncomment if this changes
    #prereq_description=$(git describe --long --abbrev=10 | tr -d '\n')
  fi

  if [ -z "${prereq_description}" ]; then
    # No version.txt and no "git describe" tag, default to using similar form as
    # version.txt case but use x's for unknown version info.
    local num_commits
    num_commits=$(git rev-list --all --no-merges --count | tr -d '\n')
    prereq_description="${prereq}-x.x.x-${num_commits}-g${head_commit_fullhash:0:10}"
  fi
  echo "${prereq_description}"
}

# Define as a local function to avoid script dependencies for build nodes.
is_platform_arm() {
    if [ $(arch) == "aarch64" ] || [ $(arch) == "arm64" ]; then 
        true
    else
        false
    fi
}

get_artifactory_filename(){
  local prereq="${1}"
  local head_commit_fullhash="${2}"
  local prereq_description
  local dependency_string
  local file_name_local
  local cmake_file_commit_hash="${3}"
  prereq_description=$(get_prereq_description "${prereq}" "${head_commit_fullhash}")
  dependency_string=$(get_dependency_string "${prereq}")
  if [ $? -ne 0 ]; then
    echoerr "ERROR: Failed to get dependency string for ${prereq}"
    exit 1
  fi

  file_name_local="${prereq_description}_${cmake_file_commit_hash}${PLATFORM}${dependency_string}.tar.gz"

  echo "${file_name_local}"
}

# Outputs a key that can be used to search Artifactory for the corresponding artifact
# The search key has the form:
#     "sccm_<component>_<platform>_<head_commit_fullhash>[<dep_string>]"
get_artifactory_key(){
  local component="${1}"
  local head_commit_fullhash="${2}"
  local dependency_string
  local cmake_file_commit_hash="${3}"
  local search_key
  dependency_string=$(get_dependency_string "${component}")
  if [ $? -ne 0 ]; then
    echoerr "ERROR: Failed to get dependency string for ${component}"
    exit 1
  fi

  if [ -z "$cmake_file_commit_hash" ];
  then
    search_key="sccm_${component}_${PLATFORM}_${head_commit_fullhash}${dependency_string}"
  else
    search_key="sccm_${component}_${PLATFORM}_${head_commit_fullhash}_${cmake_file_commit_hash}${dependency_string}"
  fi

  echo "${search_key}"
}

artifactory_upload(){
  local prereq="${1}"
  local head_commit_fullhash="${2}"
  local prereq_dir="${3}"
  local file_name
  local search_key
  local curl_result=0
  local exclude_pattern
  local download_url=""
  local existing_filename=""
  local search_url=""
  local url_result=0
  local cmake_file_commit_hash

  cmake_file_commit_hash=$(get_prereq_cmake_file_last_commit_hash "${prereq}")
  exclude_pattern=$(get_exclude_patterns "${prereq}")
  pushd "${prereq_dir}" || exit 1
    file_name=$(get_artifactory_filename "${prereq}" "${head_commit_fullhash}" "${cmake_file_commit_hash}" "${prereq_dir}" "${prereq_dir}")
  popd || exit 1
  search_key=$(get_artifactory_key "${prereq}" "${head_commit_fullhash}" "${cmake_file_commit_hash}")

  rm -f "${file_name}"
  pushd "${prereq_dir}" || exit 1
    if [ -d "export" ]; then
      eval tar "${exclude_pattern[@]}" -czvf "${file_name}" "export*"
    elif [ -d "../export" ]; then
        # For convenience, let's move it in and then out
        mv ../export .
        eval tar "${exclude_pattern[@]}" -czvf "${file_name}" "export*"
        mv export ../
    else
      echoerr "export for ${prereq} not found at ${prereq_dir}"
      exit 1
    fi
  popd || exit 1

  # Move file back to the CWD
  mv "${prereq_dir}/${file_name}" .

  # First make a quick check that the key doesn't already exist on the server.
  search_url=$(build_artifactory_search_url_using_key "${search_key}") || url_result=1
  if [[ ${url_result} -ne 0 ]]; then
    # Should never happen
    echoerr "Failed to build search URL for key ${search_key}, aborting."
    exit 1
  fi
  echo "Checking if artifact with key ${search_key} already exists."
  echo "Using search URL ${search_url}"
  download_url=$(get_artifactory_download_url "${search_url}") || url_result=$?
  if [[ ${url_result} -eq 2 ]]; then
    # The download_url contains a failure message when the result is non-zero
    echo $download_url
    exit 1
  elif [[ ${url_result} -ne 0 ]]; then
    # The download_url contains a failure message when the result is non-zero
    printf "\n%s\n" "${download_url}"
  elif [[ "${download_url}" != "" ]]; then
    # Check if the filename is a match.
    existing_filename="${download_url##*/}"
    if [[ "${existing_filename}" == "${file_name}" ]]; then
      echo
      echo "WARNING: File ${file_name} already exists on Artifactory and will be replaced."
      echo
    else
      # For now, abort this script if it ever detects this situation so that we
      # can confirm our theory on how it is happening (multiple CI jobs triggered
      # at the same time building the same new artifact).
      echoerr "ERROR: an artifact with key ${search_key} already exists on Artifactory with a different name."
      echoerr "Received download URL: ${download_url}"
      echoerr "New upload filename: ${file_name}"
      echoerr "Aborting."
      exit 1
    fi
  fi

  #
  # Using ampcx-generic-thirdparty repository for the anonymous download permissions
  curl --fail -H "X-JFrog-Art-Api: ${ARTIFACTORY_TOKEN}" \
  -X PUT "${ARTIFACTORY_UPLOAD_URL}/ampcx-generic-thirdparty/sccm-${prereq}/${file_name};key=${search_key}"\
  -T "${file_name}" \
  || curl_result=$?
  if [ $curl_result -ne 0 ]; then
    echo "ERROR: Failed to upload ${prereq}"
    exit 1
  fi

  # With the artifact uploaded, do a check to confirm that there is still only
  # one file with the key (in case another file with the same key was uploaded
  # at the same time).
  echo ""
  echo "After upload, verify there is only one artifact with key ${search_key}."
  echo "Using search URL ${search_url}"
  download_url=$(get_artifactory_download_url "${search_url}") || url_result=$?
  if [[ ${url_result} -eq 2 ]]; then
    # The download_url contains a failure message when the result is non-zero
    echo $download_url
    # The result code of 2 indicates that multiple files with this key
    # are stored on artifactory. Automatically remove the file that was just
    # uploaded as an attempt to resolve this. If multiple jobs are running
    # concurrently and both detect multiple files exist then they could both
    # be removed leaving nothing on Artifactory for this artifact, and would
    # cause a new artifact to be built and uploaded the next time a job is run,
    # and that would resolve the missing artifact more easily than attempting
    # to futher resolve this issue here (e.g. by re-uploading again here).
    echo "Detected multiple files with search key for this artifact."
    echo "Removing ${file_name} that was just uploaded."
    curl --fail -H "X-JFrog-Art-Api: ${ARTIFACTORY_TOKEN}" \
    -X DELETE "${ARTIFACTORY_UPLOAD_URL}/ampcx-generic-thirdparty/sccm-${prereq}/${file_name}" || curl_result=$?
    if [ $curl_result -ne 0 ]; then
      echo "ERROR: Failed to remove duplicate ${prereq}, ${ARTIFACTORY_UPLOAD_URL}/ampcx-generic-thirdparty/sccm-${prereq}/${file_name}"
      exit 1
    fi
  fi
}

build_artifactory_search_url(){
  local component="${1}"
  local head_commit_fullhash="${2}"
  local cmake_file_commit_hash="${3}"
  local component_search_key=""
  local search_url=""

  component_search_key=$(get_artifactory_key "${component}" "${head_commit_fullhash}" "${cmake_file_commit_hash}")
  search_url=$(build_artifactory_search_url_using_key "${component_search_key}")

  echo "${search_url}"
  return 0
}

build_artifactory_search_url_using_key(){
  local component_search_key="${1}"
  local search_url="${ARTIFACTORY_PROPERTY_SEARCH_URL}?key=${component_search_key}"

  echo "${search_url}"
  return 0
}

get_artifactory_download_url(){
  local search_url="${1}"
  local download_url=""
  local ret=1
  local response=""

  # Use the provided search URL to see if the item exists on Artifactory.
  response=$(curl "${search_url}")

  local num
  num=$(echo "${response}" | grep -o \\\"uri\\\" | wc -l)
  if [[ "$num" -gt 1 ]]; then
    echoerr "$num URLs returned for search: '${search_url}': ${response}"
    return 2
  fi

  # Examine the response to locate the Artifactory download URL for the item
  # that matched the search key
  local regex_for_download_url="(\"uri\" : \"https://engci-maven.cisco.com/artifactory/api/storage)/(ampcx-generic-thirdparty/[^\"/]+)/([^\"]+)\""
  if [[ ${response} =~ ${regex_for_download_url} ]]; then
    local artifact_repo_dir="${BASH_REMATCH[2]}"
    local artifact_filename="${BASH_REMATCH[3]}"

    # The URL provided in the search results is a URL to retrieve additional
    # information about the file including the actual download URL. Instead
    # of performing a second query construct the download URL directly.
    download_url="${ARTIFACTORY_DOWNLOAD_URL}/${artifact_repo_dir}/${artifact_filename}"
    echo "${download_url}"
    ret=0
  else
    echoerr "No download URL for search: '${search_url}'"
    echoerr "Response:"
    echoerr "${response}"
    ret=1
  fi
  return ${ret}
}

#
# main starts here...

exec_action=$(basename $0)
if [ "${exec_action}" = "get_artifactory_url.sh" ]; then
    if [ $# -ge 1 ]; then
      get_prereq_download_url "${1}"
    else
      echo "Usage: get_artifactory_url.sh <prereq>"
      exit 1
    fi
elif [ "${exec_action}" = "upload_artifact_to_artifactory.sh" ]; then
    if [ $# -ge 1 ]; then
      upload_prereq_to_artifactory "${1}"
    else
      echo "Usage: upload_artifact_to_artifactory.sh <prereq>"
      exit 1
    fi
fi
