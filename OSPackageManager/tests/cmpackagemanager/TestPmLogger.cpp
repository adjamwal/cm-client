#include <gtest/gtest.h>
#include "PmLogger.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <system_error>
#include <thread>
#include <clocale>

namespace
{
    constexpr std::string_view kLogFile = "CDD95502_343A_4A54_B52E_7C9F0F4E5BA2_log_file.log";
}

class TestPmLogger: public ::testing::Test
{
public:
    TestPmLogger():
        fileName_(kLogFile)
    {
        filePath_ = std::filesystem::temp_directory_path() / fileName_;
        std::setlocale(LC_CTYPE, "UTF-8");
    }
protected:
    void clearFile()
    {
        if (std::filesystem::exists(filePath_))
        {
            std::error_code errCode;
            bool bRet = std::filesystem::remove(filePath_, errCode);
            ASSERT_TRUE(bRet);
        }
    }
    
    // This function runs before each test case
    void SetUp() override
    {
        clearFile();
        pLogger_ = std::make_unique<PmLogger>();
        pLogger_->initFileLogging(filePath_.parent_path().native(),
            filePath_.filename().native(), 1048576 * 15, 5);
    }

    // This function runs after each test case
    void TearDown() override
    {
        pLogger_.reset();
        clearFile();
    }
    
    void checkLogRecordsExists(const std::vector<std::string>& records,
                               const std::function<void(const std::string&)>& predicate = {})
    {
        std::ifstream inputFile;
        inputFile.open(filePath_.native());
        ASSERT_TRUE(inputFile.is_open());
        
        std::string line;
        size_t i = 0;
        while (std::getline(inputFile, line))
        {
            ASSERT_TRUE(i < records.size());
            EXPECT_TRUE(line.find(records[i]) != std::string::npos);
            
            if (predicate) {
                predicate(line); // Call the predicate functor on each processed line
            }
            
            ++i;
        }
        EXPECT_TRUE(i == records.size());
        
        inputFile.close();
    }
    
    std::unique_ptr<PmLogger> pLogger_;
    std::string fileName_;
    std::filesystem::path filePath_;
};

class TestPmLoggerMacros : public TestPmLogger {
public:
    void SetUp() override
    {
        clearFile();
        PmLogger::initLogger();
        auto &logger = PmLogger::getLogger();
        logger.initFileLogging(filePath_.parent_path().native(),
                                  filePath_.filename().native(), 1048576 * 15, 5);
    }
    
    void TearDown() override
    {
        PmLogger::releaseLogger();
        clearFile();
    }
};

TEST_F(TestPmLoggerMacros, ShrinkCallingFilePath)
{
    PM_LOG_DEBUG("test");
    checkLogRecordsExists({"test"}, [](const std::string& line) {
        const std::string filenameMarker(__FILENAME__);
        size_t pos = line.find(filenameMarker);
        ASSERT_TRUE(pos != std::string::npos);
        
        // Extract the part of the line before and after the filenameMarker
        std::string partBefore = line.substr(0, pos);
        std::string partAfter = line.substr(pos + filenameMarker.length());
        
        // Check that both parts of the line do not contain any path separator characters
        const char pathSeparator = '/'; // You can modify this if needed for a different platform
        ASSERT_FALSE(partBefore.find(pathSeparator) != std::string::npos || partAfter.find(pathSeparator) != std::string::npos);
    });
}

TEST_F(TestPmLogger, LogLevelChangeRecordExists)
{
    pLogger_->SetLogLevel(IPMLogger::LOG_INFO);
    std::vector<std::string> logRecords = {
        "Set new log level: Info"
    };
    checkLogRecordsExists(logRecords);
}

TEST_F(TestPmLogger, TestVariousMessages)
{
    std::vector<std::string> logRecords = {
        "Function returned following error code: 5, that means: \"No such file\"",
        "Some people take fly agaric",
        "Failed to fetch checking url 8"
    };
    
    std::vector<std::string> logRecordLevels = {
        "[Info]",
        "[Debug]",
        "[Error]"
    };
    
    pLogger_->Log(IPMLogger::LOG_INFO, "Function returned following error code: %d, that means: \"%s\"", 5, "No such file");
    pLogger_->Log(IPMLogger::LOG_DEBUG, "Some people take fly agaric");
    pLogger_->Log(IPMLogger::LOG_ERROR, "Failed to fetch checking url %d", 8);
    
    checkLogRecordsExists(logRecords);
    checkLogRecordsExists(logRecordLevels);
}

