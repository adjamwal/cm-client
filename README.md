# Cloud Management Connector client for macOS and Linux

The Cloud Management Connector for Mac and Linux largely follows what was developed before it. Much of the details in this document come from the documentation for the [Cloud Management Connector for Windows](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/Docs/CM.md).

On Mac, and Linux, it will carry out the same responsibilities as it has for Windows.  It is responsible for managing Cisco's security software on the endpoint, syncing with the Cloud Management Backend; installing, updating, and configuring software as required.

The three main components of the Cloud Management Connector client are as follows:

* The cloud management service
    * This is the main service that is responsible for launching the other components.  The service will spawn the CMID and the Package Manager processes.
* Cloud Management Identity (CMID)
    * This component manages the endpoint's identity, providing a common identity to all of Cisco's security software modules.  The component is developed by the AnyConnect team.  The source code for this component can be found [here](https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity).
* Cloud Management Package Manager
    * This component, as its name implies, is in charge of managing Cisco's security software packages.  The component checks in with the cloud to determine if any of the software packages need to be installed or updated.

# Getting started

Fork the `UnifiedConnector/cm-client` repository.  Since this repository includes submodules, the command to clone the forked repository is:

```
git clone --recursive git@code.engine.sourcefire.com:<your username>/cm-client.git
```

Next, add an upstream remote for fetching from the shared UnifiedConnector repository:

```
git remote add upstream git@code.engine.sourcefire.com:UnifiedConnector/cm-client.git
```

To update the submodules:

```
git submodule sync --recursive
git submodule update --init --recursive
```

# Making Changes

Before making changes, make sure your branch you are starting with has the latest code from upstream.  To update to the latest source and submodules, use the commands:

```
git pull upstream master
git submodule sync --recursive
git submodule update --init --recursive
```

To make changes to files that are not part of a submodule:

```
git checkout -b <branch name>
* Make changes *
git add ...
git commit ...
# Using -u will automatically set the branch to track origin
git push -u origin <branch name>
* Create pull request in GitHub *
```

# Environment

To be able to build cm-client, the following software packages are required:

- CMake - which can be acquired from [here](https://cmake.org/download/)
- Xcode - 12.2 and higher (Minimum for Universal binaries)
- Xcode Command Line Tools - running `clang`, `gcc` or similar commands will trigger a prompt, or just run `xcode-select --install` to begin the install process.

# Build

To build cm-client, set the CM_BUILD_VER environment variable and run the `build` symlink on the root of the repository.  Usage for this script is as follows:

~~~
export CM_BUILD_VER=1.0.0

Usage: build [-c|-h]
 -c    clean build
 -h    help (this usage)
 -r    release (default: debug)
 -x    Xcode project generator (macOS only)

 * Run without any arguments to build cm-client
~~~

This will build all third-party dependencies and the Cloud Management connector client.

On a subsequent build after making code changes, to build just use `make` (**TODO** it's easy to switch the build system to Xcode instead but for simplicity, we currently only support `make`).

~~~
* make code changes *
cd debug
# Alternatively, you can also build with the command
# cmake --build .
make
~~~

## Exported third-party and first-party libraries

First and third party libraries are all exported into the cmake build directory `debug` under the directory `export`.  In the `debug/export`, it will contain exported headers and libraries for the following -

Third-party components:
* curl
* ciscossl
* jsoncpp
* Google Test

First-party components:
* PackageManager (PM)
* Cloud Management Identity (CMID)

## Xcode (experimental)

To use the Xcode build system in macOS, configure the build using the following command.

~~~sh
$ ./build -x
~~~

It's currently labelled as experimental because the install steps will succeed, but for some reason the artifacts that are supposed to be copied over are not available after install.  At this point it's not clear why that is the case.

If this does occur then the install step must be run manually to export the header for the particular library that failed to install its headers and libraries to the cmake `export` directory.

~~~sh
# Find the build directory for the specific third-party component where the exports are missing
cd ./debug/third-party/<third-party-component>/src/third-party-<third-party-component>-build
# From within that directory run
cmake --build . --config Debug --target install
~~~

Examples where this might be the case include `<third-party-components>` such as `jsoncpp`, and `gtest`.
