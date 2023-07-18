#!/bin/bash
# shellcheck source=/dev/null

# If we're building on Jenkins then WORKSPACE is defined, used that to derive the path to WORKSPACE_ROOT
if [ ! -z "${WORKSPACE}" ]; then
    WORKSPACE_ROOT=${WORKSPACE}/cm-client
fi

if [ -z "${WORKSPACE_ROOT}" ]; then
    SCRIPT_DIR=$(dirname "$(readlink -f "${0}")")
    WORKSPACE_ROOT="$(realpath ${SCRIPT_DIR}/../..)"
fi

if [ ! -d "${WORKSPACE_ROOT}/third-party" ]; then

    echo "ERROR: Could not find third-party directory, please verify \
          ${WORKSPACE_ROOT} in your environment"
    exit 1
fi

if [ "$2" != "" ]; then
    CM_THIRDPARTY_EXPORT=${2}
    CRASHPAD_CFLAGS="-I${CM_THIRDPARTY_EXPORT}/include -Os -Wno-deprecated"
    CRASHPAD_CXXFLAGS="-I${CM_THIRDPARTY_EXPORT}/include -Os -Wno-deprecated" CRASHPAD_LDFLAGS="-L${CM_THIRDPARTY_EXPORT}/lib -Wl,-dead_strip,-no_dead_strip_inits_and_terms -framework Foundation -framework SystemConfiguration"
fi

CXX="clang++ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -stdlib=libc++ -std=c++17"

CRASHPAD_DEP_DIR="${WORKSPACE_ROOT}/third-party/crashpad"
CRASHPAD_DIR="${CRASHPAD_DEP_DIR}/crashpad"
CRASHPAD_BUILD="${CRASHPAD_DEP_DIR}/build.sh"

CRASHPAD_BUILD_DIR="${CRASHPAD_DIR}/out/Debug"

DEPOT_TOOLS_URL=https://chromium.googlesource.com/chromium/tools/depot_tools.git
DEPOT_TOOLS_DIR="${CRASHPAD_DEP_DIR}/depot_tools"

cmd_available() {
    command -v "$1" &>/dev/null
}

use_custom_clang() {
    local minimum_major_version=6
    local major_version
    major_version=$(clang -dumpversion | cut -d. -f1)

    [ "${major_version}" -lt "${minimum_major_version}" ]
}

init() {
    if [ ! -d "${DEPOT_TOOLS_DIR}" ]; then
        echo "Fetching depot_tools..."
        if ! git clone "${DEPOT_TOOLS_URL}" "${DEPOT_TOOLS_DIR}"; then
            echo "ERROR: Failed to fetch depot_tools"
            exit 1
        fi
    fi
}

sync() {
    echo '================================================================================'
    echo 'Syncing crashpad dependencies'
    echo '================================================================================'

    pushd "${CRASHPAD_DEP_DIR}" || exit 1

    if ! PATH=$PATH:$DEPOT_TOOLS_DIR gclient sync; then
        echo "ERROR: Failed to download Crashpad dependencies"
        exit 1
    fi

    popd || exit 1
}

