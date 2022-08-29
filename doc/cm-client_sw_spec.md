**Cloud Management connector for macOS and Linux: Software Requirement Specification**

This document captures the high level software design and requirements for two related products:
1. Cloud Management connector for macOS (OS X), and
2. Cloud Management connector for Linux.

It is authored and maintained by the software development team to serve two purposes:
* It is the specification that captures the software's required behaviour, and
* It is the specification from which Connector-level black-box tests are developed.

The existence of this document does not imply software requirements need be finalized before design or implementation work begins.  However, existing requirements should be considered prior to making a software change. Requirements can be added, removed or updated as part of the development process.

Contents:

<!-- TOC depthto:3 withlinks:true updateonsave:true anchormode:github.com -->

- [Requirement Standard](#requirement-standard)
    - [Best Practices](#best-practices)
        - [Writing requirements](#writing-requirements)
        - [Deleting requirements](#deleting-requirements)
    - [Standard Terms](#standard-terms)
    - [Acronyms](#acronyms)
- [Reference Documents](#reference-documents)
- [Configuration Parameters](#configuration-parameters)
- [Design Description and Requirements](#design-description-and-requirements)
    - [Directory Structure](#directory-structure)
        - [Mac Connector Directory Structure](#mac-connector-directory-structure)
        - [Linux Connector Directory Structure](#linux-connector-directory-structure)
    - [General](#general)
        - [System Context](#system-context)
    - [Supported Platforms](#supported-platforms)
    - [Versioning](#versioning)

<!-- /TOC -->

# Requirement Standard

In general, requirements have all of these attributes:
* __Clear__  - _in what is required_
* __Complete__ - _without missing information_
* __Consistent__ - _with other requirements_
* __Verifiable__ - _such that the expected output is deterministic_
* __Implementation-free__ - _so to not artificially constrain design_

The high-level software design and architecture are described in prose using natural language.  Key aspects that require verification are tagged as software requirements and written using formal requirement language.  It is acceptable for requirements to reference tables and diagrams such as UML state-charts and activity diagrams provided they unambiguously and accurately convey the requirement's intent.

Each "requirement" uses one of the following terms to show significance:
* __Shall__ - denotes mandatory requirement.  The product is complete when all mandatory requirements are met.
* __Should__ - denotes non-mandatory provision, such as a design goal.  Consideration is warranted but it is a "nice-to-have" or desirable to support future evolution.

A requirement may contain supplemental text beginning with these special keywords:
* __Exemption__ - Specifies when a requirement is not applicable without complicating the requirement text.  It is an alternative to "if... but not when..."
* __Justification__ - Explains why the requirement is necessary when the rationale is not obvious.
* __Note__ - Provides additional commentary and a means to illustrate using examples. Although the requirement text should be complete on its own, a note can highlight nuances and enhance understanding.

For brevity, the term "Connector" refers to the Secure Endpoint software.  Requirements apply to both macOS and Linux platform families unless stated otherwise.  Requirements that only apply to one platform are tagged <sup>Mac</sup> or <sup>Linux</sup>.

## Best Practices

### Writing requirements

An important goal of this software specification is to provide guidance for user level test cases. To this end, requirements should focus on the Connector's black-box behavior; inputs are controllable and the output of actions are observable. The recommended sentence structure for requirements is:

> The Connector shall [perform an action] if [condition] is met.

To minimize ambiguity, a single requirement should specify a single action.  However there are cases where specifying multiple actions triggered by the same condition in a single requirement can improve readability without introducing ambiguity (e.g., when the order of actions does not affect the output).  An acceptable sentence structure for specifying two actions is:

> The Connector shall, if [condition] is met, [perform action 1], and [perform  action 2].

In cases where a requirement specifies multiple conditions, the recommended structure is to use an unordered list with either "any" or "all":

> The Connector shall [perform an action] if [any|all] of the following conditions are met:
> - Condition 1
> - Condition 2
> - ...
> - Condition N

Similarly, if a requirement specifies multiple independent actions, the recommended structure is to use an unordered list for the actions:

> The Connector shall, if [condition] is met, perform all of the following actions:
> - Action 1
> - Action 2
> - ...
> - Action N

The use of serial comma is encouraged in cases where it resolves ambiguity.

### Deleting requirements

When the Connector behaviour is changed in a way that makes existing requirements obsolete, mark that requirement as "_Deleted. Followed by an optional reason for the change._" This makes it easier to track changes in the requirements over time and prevents re-use of the same requirement ID as doing so would cause problems when, say, an external document referenced that requirement before it was repurposed.

## Standard Terms

| Term                      | Description
|---------------------------|------------
| Cloud Management          | The Cloud Management client and server provides a service that is responsible for providing a common endpoint identity, and managing Cisco's security software on the endpoint.
| Crash Report              | A binary object containing detailed information about a process that crashed due to a software exception. This includes the software version, platform name, unique client ID, and a snapshot of the process' working memory before the crash occurred.
| Package Manager           | The Cloud Management software component for installing, and updating Cisco's security software
| Cloud Management Identity | Refers to the common identity shared between all Cisco's security software

## Acronyms

| Term                    | Description
|-------------------------|------------
| CLI                     | Command Line Interface
| GUI                     | Graphical User Interface
| RHEL                    | Red Hat Enterprise Linux
| EL                      | Enterprise Linux := RHEL-based Linux
| RPM                     | RPM Package Manager (originally Red Hat Package Manager)
| TTL                     | Time to Live
| UI                      | User Interface

# Reference Documents
<table>
  <tbody>
    <tr><th>Tag</th><th>Document</th><th>Applicable Revision</th></tr>
    <tr>
      <td>[AppleTN2459]
      <td><a href="https://developer.apple.com/library/content/technotes/tn2459/_index.html#//apple_ref/doc/uid/DTS40017658">Secure Kernel Extension Loading</a>
      <td>Latest
    </tr>
    <tr>
    <td>[Cloud Management Connector for Windows]
    <td><a href="https://code.engine.sourcefire.com/UnifiedConnector/WindowsUnifiedConnector/blob/master/Docs/CM.md">Clound Management Connector for Windows</a>
      <td>Latest
    </tr>
    <tr>
      <td>[MacSignedFiles]
      <td><a href="https://code.engine.sourcefire.com/Cloud/ampcx/wiki/Mac-Signed-Files">Mac Signed Files</a>
      <td>Latest
    </tr>
    <tr>
      <td>[LinuxSignedFiles]
      <td><a href="https://code.engine.sourcefire.com/Cloud/ampcx/wiki/Linux-Signed-Files">Linux Signed Files</a>
      <td>Latest
    </tr>
    <tr>
      <td>[Crashpad]
      <td><a href="https://code.engine.sourcefire.com/Cloud/ampcx/wiki/Crashpad-and-Crash-Minidumps">Crashpad and Crash minidumps</a>
      <td>Latest
    </tr>
  </tbody>
</table>


# Configuration Parameters

The Connector is configured through XML configuration files.  The list of supported parameters and the corresponding XPaths are as follows:

<table>
  <tbody>
    <tr><th>Parameter</th><th>XPath</th></tr>
    <tr><td>Cloud Management Connector Log Level<td>uc/loglevel</tr>
    <tr><td>Cloud Management Identity Verbose Logging<td>uc/enable_verbose_logs</tr>
    <tr><td>Package Manager Log Level<td>pm/loglevel</tr>
    <tr><td>Package Manager Check-in Interval<td>pm/CheckinInterval</tr>
  </tbody>
</table>

# Design Description and Requirements

## Directory Structure

### Mac Connector Directory Structure

<table>
  <tbody>
    <tr>
      <th>Directory</th>
      <th>Path</th>
    </tr>
    <tr>
      <td>Application Support Directory
      <td>/Library/Application Support/Cisco/Cloud Management Connector
    </tr>
    <tr>
      <td>Crash Handler Report Directory
      <td>/Library/Application Support/Cisco/Cloud Management Connector/ch
    </tr>
  </tbody>
</table>

### Linux Connector Directory Structure

<table>
  <tbody>
    <tr>
      <th>Directory</th>
      <th>Path</th>
    </tr>
    <tr>
      <td>Crash Handler Report Directory
      <td>/opt/cisco/cm/etc/ch
    </tr>
  </tbody>
</table>

## General

### System Context
```
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
```

## Supported Platforms

This section defines the minimum list of operating system versions supported by the Cloud Management Connector.  Supporting a version implies the connector has been verified to meet all functional and non-functional (e.g., performance, security) requirements applicable on platform unless stated otherwise.  While there may be reasons to support versions beyond the minimum list (e.g., an important customer asks for support for an older version), that is an exception and should be handled on a temporary and case-by-case basis.

<table>
  <tbody>
    <tr><th>Requirement</th><th>Text</th></tr>
    <tr>
      <td>PLAT_010<sup>Mac</sup>
      <td>The Connector shall support the two most recent patch versions for the latest version of macOS
    </tr>
    <tr>
      <td>PLAT_011<sup>Mac</sup>
      <td>The Connector shall support the latest patch release for macOS versions n-1 and n-2; where n denotes the latest version of macOS
      <br><i>Example: If the latest version of macOS is 11, then we support the latest patch release for 10.15 and 10.14.</i>
    </tr>
    <tr>
      <td>PLAT_120<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) CentOS 7.
    </tr>
    <tr>
      <td>PLAT_121<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) CentOS 8.
    </tr>
    <tr>
      <td>PLAT_122<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) AlmaLinux 8.
    </tr>
    <tr>
      <td>PLAT_123<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) Rocky Linux 8.
    </tr>
    <tr>
      <td>PLAT_130<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Red Hat Enterprise Linux 6.
    </tr>
    <tr>
      <td>PLAT_140<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Red Hat Enterprise Linux 7.
    </tr>
    <tr>
      <td>PLAT_141<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Red Hat Enterprise Linux 8.
    </tr>
    <tr>
      <td>PLAT_150<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) Oracle Linux 6 (RHCK).
    </tr>
    <tr>
      <td>PLAT_160<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) Oracle Linux 7 (RHCK and UEK).
    </tr>
    <tr>
      <td>PLAT_161<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) Oracle Linux 8 (RHCK and UEK).
    </tr>
    <tr>
      <td>PLAT_162<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of OpenSuse Leap 15.0.
    </tr>
    <tr>
      <td>PLAT_163<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Suse Linux Enterprise 15 (SP0).
    </tr>
    <tr>
      <td>PLAT_164<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of OpenSuse Leap 15.1.
    </tr>
    <tr>
      <td>PLAT_165<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Suse Linux Enterprise 15 SP1.
    </tr>
    <tr>
      <td>PLAT_166<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of OpenSuse Leap 15.2.
    </tr>
    <tr>
      <td>PLAT_167<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Suse Linux Enterprise 15 SP2.
    </tr>
    <tr>
      <td>PLAT_168<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of OpenSuse Leap 15.3.
    </tr>
    <tr>
      <td>PLAT_169<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Suse Linux Enterprise 15 SP3.
    </tr>
    <tr>
      <td>PLAT_170<sup>Linux</sup>
      <td>The Connector shall support the two most recent point releases of Ubuntu 18.04 LTS.
    </tr>
    <tr>
      <td>PLAT_180<sup>Linux</sup>
      <td>The Connector shall support the two most recent point releases of Ubuntu 20.04 LTS.
    </tr>
    <tr>
      <td>PLAT_181<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Debian 10.
    </tr>
    <tr>
      <td>PLAT_182<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Debian 11.
    </tr>
    <tr>
      <td>PLAT_185<sup>Linux</sup>
      <td>The Connector shall support the 18 most recent releases of Amazon Linux 2.
      <br><i>Notes:
        <ol>
          <li>Amazon Linux 2 is a rolling release.
          <li>Refer to <a href="#reference-documents">[AmazonLinux2ReleaseNotes]</a> for information about Amazon Linux past releases.
        </ol>
      </i>
    </tr>
    <tr>
      <td>PLAT_190<sup>Mac</sup>
      <td>The Connector shall support the following architectures:
      <ul>
        <li>x86-64
        <li>ARM64
      </ul>
    </tr>
    <tr>
      <td>PLAT_200<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) AlmaLinux 9.
    </tr>
    <tr>
      <td>PLAT_201<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of Red Hat Enterprise Linux 9.
    </tr>
    <tr>
      <td>PLAT_202<sup>Linux</sup>
      <td>The Connector shall support the two most recent versions of (EL) Oracle Linux 9 (RHCK and UEK).
    </tr>
  </tbody>
</table>

## Versioning
This section defines how the connector version is determined and represented.

A connector version is identified by its platform identifier (e.g. CentOS6, CentOS7, or Mac) and its version identifier which is of the form `<major>.<minor>.<patch>.<build>`. The platform identifier is typically used in the filename for the install package while the version number is diaplayed in various places including the Portal and UIs.

The `<major>` number is not expected to change often, if at all.<br>
The `<minor>` number is incremented regularly for each release.<br>
The `<patch>` number is used when a release is needed but the next regular release either includes too many changes or cannot be released for some time.<br>
The `<build>` number, if present, is an ever increasing number uniquely identifying the build for the given platform independent of the `<major>`, `<minor>`, and `<patch>` fields. Its value is not a cumulative indication of which features or fixes are included in which build.

<table>
  <tbody>
    <tr><th>Requirement</th><th>Text</th></tr>
    <tr>
      <td>VER_010
      <td>Each Connector release shall be uniquely identified by a platform identifier and a version identifier.
    </tr>
    <tr>
      <td>VER_020
      <td>The connector version identifier shall consist of 4 numbers separated by periods in the following format: `<Major Version>.<Minor Version>.<Patch Version>.<Build Number>`.
    </tr>
  </tbody>
</table>
