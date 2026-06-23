#pragma once

#include <fstream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <signal.h>
#endif

namespace Paingine2D {
    class CrashHandler;

    class CrashHandlerPlatform {
    public:
        static std::string GetDefaultCrashFolder();

        bool Install(CrashHandler &handler);
        void Shutdown();

        void WriteSystemInfo(std::ofstream &logFile) const;

    private:
        bool m_installed = false;

#ifdef _WIN32
        LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter = nullptr;

        bool WriteMiniDump(EXCEPTION_POINTERS *exceptionPointers, const std::string &filePath) const;
        void WriteLogFile(const CrashHandler &handler,
                          EXCEPTION_POINTERS *exceptionPointers,
                          const std::string &crashReportPath) const;
        static void WriteWindowsStackTrace(std::ofstream &logFile, EXCEPTION_POINTERS *exceptionPointers);
        static LONG WINAPI WindowsExceptionHandler(EXCEPTION_POINTERS *exceptionPointers);
#elif defined(__APPLE__) || defined(__linux__)
        struct sigaction m_previousSignalActions[NSIG]{};

        void InstallSignalHandlers();
        void RestoreSignalHandlers();
        void WriteStackTrace(std::ofstream &logFile) const;
        void WriteUnixCrashReport(const CrashHandler &handler,
                                  int signalNumber,
                                  siginfo_t *info,
                                  const std::string &crashReportPath) const;
        static void UnixSignalHandler(int signalNumber, siginfo_t *info, void *context);
#endif
    };
} // namespace Paingine2D
