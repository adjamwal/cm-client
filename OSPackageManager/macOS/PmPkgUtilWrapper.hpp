#include "IPmPkgUtil.hpp"

/**
 * @brief A class that wraps the 'pkgutil' utility to perform package-related operations.
 * Implements the IPmPkgUtil interface.
 */
class PmPkgUtilWrapper : public IPmPkgUtil {
public:
    /**
     * @brief Lists the packages installed on the specified volume or the default volume.
     * @param volumePath The path to the volume (optional).
     * @return A vector of package identifiers.
     */
    std::vector<std::string> listPackages(const std::string& volumePath = std::string()) const override;
    
    /**
     * @brief Retrieves information about a specific package.
     * @param packageIdentifier The identifier of the package.
     * @param volumePath The path to the volume where the package is located (optional).
     * @return The package information.
     */
    PmPackageInfo getPackageInfo(const std::string& packageIdentifier, const std::string& volumePath = std::string()) const override;
    
    /**
     * @brief Lists the files contained within a specific package.
     * @param packageIdentifier The identifier of the package.
     * @param volumePath The path to the volume where the package is located (optional).
     * @return A vector of file paths.
     */
    std::vector<std::string> listPackageFiles(const std::string& packageIdentifier, const std::string& volumePath = std::string()) const override;
    
private:
    /**
     * @brief Executes a command and returns the output as a string.
     * @param command The command to execute.
     * @param volumePath The path to the volume (optional).
     * @return The output of the command.
     */
    virtual std::string executeCommand(const std::string& command, const std::string& volumePath) const;
    
    /**
     * @brief Parses the package information from the output of the 'pkgutil --pkg-info' command.
     * @param output The output of the command.
     * @return The parsed package information.
     */
    PmPackageInfo parsePackageInfo(const std::string& output) const;
};
