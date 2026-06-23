#include "CrashHandler.h"

#include "CrashHandlerPlatform.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

namespace {
    std::filesystem::path NormalizeReportFolder(const std::string &folder) {
        if (folder.empty()) {
            return {};
        }

        return std::filesystem::path(folder);
    }

    std::string BuildTimestamp() {
        const time_t now = time(nullptr);
        tm timeinfo{};
#ifdef _WIN32
        localtime_s(&timeinfo, &now);
#else
        localtime_r(&now, &timeinfo);
#endif

        char timestamp[80]{};
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &timeinfo);
        return timestamp;
    }
}

namespace Paingine2D {
    CrashHandler *CrashHandler::GetInstance() {
        static CrashHandler instance;
        return &instance;
    }

    std::string CrashHandler::GetDefaultCrashFolder() {
        return CrashHandlerPlatform::GetDefaultCrashFolder();
    }

    CrashHandler::CrashHandler()
        : m_platform(std::make_unique<CrashHandlerPlatform>()) {
    }

    CrashHandler::~CrashHandler() {
        Shutdown();
    }

    bool CrashHandler::Initialize(const std::string &applicationName, const std::string &crashReportFolder) {
        Shutdown();

        m_applicationName = applicationName;
        m_crashReportFolder = crashReportFolder.empty() ? GetDefaultCrashFolder() : crashReportFolder;

        const std::filesystem::path reportFolder = NormalizeReportFolder(m_crashReportFolder);
        std::error_code error;
        std::filesystem::create_directories(reportFolder, error);
        if (error) {
            std::cerr << "CrashHandler: failed to create crash report folder '"
                      << reportFolder.string() << "': " << error.message() << std::endl;
        }

        return m_platform->Install(*this) && !error;
    }

    void CrashHandler::Shutdown() {
        if (m_platform) {
            m_platform->Shutdown();
        }
    }

    std::string CrashHandler::GenerateCrashReportPath() const {
        const std::string reportName = m_applicationName + "_" + BuildTimestamp();
        return (std::filesystem::path(m_crashReportFolder) / reportName).string();
    }

    void CrashHandler::WriteSystemInfo(std::ofstream &logFile) const {
        logFile << "======= Crash Report for " << m_applicationName << " =======" << std::endl;
        logFile << "Time: " << time(nullptr) << std::endl;
        m_platform->WriteSystemInfo(logFile);
    }
} // namespace Paingine2D
