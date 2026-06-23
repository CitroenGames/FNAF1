#pragma once

#include <iosfwd>
#include <memory>
#include <string>

namespace Paingine2D {
    class CrashHandlerPlatform;

    class CrashHandler {
    public:
        static CrashHandler *GetInstance();

        static std::string GetDefaultCrashFolder();

        bool Initialize(const std::string &applicationName, const std::string &crashReportFolder = "");
        void Shutdown();

    private:
        CrashHandler();
        ~CrashHandler();

        std::string GenerateCrashReportPath() const;
        void WriteSystemInfo(std::ofstream &logFile) const;

        std::string m_applicationName;
        std::string m_crashReportFolder;
        std::unique_ptr<CrashHandlerPlatform> m_platform;

        friend class CrashHandlerPlatform;
    };
} // namespace Paingine2D
