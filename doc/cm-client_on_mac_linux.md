# Cloud Management Connector for Mac and Linux

The Cloud Management Connector for Mac and Linux largely follows what was developed before it. Much of the details in this document come from the documentation for the [Cloud Management Connector for Windows](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/Docs/CM.md).

On Mac, and Linux, it will carry out the same responsibilities as it has for Windows.  It is responsible for managing Cisco's security software on the endpoint, syncing with the Cloud Management Backend; installing, updating, and configuring software as required.

The three main components of the Cloud Management Connector client are as follows:

* The cloud management service
    * This is the main service that is responsible for launching the other components.  The service will spawn the CMID and the Package Manager processes.
* Cloud Management Identity (CMID)
    * This component manages the endpoint's identity, providing a common identity to all of Cisco's security software modules.  The component is developed by the AnyConnect team.  The source code for this component can be found [here](https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity).
* Cloud Management Package Manager
    * This component, as its name implies, is in charge of managing Cisco's security software packages.  The component checks in with the cloud to determine if any of the software packages need to be installed or updated.

## Configuration

The Cloud Management Connector and it's individual components get their configuration from two JSON files.  They are described in the table below.

| Configuration    | Filename       | Description                                                  |
| ---------------- | -------------- | ------------------------------------------------------------ |
| Bootstrap        | bs.json        | The initial file that is loaded on start and contains details that tie the service to a business and to service endpoints |
| CM configuration | cm_config.json | A file with configurable parameters that influence the behaviour of the Cloud Management service, Cloud Management Identity and the Package Manager |

When an installer is downloaded, the injection of these initial files are done by a tool called PASS (Package And Signing Service). It packages the Cloud Management Connector service, `bs.json`, `cm_config.json`, and optionally, the rest of the Secure Endpoint and AnyConnect installers.  On macOS, the exact installer package format is not clear (macOS packages would need to be notarized). The package can be packaged as a DMG similar to how Secure Endpoint is currently deployed, where the json configuration can be hidden files inside the DMG image.

* **NOTE** The JSON files are not signed

### Bootstrap Configuration File

An example of a typical Bootstrap file can be found below.  The bootstrap contains the information needed by the Cloud Management service, and Cloud Management Identity component to register a new client identity.  This file is installed with the client, and updated when a new version of the Cloud Management Connector client is installed.

Example of a bootstrap configuration file (bs.json):

~~~json
{
    "business_id": "f8911c82-3083-4bc7-9cff-3857c41206e0",
    "installer_key": "11111111-2222-3333-4444-555541c2c956",
    "identify_url": "https://identify.qa.uc.amp.cisco.com/identify",
    "event_url": "https://identify.qa.uc.amp.cisco.com/event"
}
~~~

| Key           | Description |
| ------------- | ----------- |
| business_id   | A unique ID tied to a specific business         |
| installer_key | A unique key to identify the installer     |
| identity_url  | URL to register a new Cloud Management Identity |
| event_url     | URL used by Cloud Management components for sending events |

See a similar example under the Windows Cloud Management Connector repository [here](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/Resources/config/bs.json)

### Cloud Management Configuration File

This file is also part of the initial installation.  The file has configurable options for the Cloud Management service, Cloud Management Identity component and the Package Manager component.  It is installed with the client, and can be updated as part of the PackageManger check-in.  If a new file is provided, Package Manager will overwrite the file, and reload its configuration.  The Cloud Management service, and CMID will also monitor this file for changes, and reload the file on update.

