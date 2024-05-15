-- Define a constant for the log file path
property loggingPath : "/Library/Logs/Cisco/SecureClient/CloudManagement/"
property logFilePath : loggingPath & "cm_uninstall.log"

-- Define a global flag to track errors
property hasError : false

(*
	The zero_pad function taken from:
	http://www.nineboxes.net/2009/10/an-applescript-function-to-zero-pad-integers/
*)
on zero_pad(value, string_length)
	set string_zeroes to ""
	set digits_to_pad to string_length - (length of (value as string))
	if digits_to_pad > 0 then
		repeat digits_to_pad times
			set string_zeroes to string_zeroes & "0" as string
		end repeat
	end if
	set padded_value to string_zeroes & value as string
	return padded_value
end zero_pad

-- Function to log actions with the [MM/DD/YYYY HH:mm] timestamp format
on logAction(actionText)
	set now to (current date)
	set result to (year of now as integer) as string
	set result to result & "-"
	set result to result & zero_pad(month of now as integer, 2)
	set result to result & "-"
	set result to result & zero_pad(day of now as integer, 2)
	set result to result & " "
	set result to result & zero_pad(hours of now as integer, 2)
	set result to result & ":"
	set result to result & zero_pad(minutes of now as integer, 2)
	set result to result & ":"
	set result to result & zero_pad(seconds of now as integer, 2)
	
	set logEntry to "[" & result & "] " & actionText
    try
        -- Ensure the directory structure exists
        do shell script "mkdir -p " & quoted form of loggingPath with administrator privileges
            
        -- Try to create the log file if it doesn't exist
        do shell script "touch " & quoted form of logFilePath with administrator privileges
    
        -- Append the log entry to the file
        do shell script "echo " & quoted form of logEntry & " >> " & logFilePath with administrator privileges
	on error
        display dialog "Failed to log action. Check log file path: " & logFilePath
	end try

end logAction

-- Function to handle errors and display a failure message
on handleError(errorMsg)
	logAction("Error: " & errorMsg)
	set hasError to true
end handleError

-- Define file and directory paths
set CM_DIR to "/opt/cisco/secureclient/cloudmanagement"
set BIN_DIR to (CM_DIR & "/bin")
set CM_BINARY to "csccloudmanagement"
set CMID_BINARY to "csc_cmid"
set PM_BINARY to "cmpackagemanager"
set CMREPORT_BINARY to "cmreport"
set LAUNCHD_DIR to "/Library/LaunchDaemons"
set LAUNCHD_FILE to "com.cisco.secureclient.cloudmanagement.plist"
set CM_PACKAGE_ID to "com.cisco.secureclient.cloudmanagement"
set CM_UNINSTALLER_PKG_ID to "com.cisco.secureclient.cloudmanagement-uninstaller"
set CM_BOOTSTRAP_PKG_ID to "com.cisco.secureclient.cloudmanagement_bootstrap"
set CM_CRASHPAD_DIR to (CM_DIR & "/ch")

-- Uninstall Cisco Secure Client CloudManagement
logAction("Uninstalling Cisco Secure Client CloudManagement ...")

-- Stop the specified launchd service
try
	do shell script "launchctl bootout system " & (quoted form of (LAUNCHD_DIR & "/" & LAUNCHD_FILE)) with administrator privileges
	logAction("Stopped launchd service: " & LAUNCHD_FILE)
on error errMsg
	handleError("Failed to stop launchd service: " & errMsg)
end try

-- Define a list of binaries to check and stop
set binaryList to {CM_BINARY, CMID_BINARY, PM_BINARY, CMREPORT_BINARY}

-- Iterate through the list of binaries and stop them if they are running
repeat with binaryName in binaryList
	set processID to do shell script "ps -A -o pid,command | grep " & (quoted form of ("(" & (BIN_DIR & "/" & binaryName) & ")")) & " | egrep -v 'grep|cm_uninstall' | awk '{print $1}'"
	
	if processID is not equal to "" then
		try
			do shell script "kill -KILL " & processID with administrator privileges
			logAction("Killed process: " & processID)
		on error errMsg
			handleError("Failed to kill process: " & errMsg)
		end try
	end if
