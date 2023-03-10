#!/bin/bash
#
# Script to create an AMP Apple Disk Image (DMG) with an HFS+ partition 
# containing the files and directory structure as specified in the input 
# directory. Runs on Mac using "hdiutil" and on Linux using "libdmg-hfsplus".
# Volume name is fixed to "ampmac_connector"; this is a limitation with 
# the Linux/libdmg implementation which requires a pre-built HFS+ base image
# (which has a hard-coded volume name).

function cmd_available() {
    command -v "$1" &>/dev/null
}

function print_usage_and_die() {
    local MSG="$*"
    echo -e "${MSG}\nUsage: mkdmgfd.sh [output DMG file name] [source path]"
    exit 1
}

function print_error_and_die() {
    echo "Error: $*"
    exit 1
}

function check_success() {
    if [ $? -ne 0 ]; then
        print_error_and_die "$*"
    fi
}

function cleanup() {
    local err=$?

    if [ -n "$TMPDIR" ] && [ -d "$TMPDIR" ]; then
        rm -rf "$TMPDIR"
    fi

    trap '' EXIT
    exit $err
}

# Input parameters and sanity checks
[ -z "$1" ]                 && print_usage_and_die "Missing output DMG file name"
[ -z "$2" ]                 && print_usage_and_die "Missing source path"
[ ! -d "$2" ]               && print_usage_and_die "Source path must be a directory"
DMG_NAME="${1}"
SOURCE_PATH="${2}"

# Output DMG file properties
VOLUME_NAME="cisco_sccm"

# Path to script directory
SCRIPT_NAME=$(basename "$0")
SCRIPT_DIR=$(dirname "$0")

# Store directories and files to process in arrays
# Honor spaces file names and ignore root directory
DIRS=()
while IFS=  read -r -d $'\0'; do
    DIRS+=("$REPLY")
done < <(find "${SOURCE_PATH}" ! -path "${SOURCE_PATH}" -type d -print0)
echo "Found source directories: ${DIRS[*]}"

FILES=()
while IFS=  read -r -d $'\0'; do
    FILES+=("$REPLY")
done < <(find "${SOURCE_PATH}" -type f -print0)
echo "Found source files: ${FILES[*]}"

# macOS has native support for HFS+ while some Linux systems has an optional
# "hfsplus-tools" package for creating HFS+ images. In cases where the system
# does not have a tool to create a base HFS+ image, use a "stock" HFS+ image
# file that was created elsewhere ahead of time. A 64 MiB image is used but
# capability limit is set to 63 MiB to provide margin. The file is stored as a
# compressed tarball to save storage.
#
# A new stock image can be built on a Mac using the command:
# hdiutil create -type UDIF -layout NONE -size <size> -fs HFS+ -volname "ampmac_connector" <output file>
HFSPLUS_BASE_IMAGE="hfsplus_base_image.dmg"
HFSPLUS_STOCK_IMAGE="64mb_hfsplus_base_image.dmg"
HFSPLUS_STOCK_IMAGE_TGZ="${HFSPLUS_STOCK_IMAGE}.tgz"
HFSPLUS_STOCK_IMAGE_TGZ_PATH="${SCRIPT_DIR}/${HFSPLUS_STOCK_IMAGE_TGZ}"
HFSPLUS_STOCK_IMAGE_CAPACITY=66060288

# Make temporary working directory. Clean up on exit.
TMPDIR=$(mktemp -d -t "${SCRIPT_NAME}"_XXXXX)
trap cleanup EXIT

# Create dmg file using one of two methods, in order of preference:
# 1. The macOS "hdiutil" tool
# 2. The cross-platform open source "libdmg-hfsplus" library.
#
# The target is a UDIF read-only compressed "Apple Disk Image" with a single
# HFS partition.
if cmd_available "hdiutil"; then
    echo "Using hdiutil"
    echo "Creating compressed DMG file"
    hdiutil create -fs HFS+ -layout NONE -srcfolder "${SOURCE_PATH}" -volname "${VOLUME_NAME}" -ov "${DMG_NAME}" &>/dev/null
