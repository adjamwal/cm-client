# DMG Resources
- `_DS_Store`: A file that stores the custom attributes of its containing folder, such as position of icons, window size, background image etc.

- `background.png`: An image to be used for the background of the created DMG. This image is referenced from the included DS_Store file.

- `mkdmg.sh`: A script that creates a policy-injected installer DMG file on macOS or Linux, using the specified `.pkg` and policy files, as well as attributes defined in `DS_Store` and `background.png`. The script takes three arguments; A name for the DMG being created, a policy.xml file, and a .pkg file.

  Example:
  ```
  ./mkdmg.sh <DMG filename> </path/to/policy> </path/to/pkg>
  ```
  On macOS, the script uses the built-in utility `hdiutil` to create the DMG file.

  On Linux, the script uses one of two methods to generate a base HFS+ image: (1) `mkfs` using "hfsplus-tools" ([CentOS Extras](https://centos.pkgs.org/7/centos-extras-x86_64/hfsplus-tools-540.1.linux3-4.el7.x86_64.rpm.html), [Fedora](https://src.fedoraproject.org/rpms/hfsplus-tools)) or, (2) fixed size "stock" HFS+ image archived in Git. The algorithm prefers to use "hfsplus-tools", if available, as the base HFS+ image can be customized according to the underlying data and result in an image with less unused disk space. With a base HFS+ image, the open source cross platform library [libdmg-hfsplus](https://github.com/planetbeing/libdmg-hfsplus) is then used to populate the image and create the DMG file. libdmg-hfsplus can be built from source. The bundled utility programs `hfsplus` and `dmg` must be installed for the script to work.

  The commands to build and install libdmg-hfsplus are:
  ```
  cmake3 -S . -B build
  cmake3 --build build
  sudo cmake3 --build build --target install
  ```

- `mkdmgfd.sh`: A script that creates a DMG file on macOS or Linux based on files and directory structure of an input source directory (i.e., Make DMG from directory). This script can be invoked directly and is also used by `mkdmg.sh`.

- `64mb_hfsplus_base_image.dmg.tgz`: An empty 64 MiB HFS+ disk image file stored as a compressed tarball. This is the base image used by `mkdmg.sh` on Linux. Having a base image file available avoids requiring dependencies needed to build an HFS+ image on-demand (e.g., `mkfs.hfs`). A downside of using a fixed-capacity base disk image is more content would require a new larger base image. The final DMG file is compressed so empty space in the HFS+ image will not inflate file size. However, when the DMG is mounted the volume will consume 64 MiB. It is more efficient to match base image capacity to the content as much as possible. A new base image can be built on a Mac using the command:
  ```
  hdiutil create -type UDIF -layout NONE -size <size> -fs HFS+ -volname "cisco_sccm" <output file>
  ```

# Updating DMG Resources

## Constraints

Avoid (or at least be cautious about) changing the directory that the DMG will be mounted to; currently `/Volume/cisco_sccm/cisco-secureclient-cloudmanagement.pkg`.

## Procedure

1) Open Disk Utility > File > New Image > Blank Image...

2) Save as "cisco_sccm" and change the name to match.

3) Make a `/Volumes/cisco_sccm/.background` directory and place the updated background image here.

4) Select the DMG. With `cmd+j` open the view options and set the background image.

5) Place the pkg file in the desired location.

6) Eject the DMG. From Disk Utility > View > Convert select read only or compress.

7) Mount the converted DMG and copy the `.DS_Store` file and used `background.png` files into the repo and open a pull request. When copying the `.DS_Store` file to the repo, rename it to `_DS_Store`.