end repeat

-- Remove launchd file if it exists
if (do shell script "[ -e " & (quoted form of (LAUNCHD_DIR & "/" & LAUNCHD_FILE)) & " ] ; echo $?") is equal to "0" then
	try
		do shell script "rm -rf " & (quoted form of (LAUNCHD_DIR & "/" & LAUNCHD_FILE)) with administrator privileges
		logAction("Removed launchd file: " & LAUNCHD_FILE)
	on error errMsg
		handleError("Failed to remove launchd file: " & errMsg)
	end try
end if

-- Remove files and directories
try
	do shell script "rm -rf " & (quoted form of CM_DIR) with administrator privileges
	logAction("Removed directory: " & CM_DIR)
on error errMsg
	handleError("Failed to remove directory: " & errMsg)
end try

try
	do shell script "rm -rf " & (quoted form of CM_CRASHPAD_DIR) with administrator privileges
	logAction("Removed directory: " & CM_CRASHPAD_DIR)
on error errMsg
	handleError("Failed to remove directory: " & errMsg)
end try

-- Forget the package using pkgutil
try
    do shell script "pkgutil --forget " & (quoted form of CM_PACKAGE_ID) with administrator privileges
    logAction("Forgot package: " & CM_PACKAGE_ID)
on error errMsg
    logAction("Failed to forget package: " & errMsg)
end try

try
    do shell script "pkgutil --forget " & (quoted form of CM_UNINSTALLER_PKG_ID) with administrator privileges
    logAction("Forgot package: " & CM_UNINSTALLER_PKG_ID)
on error errMsg
    logAction("Failed to forget package: " & errMsg)
end try

try
    do shell script "pkgutil --forget " & (quoted form of CM_BOOTSTRAP_PKG_ID) with administrator privileges
    logAction("Forgot package: " & CM_BOOTSTRAP_PKG_ID)
on error errMsg
    logAction("Failed to forget package: " & errMsg)
end try

-- Display a single failure message if any errors occurred
if hasError then
	display dialog "Uninstall was failed. Check the log for details at " & logFilePath & "."
else
	-- Delete the cm_uninstall.app bundle and its parent folder if it exists
	try
		-- Specify the path to cm_uninstall.app
		set cmUninstallApp to "Uninstall CloudManagement.app"
		set cmUninstallPath to "/Applications/Cisco/Cloud Management/"
		set cmUninstallAppPath to cmUninstallPath & cmUninstallApp
		
		-- Check if cm_uninstall.app exists and delete it
		if (do shell script "[ -e " & quoted form of cmUninstallAppPath & " ] ; echo $?") is equal to "0" then
			do shell script "rm -rf " & quoted form of cmUninstallAppPath with administrator privileges
			logAction("Removed Uninstall_CloudManagement.app: " & cmUninstallAppPath)
			
			-- Remove the parent folder as well
			do shell script "rmdir " & quoted form of cmUninstallPath with administrator privileges
			logAction("Removed parent folder: " & cmUninstallPath)
		end if
	on error errMsg
		handleError("Failed to remove " & cmUninstallApp & " or its parent folder: " & errMsg)
	end try
 
    -- Check if the folder loggingPath exists before attempting to remove it
    if (do shell script "[ -d " & quoted form of loggingPath & " ] ; echo $?") is equal to "0" then
        try
            do shell script "rm -rf " & quoted form of loggingPath with administrator privileges
        on error errMsg
            logAction("Failed to remove folder: " & loggingPath & " - " & errMsg)
        end try
    end if
    	
	-- Display success message
	display dialog "Successfully uninstalled Cisco Secure Client CloudManagement."
end if

-- Exit the script with success status
if hasError then
	return 1
else
	return 0
end if