elif cmd_available "hfsplus" && cmd_available "dmg"; then
    echo "Using libdmg-hfsplus"

    # Calculate grand total of source file sizes in bytes, confirm non-zero.
    SRC_TOTAL_SIZE=0
    for e in "${FILES[@]}"; do
        ((SRC_TOTAL_SIZE+=$(stat "$e" --printf="%s")))
        check_success "Failed to calculate file size for $e"
    done

    if [[ ! $SRC_TOTAL_SIZE -gt 0 ]]; then
        print_error_and_die "Source files have zero total size"
    fi
    echo "Total source files size: $SRC_TOTAL_SIZE bytes"

    if cmd_available mkfs.hfsplus; then
        # Calculate target image size, over-size by 1-2 MB
        HFSPLUS_TARGET_IMAGE_SIZE_MB=$(((SRC_TOTAL_SIZE + 1048576)/1048576 + 1))

        echo "Using mkfs to create an HFS+ base image with $HFSPLUS_TARGET_IMAGE_SIZE_MB MB capacity"

        dd if=/dev/zero of="${TMPDIR}/${HFSPLUS_BASE_IMAGE}" bs=1M count="$HFSPLUS_TARGET_IMAGE_SIZE_MB"
        check_success "Failed to create HFS+ base image; dd failed"

        mkfs -t hfsplus -v "${VOLUME_NAME}" "${TMPDIR}/${HFSPLUS_BASE_IMAGE}"
        check_success "Failed to create HFS+ base image; mkfs failed"
    else
        echo "Using stock HFS+ image"
        if [[ $SRC_TOTAL_SIZE -ge $HFSPLUS_STOCK_IMAGE_CAPACITY ]]; then
            print_error_and_die "HFS+ stock image is too small. Capacity is ${HFSPLUS_STOCK_IMAGE_CAPACITY} bytes but ${SRC_TOTAL_SIZE} bytes are required."
        fi

        if [ ! -f "${HFSPLUS_STOCK_IMAGE_TGZ_PATH}" ]; then
            print_error_and_die "HFS+ base image tarball file not found"
        fi

        tar -zxf "${HFSPLUS_STOCK_IMAGE_TGZ_PATH}" -C "${TMPDIR}"
        check_success "Failed to extract HFS+ base image"

        mv "${TMPDIR}/${HFSPLUS_STOCK_IMAGE}" "${TMPDIR}/${HFSPLUS_BASE_IMAGE}"
    fi

    for e in "${DIRS[@]}"; do
        echo "HFS+ image: Add directory '${e#"$SOURCE_PATH"}'"
        hfsplus "${TMPDIR}/${HFSPLUS_BASE_IMAGE}" mkdir "${e#"$SOURCE_PATH"}"
        check_success "HFS+ image: Failed to add directory ${e#"$SOURCE_PATH"}"
    done

    for e in "${FILES[@]}"; do
        echo "HFS+ image: Add file '${e}' as '${e#"$SOURCE_PATH"}'"
        hfsplus "${TMPDIR}/${HFSPLUS_BASE_IMAGE}" add "${e}" "${e#"$SOURCE_PATH"}"
        check_success "HFS+ image: Failed to add ${e#"$SOURCE_PATH"}"
    done

    echo "Creating compressed DMG file"
    # The "dmg" command sometime exits with success even though it did not
    # create an output file. Add secondary check to ensure the DMG file is
    # created.
    rm -f "${DMG_NAME}"
    dmg dmg "${TMPDIR}/${HFSPLUS_BASE_IMAGE}" "${DMG_NAME}" &>/dev/null
    check_success "DMG command failed for ${TMPDIR}/${HFSPLUS_BASE_IMAGE}"
    [ -f "${DMG_NAME}" ]
else
    print_error_and_die "No tool found to build \"${DMG_NAME}\""
fi

check_success "Failed to create DMG file from ${TMPDIR}/${HFSPLUS_BASE_IMAGE}"