TEST_F(TestPmLogger, NoCrashOnMultipleThreads)
{
    const std::string strTest = "This is a test";
    constexpr int kLogOutputsCount = 5;
    constexpr int kThreadsCount = 5;
    std::vector<std::string> logRecords(kThreadsCount * kLogOutputsCount, strTest);
    auto threadFn = [this, &strTest]() {
        for (int i = 0; i < kLogOutputsCount; ++i)
        {
            pLogger_->Log(IPMLogger::LOG_ERROR, strTest.c_str());
        }
    };
    
    std::vector<std::shared_ptr<std::thread>> vThreads;
    vThreads.reserve(kThreadsCount);
    for (int i = 0; i < kThreadsCount; ++i)
    {
        vThreads.push_back(std::make_shared<std::thread>(threadFn));
    }
    
    for (auto&& pThread: vThreads)
    {
        pThread->join();
    }
    
    checkLogRecordsExists(logRecords);
    
}

TEST_F(TestPmLogger, LogLevelCheckMessageFiltering)
{
    pLogger_->SetLogLevel(IPMLogger::LOG_WARNING);
    std::vector<std::string> logRecords = {
        "Set new log level: Warning"
    };
    const std::string strTest = "This is log message of the level: ";
    for (int i = IPMLogger::LOG_ALERT; i != IPMLogger::LOG_WARNING + 1; ++i)
    {
        std::string strMessage = strTest + std::to_string(i);
        logRecords.push_back(strMessage);
    }
    
    auto fnLogAllLevelMessages = [this, &strTest]() {
        for (int i = IPMLogger::LOG_ALERT; i != IPMLogger::LOG_DEBUG + 1; ++i)
        {
            std::string strMessage = strTest + std::to_string(i);
            pLogger_->Log(static_cast<IPMLogger::Severity>(i), strMessage.c_str());
        }
    };
    fnLogAllLevelMessages();
    
    pLogger_->SetLogLevel(IPMLogger::LOG_CRITICAL);
    logRecords.push_back("Set new log level: Critical");
    for (int i = IPMLogger::LOG_ALERT; i != IPMLogger::LOG_CRITICAL + 1; ++i)
    {
        std::string strMessage = strTest + std::to_string(i);
        logRecords.push_back(strMessage);
    }
    fnLogAllLevelMessages();
    
    checkLogRecordsExists(logRecords);
}

//On different platforms error name could be different, so
//made this test only for the Mac platform.
#ifdef __APPLE__
TEST_F(TestPmLogger, TestPassingInvalidWideCharStringToLogger)
{
    std::vector<std::string> logRecords = {
        "Unable to determine buffer size in PmLogger::writeLog, system error: Illegal byte sequence",
        "error writing to the temp file."
    };
    std::wstring invalidString = L"\xD800"; // Invalid UTF-16 code point
    pLogger_->Log(IPMLogger::LOG_DEBUG, invalidString.c_str());
}
#endif


class TestPmLoggerWideChars : public ::testing::TestWithParam<std::pair<std::wstring, std::string>>
{
public:
    TestPmLoggerWideChars() :
        fileName_(kLogFile)
    {
        filePath_ = std::filesystem::temp_directory_path() / fileName_;
        std::setlocale(LC_CTYPE, "UTF-8");
    }
protected:
    void clearFile()
    {
        if (std::filesystem::exists(filePath_))
        {
            std::error_code errCode;
            bool bRet = std::filesystem::remove(filePath_, errCode);
            ASSERT_TRUE(bRet);
        }
    }

    // This function runs before each test case
    void SetUp() override
    {
        clearFile();
        pLogger_ = std::make_unique<PmLogger>();
        pLogger_->initFileLogging(filePath_.parent_path().native(),
            filePath_.filename().native(), 1048576 * 15, 5);
    }

    // This function runs after each test case
    void TearDown() override
    {
        clearFile();
        pLogger_.reset();
    }

    void checkLogRecordExists(const std::string& record)
    {
        std::ifstream inputFile;
        inputFile.open(filePath_.native());
        ASSERT_TRUE(inputFile.is_open());

        std::string line;
        size_t i = 0;
        while (std::getline(inputFile, line))
        {
            ASSERT_TRUE(i < 1);
            EXPECT_TRUE(line.find(record) != std::string::npos);
            ++i;
        }
        EXPECT_TRUE(i == 1);

        inputFile.close();
    }

    std::unique_ptr<PmLogger> pLogger_;
    std::string fileName_;
    std::filesystem::path filePath_;
};