build() {
    echo '================================================================================'
    echo 'Building crashpad'
    echo '================================================================================'

    pushd "${CRASHPAD_DIR}" || exit 1

    # Reset version file
    git checkout package.h

    local CRASHPAD_HASH
    local CRASHPAD_VERSION
    local GN_SCRIPT_EXECUTABLE
    CRASHPAD_HASH="$(git rev-parse HEAD | tr -d '\n')"
    CRASHPAD_VERSION="0.8.0"
    sed -i -e 's/PACKAGE_VERSION ".*"/PACKAGE_VERSION "'$CRASHPAD_VERSION'"/' package.h

    # Python 2.7 was removed in macOS Monterey 12.3
    # https://developer.apple.com/documentation/macos-release-notes/macos-12_3-release-notes#Python
    if cmd_available "python3"; then
        GN_SCRIPT_EXECUTABLE="python3"
    elif cmd_available "python"; then
        GN_SCRIPT_EXECUTABLE="python"
    else
        echo "ERROR: python required to build crashpad"
        exit 1
    fi

    if [[ "${OSTYPE}" == "darwin"* ]]; then
        XCODE_MAJOR_VERSION=$(xcodebuild -version | head -1 | cut -f 2 -d ' ' | cut -f 1 -d '.')
        XCODE_MINOR_VERSION=$(xcodebuild -version | head -1 | cut -f 2 -d ' ' | cut -f 2 -d '.')
        if [ "${XCODE_MAJOR_VERSION}" -gt 12 ] || [ "${XCODE_MAJOR_VERSION}" -eq 12 -a "${XCODE_MINOR_VERSION}" -ge 2 ]; then
            TARGET_CPU=mac_universal
            GN_ARGS+=" target_cpu = \"${TARGET_CPU}\""
        fi
    fi

    if [[ "${OSTYPE}" == "darwin"* ]]; then
        # Updating to 11.0 results in some new warnings treated as errors
        # TODO: FIX
        # JIRA: https://jira-eng-rtp3.cisco.com/jira/browse/AMPCX-4638
        #GN_ARGS+=" mac_sdk_min = \"11.0\" mac_deployment_target = \"11.0\""
        GN_ARGS+=" mac_sdk_min = \"10.15\" mac_deployment_target = \"10.15\""
    fi

    if use_custom_clang; then
        GN_ARGS+=" clang_path = \"${LLVM_EXPORT_DIR}\""
    fi

    CRASHPAD_BUILD_FLAGS="extra_cflags_cc = \"${CRASHPAD_CFLAGS}\" extra_ldflags = \"${CRASHPAD_LDFLAGS}\" extra_cflags_objcc = \"${CRASHPAD_CFLAGS}\""

    echo "Using CRASHPAD_BUILD_FLAGS: ${CRASHPAD_BUILD_FLAGS}"

    if [ "$PLATFORM" != "rhel7" ]; then
        export PATH=$PATH:$DEPOT_TOOLS_DIR 
    fi
    gn gen "${CRASHPAD_BUILD_DIR}" \
        --args="crashpad_use_curl_openssl_for_http = true ${CRASHPAD_BUILD_FLAGS} ${GN_ARGS}" \
        --script-executable="${GN_SCRIPT_EXECUTABLE}"

    if [ $? -ne 0 ]; then
        echo "ERROR: gn gen failed to create export directory"
        exit 1
    fi

    if [ "$PLATFORM" = "rhel7" ]; then
        export PATH=$PATH:$DEPOT_TOOLS_DIR 
    fi
    if ! ninja -C "${CRASHPAD_BUILD_DIR}"; then
        echo "ERROR: Failed to build Crashpad"
        exit 1
    fi

    # Create missing symlinks
    pushd "${CRASHPAD_BUILD_DIR}" || exit 1
        [ ! -e libcrashpad_common.a ] && cp -r ./obj/client/libcommon.a libcrashpad_common.a
        [ ! -e libbase.a ] && ln -s ./obj/third_party/mini_chromium/mini_chromium/base/libbase.a libbase.a
        [ ! -e libcrashpad_client.a ] && ln -s ./obj/client/libclient.a libcrashpad_client.a
        [ ! -e libcrashpad_util.a ] && ln -s ./obj/util/libutil.a libcrashpad_util.a
        [ ! -e libcrashpad_net.a ] && ln -s ./obj/util/libnet.a libcrashpad_net.a
        [ ! -e libcrashpad_mig_output.a ] && ln -s ./obj/util/libmig_output.a libcrashpad_mig_output.a
        [ ! -e libcrashpad_minidump.a ] && ln -s ./obj/minidump/libminidump.a libcrashpad_minidump.a
        [ ! -e libcrashpad_handler_lib.a ] && ln -s ./obj/handler/libhandler.a libcrashpad_handler_lib.a
        [ ! -e libcrashpad_context.a ] && ln -s ./obj/snapshot/libcontext.a libcrashpad_context.a
        [ ! -e libcrashpad_snapshot.a ] && ln -s ./obj/snapshot/libsnapshot.a libcrashpad_snapshot.a
    popd || exit 1

    # Reset version file
    git checkout package.h

    # Create DWARF debug symbol file for crashpad_handler
    if [[ "${OSTYPE}" == "darwin"* ]]; then
        if ! dsymutil "${CRASHPAD_BUILD_DIR}/crashpad_handler" -o "${CRASHPAD_BUILD_DIR}/crashpad_handler.dSYM"; then
            echo "ERROR: Failed to create DWARF debug symbol file for crashpad_handler"
            exit 1
        fi
    fi
    popd || exit 1
}

function test() {
    echo '================================================================================'
    echo 'Testing crashpad'
    echo '================================================================================'

    pushd "${CRASHPAD_DIR}" || exit 1

    if ! python3 build/run_tests.py "${CRASHPAD_BUILD_DIR}"; then
        echo "ERROR: Crashpad tests failed"
        exit 1
    fi

    popd || exit 1
}

create_artifactory_archive() {
    echo '================================================================================'
    echo 'Creating crashpad Artifactory archive'
    echo '================================================================================'

    pushd "${CRASHPAD_DIR}" || exit 1

    local master_commit_longhash
    local master_commit_hash
    local master_commit_date
    local num_commits_since_branch
    local head_commit_longhash
    local head_commit_hash
    master_commit_longhash=$(git merge-base $(git rev-parse HEAD master))
    master_commit_hash=$(git log -1 --pretty=%h ${master_commit_longhash})
    master_commit_date=$(git log -1 --pretty=%cI "${master_commit_longhash}" | cut -c 1-4,6-7,9-10)
    num_commits_since_branch=$(git log --oneline "${master_commit_longhash}"..HEAD | wc -l)
    # remove leading whitespace characters
    num_commits_since_branch="${num_commits_since_branch#"${num_commits_since_branch%%[![:space:]]*}"}"
    head_commit_longhash=$(git log -1 --pretty=%H)
    head_commit_hash=$(git log -1 --pretty=%h)

    echo "Filename format is <upstream release hash date>_<upstream release hash>_<num commits since release>_<head commit hash>_<platform>.tar.gz"
    local archive_name="../${master_commit_date}_${master_commit_hash}_${num_commits_since_branch}_${head_commit_hash}_mac.tar.gz"

    echo "Creating archive $(pwd)/${archive_name}"
    tar -zcvf "${archive_name}" \
        --exclude=Release --exclude=obj --exclude=apple_cctools \
        --exclude=apple_cf --exclude=getopt --exclude=gtest \
        --exclude=gyp --exclude=llvm --exclude zlib --exclude .git \
        out/ third_party/
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to create Crashpad Artifactory archive"
        exit 1
    fi

    echo "Created archive $(pwd)/${archive_name}"
    echo "  namespace=ampcx"
    echo "  component=crashpad"
    echo "  platform=mac"
    echo "  tag=${master_commit_date}_${num_commits_since_branch}"
    echo "  key=ampcx-crashpad-mac-${head_commit_longhash}"

    popd || exit 1
}

init

case $1 in
    sync )
        sync
        ;;

    build )
        build
        ;;

    test )
        test
        ;;

    archive )
        create_artifactory_archive
        ;;

    * )
        sync
        build
        test
        ;;
esac