The complete JSON schema for this configuration file can be found [here](https://code.engine.sourcefire.com/UnifiedConnector/identity-catalog/blob/master/qa/layouts/cm.json)

An example of a simple `cm_config.json` file can be found [here](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/Resources/config/cm_config.json)

~~~json
{
  "id": {
    "enable_verbose_logs": true
  },
  "pm": {
    "loglevel": 7,
    "CheckinInterval": 300000,
    "MaxStartupDelay": 2000,
    "maxFileCacheAge_s": 604800,
    "AllowPostInstallReboots": true
  },
  "uc": {
    "loglevel": 7
  }
}
~~~

The configuration file is split into three main sections.  At the root of the configuration, `id` is meant for configuring the Cloud Management Identity component, `pm` for Package Manager, and `uc` for the main Cloud Management service.

**"id" - Cloud Management Identity Configuration**

| Key                 | Test Only | Description |
| ------------------- | --------- | ----------- |
|  enable_verbose_log | N         | Enable verbose logging |


**"pm" - Package Manager Configuration**

| Key                 | Test Only | Description |
| ------------------- | --------- | ----------- |
| loglevel            | N         | Debug(7), Info(6), ..., Error(3), Critical(2), Alert(1)      |
| CheckinInterval     | N         | Check-in interval in milliseconds                            |
| AllowPostInstallReboots | N     | Allow reboots after installation of security software module |
| MaxEventTTL_s       | Y         | Event Cache TTL in seconds (default: 1 week)                 |
| maxFileCacheAge_s   | Y         | Max File Cache Age in seconds (default: 1 week)              |
| MaxStartupDelay     | Y         | Sleep delay for the very first check-in for PM               |

* **NOTE** See [schema](https://code.engine.sourcefire.com/UnifiedConnector/identity-catalog/blob/master/qa/layouts/cm.json) for full set of log levels

**"id" - Cloud Management Service Configuration**

| Key                 | Test Only | Description |
| ------------------- | --------- | ----------- |
| loglevel            | N         | Debug(7), Info(6), ..., Error(3), Critical(2), Alert(1)      |

### Product Update Window

The update window functionality is provided by the backend. If it is specified and a connector checks in outside the window, the backend will provide an empty checkin request, so the client will do nothing. The client's responsibility is to report the timezone offset in the checkin request to allow the backend to offset the window accordingly.

## Cloud Management Identity (CMID)

The CMID component is provided by the AnyConnect team.  The shared library for Windows, Mac and Linux can be found here in GitHub.

https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity

Building [cm-client](https://code.engine.sourcefire.com/UnifiedConnector/cm-client) will automatically build CMID in its entirety.

The CMID component is largely self-contained.  It provides the following components.

* CMID Process - PATH: `.../bin/csc_cmid`, aka `csc_cmid`
    * Service that is to be bundled as part of the installer package
* CMID Control - PATH: `.../lib/libcmidcontrolplugin.a`, aka `libcmidcontrolplugin`
    * A library to be used by the CM service to launch and control the CMID process
* CMID client API - PATH: `.../lib/libcmidapi.dylib`, aka `libcmidapi`
    * This library is used by other security software modules (i.e. Secure Endpoint and AnyConnect) to connect to the CMID process to retrieve the CMID (Cloud Management Identity) / refresh token

The diagram below illustrates where these components reside in the Cloud Management ecosystem.  Components external to the CMID library are shown in dashed lines.

~~~
    .─────────.                                                         .─────────.
 ,─'           '─.                                                   ,─'           '─.
;       PM        :                                                 ;      CMID       :
:      Cloud      ;                                                 :     Cloud       ;
 ╲               ╱                                                   ╲               ╱
  '─.         ,─'                                                     '─.         ,─'
     `───────'                                                           `───────'
         ▲                                                                   ▲
         │                                                                   │
         │                  ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐                │
         ▼                     Cloud Management Connector                    │         ┌ ─ ─ ─ ─ ─ ─ ─ ─ ┐
 ┌ ─ ─ ─ ─ ─ ─ ─ ┐          │        client service         │                │           Secure Endpoint
  Package Manager                                                            ▼         │                 │
 │    Service    │  start / │                               │  start / ┌──────────┐
   ┌──────────┐      stop          ┌──────────────────────┐     stop   │ csc_cmid │    │  ┌──────────┐   │
 │ │libcmidapi│  │◀─────────│      │ libcmidcontrolplugin │─┼─────────▶│          │       │libcmidapi│
   └──────────┘                    └──────────────────────┘            └──────────┘    │  └──────────┘   │
 │       ▲       │          │                               │                ▲                  ▲
  ─ ─ ─ ─│─ ─ ─ ─            ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─                 │         └ ─ ─ ─ ─│─ ─ ─ ─ ┘
         └───────────────────────────────────────────────────────────────────┴──────────────────┘
                                                                 Get ID / Refresh token
~~~

### Control Plug-in Library and CMID Process

The Cloud Management Connector service interacts with the CMID process through the CMID control plug-in library.  The control plug-in library is in charge of starting, monitoring, and stopping the CMID process.

For more details on the Control Plug-in see the code the [CMID Control Plug-in Source](https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity/tree/master/cmid/controlplugin).

The module to start and stop the CMID process is through an abstraction layer defined by the API in [PackageManagerInternalModuleAPI.h](https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity/blob/master/cmid/external/include/PackageManagerInternalModuleAPI.h).

Basic use of this API is defined as follows:

~~~cpp
//Create a module instance - REQUIRED
MODULE_CTX_T  modContext = {0};
modContext.nVersion = PM_MODULE_INTERFACE_VERSION;
CreateModuleInstance(&modContext);

modContext.fpStart(pathToBinary, pathToData, pathToConfig);
~~~

To stop the CMID process, use the module instance earlier with API as follows:

~~~cpp
modContext.fpStop();
ReleaseModuleInstance(&modContext);
~~~

On a configuration update, call the `fpConfigUpdated()` method with the module instance defined above.

### CMID API library

Software modules such as Secure Endpoint will use the CMID API library to get the Cloud Management Identity and refresh token.

The public API of the CMID API library is defined in [CMIDAPI.h](https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity/blob/master/cmid/public/include/CMIDAPI.h).  For a full list of C interfaces it exports, refer to the header file.

Some APIs worth highlighting are:

~~~c
//! Gets the CMID
cmid_result_t
cmid_get_id(IN OUT char* p_id, IN OUT int* p_buflen);

//! Gets the token associated with the CMID
cmid_result_t
cmid_get_token(IN OUT char* p_token, IN OUT int* p_buflen);

//! Refreshes the token that is associated with the CMID
cmid_result_t
cmid_refresh_token();
~~~

> *TODO* How often does one call this method?

## Package Manager (PM)

While each OS will have it's own Package Manager, they all share a common [PackageManager library](https://code.engine.sourcefire.com/UnifiedConnector/PackageManager).

The shared library provides the following functionality:
- Centralized software configuration and package discovery
- Sharing of software configuration and package information with the Package Manager cloud
- Orchestration of package and configuration updates, such as:
    - Downloading a list of configuration and package updates
    - Installation of software packages and configuration

~~~
**NOTE** Removing a product has not yet been implemented
~~~

### Package Manager Exported Interfaces and libraries

Package manager exports public headers with interfaces that the Mac and Linux Cloud Management Connector client must implement for the OS specific Package Manager process.  These header files that export these interfaces are described below.

~~~
                               ┌───────────────────────────────────┐
                               │            PM Process             │
                               │                                   │
                               │ ┌───────────────────────────────┐ │
                               │ │          IPmLogger.h          │ │
┌──────────────────────────┐   │ └───────────────────────────────┘ │
│ Cloud Management Service │   │ ┌───────────────────────────────┐ │
│                          │   │ │ IPmPlatformComponentManager.h │ │
│  ┌─────────────────────┐ │   │ └───────────────────────────────┘ │
│  │ PM Service Control  │─┼──▶│ ┌───────────────────────────────┐ │
│  └─────────────────────┘ │   │ │  IPmPlatformConfiguration.h   │ │
└──────────────────────────┘   │ └───────────────────────────────┘ │
                               │ ┌───────────────────────────────┐ │
                               │ │  IPmPlatformDepdendencies.h   │ │
                               │ └───────────────────────────────┘ │
                               │ ┌───────────────────────────────┐ │
                               │ │     IPackageManager.h(*)      │ │
                               │ └───────────────────────────────┘ │
                               └───────────────────────────────────┘
~~~

| Name | Description |
| ---- | ----------- |
| IPmLogger.h | <ul><li>A common interface for handling logging</ul> |
| IPmPlatformComponentManager.h | <ul><li>Handles package discovery, installation and uninstallation</ul> |
| IPmPlatformConfiguration.h | <ul><li>Handles configuration data such as identity, ssl certificates, directories and proxy information</ul> |
| IPmPlatformDependencies.h | <ul><li>A container around IPmPlatformComponentManager and IPmPlatformConfiguration</ul> |

~~~
NOTE
  In `IPmPlatformComponentManager.h` some APIs are for windows only and are marked as optional.
  For functions that are not applicable, the function can just return success.
~~~

~~~
NOTE
  (*) IPackageManager.h is not listed in the table above because the interface it exports is
  already implemented by the PM library.  It also exports functions to initialize,
  de-initialize and retrieve the PM instance.
~~~

In addition to the exported interfaces, PackageManager also provides a header for Package Manager specific types.

| Name      | Description |
| --------- | ----------- |
| PmTypes.h | Package Manager defined types |

### Package Manager Library Integration

The Cloud Management client service spawns the Package Manager process.  Creating the process, is similar to any other long running process.

When the process is first started, its responsibility is to start the PM service.  This involves starting the main PM module -- which involves calling `InitPackageManager()`, acquiring the PM module through `GetPackageManagerInstance()`, and starting it.  The concrete classes that implement the PM exported interfaces are used to configure the PM module.

As reference, the Windows Package Manager defines a `PmAgent` class that can be used as a reference to show how all the components come together.  See the implementation of the class [here](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/WindowsPackageManager/libPmAgent/PmAgent.cpp).

The main entry point of the process will instantiate the `PmAgent` and start it.  See the Windows Package Manager code [here](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/WindowsPackageManager/WinPackageManager/WinPackageManager.cpp) for reference.

### Package Manager Platform Dependencies

### Start and monitor the PM process

The responsibility of the Cloud Management connector service is to spawn off a separate OS specific Package Manager (PM) process.  This includes starting, monitoring (keep-alive), and the stopping of the PM process.  The PM library does not provide this functionality but this ability is provided by CMID and the code to do that can be used as a reference.

### Identity catalog

The list of configuration and package updates are defined by the identity catalog.  The identity catalog is a JSON file with a list of packages and configurations to download and apply.

The identity catalog for the Windows service can be found in GitHub [here](https://code.engine.sourcefire.com/UnifiedConnector/identity-catalog).

A similar catalog must be defined and created for Mac and Linux.

> *TODO* Document the Mac (and Linux) catalog proposal

### Install package updates

The two most common ways to install an application on macOS is through using a `.pkg` installer package, or an application bundle, which the end-user places under the `/Applications` directory.

Package manager for Mac will support the installation of `.pkg` installation packages.  A package signing service such as PASS, is possible but not necessary at this point.  One of the reasons packages cannot be signed "on-the-fly" with an embedded configuration is due to Apple's requirement for packages to be notarized.

~~~
**NOTE** Due to Apple's notarization requirements, a process must send the installer to
         Apple's notarization service to be notarized prior to distribution.  It is due to
         this restriction that it's not possible to inject an installation package with
         configuration files at time of download.
         Secure Endpoint connector on Mac gets around this limitation by embedding a signed
         and notarized installer with hidden configuration files in a `.dmg` disk image.
~~~

If an installer needs to be bundled with a configuration, an installation packages may also come bundled as part of an Apple `.dmg` disk image.  The disk image as the name implies, can contain multiple packages and is able to embed configuration files in addition to the the installation package itself.  This type of package can embed an already notarized development package.

Although, the `.dmg` disk image can satisfy the requirements of a notarized installer with a configuration, the identity catalog will allow the configuration to be deployed separate from the installer.  By deploying the configuration separately, then deploying an installer it's possible for Package Manager to only support `.pkg` installers.  A similar approach to this was implemented for the Windows Secure Endpoint connector and is described in this [JIRA ticket](https://jira-eng-rtp3.cisco.com/jira/browse/AMP4E-15586).  For services that would follow a similar model the workflow would be as follows:
1. Package Manager installs a notarized secure software module installer package (with no configuration)
1. Package Manager deploys secure software module configuration to /opt/cisco/secureclient/cloudmanagement/`<module>`/bootstrap.xml
1. Secure software module detects bootstrap.xml
1. Module validates and loads bootstrap.xml
1. Module checks for configuration update

On Linux, the Package Manager will need to support both Debian and Red Hat installation packages.

When the list of packages to install are obtained, the installers are first downloaded into an installer cache directory.  Then the packages will be installed, one after another from the cache directory.

~~~
**NOTE** When there are packages to install, all packages are downloaded first prior to installation
         because installation of programs may interrupt download progress
~~~

An event is sent with each attempt to install a configuration, or software package.  The PM library is responsible for sending these events.

#### Install Package Clean up

Upon successful installation, clean up will attempt to re-send any events that failed to send, and clean up any installers in the installer cache.

> **TODO** Determine the responsibility of the Mac/Linux team with this clean up work

## Cloud Management Connector Installer Package

The Cloud Management Connector package will be distributed as a dmg.  The dmg disk image will contain the following:

- **Required**: The main Cloud Management Connector `.pkg` installer package
- **Required**: Hidden bootstrap and CM JSON configuration files
- **Optional**: Additional Cisco Security software packages (i.e. Cisco Secure Client, and Cisco Secure Endpoint)
- **Optional**: Hidden configuration files for other Cisco security software packages

It is the responsibility of the main installation package to also install the additional software packages that are bundled into the dmg disk image.  If possible this should be done as a post install step, or, if not, we could place these packages into the installer cache as part of that final step.  On launch of the Package Manager, it can check for the presence of these packages in the cache and attempt the installation on start up.

The responsibility of the main Cloud Management installer is to:
- Install the Cloud Management Service
- Install any optional .pkg packages which can include
    - Secure Client and modules such as VPN, DART, ISE, NAM, NVM, posture, sbl, umbrella, etc...
    - Secure Endpoint connector
- Install the launchctl (Mac), or systemd(Linux) service configuration to start the Cloud Management service
    - Any optional .pkg packages will install their own service configuration

### Cloud Management Installation Paths

The installer will install files to the following paths:

| File Types                | Path |
| ------------------------- | --------------------------------------------- |
| Executables and Libraries | /opt/cisco/secureclient/cloudmanagement/bin   |
| Configuration Files       | /opt/cisco/secureclient/cloudmanagement/etc   |
| Data Files                | /opt/cisco/secureclient/cloudmanagement/etc   |
| Crashes                   | /opt/cisco/secureclient/cloudmanagement/crash |

**NOTE** The paths chosen are to try to guidelines typically used for UNIX-like operating systems.  See [FHS](https://www.pathname.com/fhs/pub/fhs-2.3.html).

Data files are currently being placed in the same directory as configuration files.  It is a location to locate both files for configuration of the Cloud Management service and for components such as CMID and Package Manager to store it's own configuration.

The configuration files are referring to the configuration for Cloud Management and the Cloud Management components only and not for other Cisco security software modules.

For locations of log files see the section later on [Logging by CMID](#logging-by-cmid).

These installation paths will be common for both Mac and Linux.  On Mac, we can also package Cloud Management as an application bundle and place it in `/Applications`.  This can be an option, however, since there's no graphical interface for Cloud Management, there is no need to place it under `/Applications`.  It maybe valuable to consider distributing as an application bundle on Mac in the future. Currently, for consistency, it's better to share the same paths and layout for both Mac and Linux.

### Events sent by the installer

> *TODO* Determine what events are sent by the installer
>        Will this be handled internally by the PM library?

## Graphical assets

The Windows Cloud Management Connector service is linked below.  The icon will need to be adapted to use a theme that is more consistent with macOS.

https://code.engine.sourcefire.com/raw/UnifiedConnector/WindowsUnifiedConnector/master/Resources/images/small.ico?token=GHSAT0AAAAAAAAAA6ISKX3SLE6Y25D352S6YX75V7Q

## Logging with Cloud Management Connector client

On Windows, the team has been looking at [spdlog](https://github.com/gabime/spdlog) for logging.  It's a header only, C++ logging library that supports multiple platforms.

This is a good opportunity to adopt this as a common logger across connector teams in new projects moving forward.

These are the proposed log paths:

* On macOS
    - /Library/Logs/Cisco/SecureClient/CloudManagement/
* On Linux
    - /var/logs/cisco/secureclient/cloudmanagement/

The paths chosen above are standard logging locations for the respective OS' and it will make the most sense to locate the files where a system administrator will expect to find them.

CMID libraries and the CMID process will log to their own individual log files.  For instance, the control plugin logs will be under the name `csc_cmid_control_plugin.log` in the proposed paths above.

## Events

The PM component logs events to notify the Cloud Management server of various activities such as when a package is installed, an installation failed, or when a security software package was reconfigured.

See [here](https://code.engine.sourcefire.com/UnifiedConnector/identity/blob/master/etc/events-v1.json) for the definitions of the events.

* *TODO* Determine which of the events are missing and which are already sent as part of the CMID or PM library

## Proxy support

The Cloud Management connector components will need to support auto-disocovery of proxies.

Proxy settings for the Cloud Management service is not provided by any policy or configuration file.  To determine the proxy configuration, proxy settings must be done using the native APIs of the OS platform.  Native APIs do not provide proxy passwords, which means only non-authenticated proxies will be supported.

On macOS, this can be done using the Core Foundation [CFNetworkCopySystemProxySettings](https://developer.apple.com/documentation/cfnetwork/1426754-cfnetworkcopysystemproxysettings?language=objc)() API.

## Crash Handling

Crash handling will be similar to that of the Secure Endpoint connector, and crashes will be handled by Google's Crashpad.

Crashes are uploaded to *TBD*.

## Diagnostics tool

The Cloud Management Connector will include a diagnostic tool.

The tool will package up the configuration and data directories and create a zip file on the desktop. The tool requires elevated privileges as some data files require elevation to read. An option may need to be provided to run without elevated privileges which will cause those files to be dropped from the package for integration with AnyConnect's DART tool.

* **TODO** Determine the list of configuration files, logs and data directories to include as part of the Diagnostics tool

# Development Environment

Early exploratory work has begun and can be found in the [UnifiedConnector/cm-client](https://code.engine.sourcefire.com/UnifiedConnector/cm-client) repository.

To maximize the ability to share code where components are similar between macOS and Linux, the project will use cmake as the build system.

The software components for Cloud Management Identity [UnifiedConnector/EndpointIdentity](https://code.engine.sourcefire.com/UnifiedConnector/EndpointIdentity) and the Package Manager library [UnifiedConnector/PackageManager](https://code.engine.sourcefire.com/UnifiedConnector/PackageManager) are both submodules at the root of the repository.

External third-party components will go into a `third-party` directory.

The prototype that's being developed is still being developed and will evolve with this documentation.

## Directory Structure

| Directory        | Description |
| ---------------- | ----------- |
| EndpointIdentity | Cloud Management Identity (CMID) _submodule_    |
| PackageManager   | Common Package Manager (PM) library _submodule_ |
| client           | Cloud Management service                        |
| cmake            | Cmake build scripts                             |
| third-party      | Third-party libraries (see next section)        |

### Third-party libraries

> **NOTE** All third-party libraries are pulled into the repository as submodules.

| Directory        | Description |
| ---------------- | ----------- |
| ciscossl         | CiscoSSL                   |
| curl             | curl                       |
| fff              | Fake Function Framework    |
| gtest            | Google Test                |
| jsoncpp          | jsoncpp - C++ JSON library |

## Building

The proposed build system uses CMake.

The goal was to use CMake by _itself_ without any additional scripts to configure and build the Cloud Management Connector client.  For that to be possible, third-party components would also need to be built using the single build system.  Initial investigations into this possibility revealed the [ExternalProject](https://cmake.org/cmake/help/latest/module/ExternalProject.html) command.  The command makes it possible to build third-party projects that are built using either CMake or configure scripts.

CiscoSSL (aka OpenSSL) is a more complicated build that uses a Perl build script.  Due to this limitation, a build script was added to first build CiscoSSL, and then CMake is then used to build everything else.

On the base of the repository, there's a `build` symlink that is symlink to a shell script that can be used to build the entire project.  When the script is run it will.
1. Create a `debug` build directory for CMake, then runs `cmake` to create the preferred build system (by default it create a Makefile build - todo make this configurable)
1. Calls CMake to start the build.  This will start by:
    * Calling a script to build CiscoSSL (as a Universal binary for Mac, or a standard build for Linux)
    * Use cmake modules (each containing an ExternalProject definition) to build library dependencies
    * Builds the Cloud Management Connector client.

After the `build` script has completed, after any subsequent changes, either run the `build` command again, or run `cmake --build debug` to build again.  On Mac projects using Xcode, developers can also use Xcode as an IDE for modifying, adding, debugging and building the source.

### Export directory

All build products including the exports from external projects, are all placed in the `debug` directory, under `./debug/export/{include,lib}`.  This makes it easy to do a clean build by just removing a single directory.  However, because CiscoSSL is not built with CMake it will still need to be manually cleaned.

As a convenience run the command `./build clean` to clean the CiscoSSL project, and also remove the `debug` directory to start new.

## Software Development Practices and Guidelines

The project will adhere to CSDL (Cisco Secure Development Lifecycle) guidelines.  See link for additional details:

https://cisco.sharepoint.com/sites/CSDL

The project will try to follow the existing coding standard established by the Mac, and Linux connector team.  The coding standard can be found here:

https://code.engine.sourcefire.com/Cloud/ampcxi/blob/main/doc/coding_standards.md

To keep with established practices of the Mac, Linux connector team, the Cloud Management Connector client will continue with the practice of establishing a Software Specification similar to the one found here:

https://code.engine.sourcefire.com/Cloud/ampcx/blob/master/doc/ampcx_sw_spec.md

The above document will aid in the CSDL process and document how CSDL requirements are addressed as part of this project.

Prior to starting the project GitHub pull requests will be tied to a CI pipeline.  At the time of this writing, the quickest way to do this is just to integrate with the existing ampcx Mac, and Linux Jenkins server.

https://clg5-lab-macjenkins.cisco.com

As with all CI pipelines integration will run the repository through the following stages:

- Build (Debug and Release)
- Tests
- Static Analysis

When it comes time to release, Jenkins can also produce release builds as well that can be used for distribution.

# References

[Cloud Management Connector for Windows](https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/Docs/CM.md)
[ampdocs](https://code.engine.sourcefire.com/Cloud/ampdocs/tree/master/connector/win/design/UnifiedConnector)
