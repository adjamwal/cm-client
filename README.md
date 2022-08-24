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

# Build

To build cm-client, use the `build` symlink on the root of the repository.  Usage for this script is as follows:

~~~
Usage: build [-c|-h]
 -c    clean build
 -h    help (this usage)

 * Run without any arguments to build cm-client
~~~

This will build all third-party dependencies and the Cloud Management connector client.

On a subsequent build after making code changes, to build just use `make` (**TODO** it's easy to switch the build system to Xcode instead but for simplicity, we currently only support `make`).

~~~
* make code changes *
cd debug
make
~~~