TEST_P(TestPmLoggerWideChars, CheckCorrectConverionToNarrowStrings)
{
    std::pair<std::wstring, std::string> logRecord = GetParam();

    pLogger_->Log(IPMLogger::LOG_INFO, logRecord.first.c_str());

    checkLogRecordExists(logRecord.second.c_str());
}

//"此消息为中文。",
//"Це повідомлення українською мовою.",
//"このメッセージは日本語です。",
//"Այս հաղորդագրությունը հայերեն է։",
//"This message is in English."
INSTANTIATE_TEST_SUITE_P(Default, TestPmLoggerWideChars, ::testing::Values(
    std::make_pair<std::wstring, std::string>(
        L"\U00006B64\U00006D88\U0000606F\U00004E3A\U00004E2D\U00006587\U00003002",
        "\xE6\xAD\xA4\xE6\xB6\x88\xE6\x81\xAF\xE4\xB8\xBA\xE4\xB8\xAD\xE6\x96\x87\xE3\x80\x82"
    ),
   std::make_pair<std::wstring, std::string>(
        L"\U00000426\U00000435\U00000020\U0000043f\U0000043e\U00000432\U00000456"
        "\U00000434\U0000043e\U0000043c\U0000043b\U00000435\U0000043d\U0000043d\U0000044f\U00000020"
        "\U00000443\U0000043a\U00000440\U00000430\U00000457\U0000043d\U00000441\U0000044c\U0000043a"
        "\U0000043e\U0000044e\U00000020\U0000043c\U0000043e\U00000432\U0000043e\U0000044e\U0000002e",
        "\xD0\xA6\xD0\xB5\x20\xD0\xBF\xD0\xBE\xD0\xB2\xD1\x96\xD0\xB4\xD0\xBE\xD0\xBC\xD0\xBB"
        "\xD0\xB5\xD0\xBD\xD0\xBD\xD1\x8F\x20\xD1\x83\xD0\xBA\xD1\x80\xD0\xB0\xD1\x97\xD0\xBD"
        "\xD1\x81\xD1\x8C\xD0\xBA\xD0\xBE\xD1\x8E\x20\xD0\xBC\xD0\xBE\xD0\xB2\xD0\xBE\xD1\x8E\x2E"
   ),
    std::make_pair<std::wstring, std::string>(
        L"\U00003053\U0000306e\U000030e1\U000030c3\U000030bb\U000030fc"
        "\U000030b8\U0000306f\U000065e5\U0000672c\U00008a9e\U00003067\U00003059\U00003002",
        "\xE3\x81\x93\xE3\x81\xAE\xE3\x83\xA1\xE3\x83\x83\xE3\x82\xBB\xE3\x83\xBC\xE3\x82\xB8\xE3\x81"
        "\xAF\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E\xE3\x81\xA7\xE3\x81\x99\xE3\x80\x82"
    ),
    std::make_pair<std::wstring, std::string>(
        L"\U00000531\U00000575\U0000057d\U00000020\U00000570\U00000561\U00000572"
        "\U00000578\U00000580\U00000564\U00000561\U00000563\U00000580\U00000578\U00000582\U00000569"
        "\U00000575\U00000578\U00000582\U00000576\U00000568\U00000020\U00000570\U00000561\U00000575"
        "\U00000565\U00000580\U00000565\U00000576\U00000020\U00000567\U00000589",
        "\xD4\xB1\xD5\xB5\xD5\xBD\x20\xD5\xB0\xD5\xA1\xD5\xB2\xD5\xB8\xD6\x80\xD5\xA4\xD5\xA1\xD5"
        "\xA3\xD6\x80\xD5\xB8\xD6\x82\xD5\xA9\xD5\xB5\xD5\xB8\xD6\x82\xD5\xB6\xD5\xA8\x20\xD5"
        "\xB0\xD5\xA1\xD5\xB5\xD5\xA5\xD6\x80\xD5\xA5\xD5\xB6\x20\xD5\xA7\xD6\x89"
    ),
    std::make_pair<std::wstring, std::string>(
        L"\U00000054\U00000068\U00000069\U00000073\U00000020\U0000006d\U00000065"
        "\U00000073\U00000073\U00000061\U00000067\U00000065\U00000020\U00000069\U00000073\U00000020"
        "\U00000069\U0000006e\U00000020\U00000045\U0000006e\U00000067\U0000006c\U00000069\U00000073"
        "\U00000068\U0000002e",
        "\x54\x68\x69\x73\x20\x6D\x65\x73\x73\x61\x67\x65\x20\x69\x73\x20\x69\x6E\x20"
        "\x45\x6E\x67\x6C\x69\x73\x68\x2E"
   )
));

