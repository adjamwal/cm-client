# Brief on checked-in prebuilt test resources for Mac 

The binaries found under /tests/Resources/Mac are intended to be used for the unit tests only. 

The `cmpackagemanager` binary is a dummy light weight process to help test out the PackageManager ModuleControlPlugin and PmAgentController unit tests. The requirement is for a simple process which can be launched by the UTs to test out the process launch implementation.

# Some info on how the checked-in cmpackagemanager test binary was built

The cmpackagemanager test binary is as of now a simple long living process. To rebuild or update the binary please build the test-executable project using the following command :

```
$ cd controlplugin/testexecutable/
$ sh build.sh
```

The frequent update of any of these resources is not recommended.