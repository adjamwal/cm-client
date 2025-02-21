# Define the paths and filenames
set(appName "Uninstall CloudManagement")
set(applescriptFile "cm_uninstall.applescript")
set(appIcon "cm_uninstall.icns")

# Define the output path
set(output_path "${CM_THIRDPARTY_EXPORT}/bin")

set(component_name "CreateOutputDirectory")

# Check if the output directory exists; if not, create it
add_custom_target(${component_name}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${output_path}"
)

# Compile the AppleScript into an application bundle
add_custom_command(TARGET ${component_name}
    COMMAND osacompile -o "${output_path}/${appName}.app" "${CMAKE_CURRENT_SOURCE_DIR}/${applescriptFile}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Compiling AppleScript"
)

# Set the app's icon using plutil
add_custom_command(TARGET ${component_name}
    COMMAND plutil -replace CFBundleIconFile -string "${appIcon}" "${output_path}/${appName}.app/Contents/Info.plist"
    COMMENT "Setting App Icon"
)

# Copy the icon file to the app's Resources directory
add_custom_command(TARGET ${component_name}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${output_path}/${appName}.app/Contents/Resources"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${appIcon}" "${output_path}/${appName}.app/Contents/Resources/"
    COMMENT "Copying Icon File"
)

# Display a success message
add_custom_command(TARGET ${component_name}
    COMMAND ${CMAKE_COMMAND} -E echo "Application bundle created: ${output_path}/${appName}.app"
    COMMENT "Creating Application Bundle"
)

# Create an alias target for convenience
add_custom_target(CreateUninstallerAppBundle
    ALL
    DEPENDS ${component_name}
)
