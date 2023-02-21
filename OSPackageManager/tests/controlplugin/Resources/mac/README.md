# Brief on checked-in prebuilt test resources for Mac 

The binaries found under /tests/Resources/Mac are intended to be used for the unit tests only. 

The `cmpackagemanager` binary is a dummy light weight process to help test out the PackageManager ModuleControlPlugin and PmAgentController unit tests. The requirement is for a simple process which can be launched by the UTs to test out the process launch implementation.

# Some info on how the checked-in cmpackagemanager test binary was built

The cmpackagemanager test binary is as of now the actual cmpackagemanager binary but without any of its external dependencies (like libcmidapi.dylib). The libcmidapi.dylib dependency of cmpackagemanager was explicitly removed (via cmake changes) before building the binary to avoid rpath resolution issues like below during the unit test execution.

```
dyld: Library not loaded: @rpath/libcmidapi.dylib
Referenced from: /<some-path>/tests/controlplugin/Resources/mac/cmpackagemanager
Reason: image not found

```

Please note that at the time of checking in this binary the cmpackagemanager was still not fully developed making the checked-in binary pretty simple and light weight.
The frequent update of any of these resources is not expected and is not intended.

Notes:
* Execute `$ otool -L cmpackagemanager` to verify if the cmpackagemanager binary has a dependency to libcmidapi.dylib or not.
* `install_name_tool` may be used if required to set appropriate server paths to resolve any @rpath resolution issues.