#include "CrashHandlerPlatform.h"

#include "CrashHandler.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <DbgHelp.h>
#include <ShlObj.h>

#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "shell32.lib")
#elif defined(__APPLE__)
#include <execinfo.h>
#include <mach/mach.h>
#include <pwd.h>
#include <sys/sysctl.h>
#include <unistd.h>
#elif defined(__linux__)
#include <execinfo.h>
#include <pwd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace {
#ifdef _WIN32
    constexpr int MaxWindowsStackFrames = 20;
#endif

#if defined(__linux__)
    std::string StripShellQuotes(std::string value) {
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            return value.substr(1, value.size() - 2);
        }

        return value;
    }
#endif

#if defined(__APPLE__) || defined(__linux__)
    std::string HomeDirectory() {
        const char *homeDirectory = getenv("HOME");
        if (homeDirectory != nullptr && homeDirectory[0] != '\0') {
            return homeDirectory;
        }

        const passwd *user = getpwuid(getuid());
        if (user != nullptr && user->pw_dir != nullptr) {
            return user->pw_dir;
        }

        return ".";
    }
#endif
}

namespace Paingine2D {
    std::string CrashHandlerPlatform::GetDefaultCrashFolder() {
#ifdef _WIN32
        char documentsPath[MAX_PATH]{};
        const HRESULT result = SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, documentsPath);
        if (SUCCEEDED(result)) {
            return std::string(documentsPath) + "\\Paingine\\Crashes";
        }

        return ".\\Crashes";
#elif defined(__APPLE__)
        return HomeDirectory() + "/Documents/Paingine/Crashes";
#elif defined(__linux__)
        return HomeDirectory() + "/.paingine/crashes";
#else
        return "./Crashes";
#endif
    }

    bool CrashHandlerPlatform::Install(CrashHandler &handler) {
        (void) handler;

        if (m_installed) {
            Shutdown();
        }

#ifdef _WIN32
        m_previousFilter = SetUnhandledExceptionFilter(WindowsExceptionHandler);
        m_installed = true;
#elif defined(__APPLE__) || defined(__linux__)
        InstallSignalHandlers();
        m_installed = true;
#else
        m_installed = true;
#endif

        return true;
    }

    void CrashHandlerPlatform::Shutdown() {
        if (!m_installed) {
            return;
        }

#ifdef _WIN32
        SetUnhandledExceptionFilter(m_previousFilter);
        m_previousFilter = nullptr;
#elif defined(__APPLE__) || defined(__linux__)
        RestoreSignalHandlers();
#endif

        m_installed = false;
    }

    void CrashHandlerPlatform::WriteSystemInfo(std::ofstream &logFile) const {
#ifdef _WIN32
        SYSTEM_INFO sysInfo{};
        GetSystemInfo(&sysInfo);

        logFile << "Platform: Windows" << std::endl;
        logFile << "Processor count: " << sysInfo.dwNumberOfProcessors << std::endl;

        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            logFile << "Memory load: " << memInfo.dwMemoryLoad << "%" << std::endl;
            logFile << "Total physical memory: " << (memInfo.ullTotalPhys / (1024 * 1024)) << " MB" << std::endl;
            logFile << "Available physical memory: " << (memInfo.ullAvailPhys / (1024 * 1024)) << " MB" << std::endl;
        }

        OSVERSIONINFOEXA osvi{};
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
        if (GetVersionExA(reinterpret_cast<OSVERSIONINFOA *>(&osvi))) {
            logFile << "OS Version: " << osvi.dwMajorVersion << "."
                    << osvi.dwMinorVersion << " Build " << osvi.dwBuildNumber << std::endl;
        }
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#elif defined(__APPLE__)
        logFile << "Platform: macOS" << std::endl;

        int mib[2]{CTL_HW, HW_NCPU};
        int ncpu = 0;
        size_t len = sizeof(ncpu);
        if (sysctl(mib, 2, &ncpu, &len, nullptr, 0) != -1) {
            logFile << "Processor count: " << ncpu << std::endl;
        }

        mach_port_t hostPort = mach_host_self();
        vm_size_t pageSize = 0;
        host_page_size(hostPort, &pageSize);

        vm_statistics_data_t vmStats{};
        mach_msg_type_number_t count = sizeof(vmStats) / sizeof(natural_t);
        if (host_statistics(hostPort, HOST_VM_INFO, reinterpret_cast<host_info_t>(&vmStats), &count) == KERN_SUCCESS) {
            const uint64_t freeMemory = static_cast<uint64_t>(vmStats.free_count) * pageSize;
            const uint64_t usedMemory = (
                static_cast<uint64_t>(vmStats.active_count) +
                static_cast<uint64_t>(vmStats.inactive_count) +
                static_cast<uint64_t>(vmStats.wire_count)
            ) * pageSize;
            logFile << "Free memory: " << (freeMemory / (1024 * 1024)) << " MB" << std::endl;
            logFile << "Used memory: " << (usedMemory / (1024 * 1024)) << " MB" << std::endl;
        }

        char osRelease[256]{};
        size_t osReleaseSize = sizeof(osRelease);
        if (sysctlbyname("kern.osrelease", osRelease, &osReleaseSize, nullptr, 0) == 0) {
            logFile << "OS Release: " << osRelease << std::endl;
        }
#elif defined(__linux__)
        logFile << "Platform: Linux" << std::endl;
        logFile << "Processor count: " << sysconf(_SC_NPROCESSORS_ONLN) << std::endl;

        sysinfo memInfo{};
        if (sysinfo(&memInfo) == 0) {
            logFile << "Total memory: " << (memInfo.totalram * memInfo.mem_unit / (1024 * 1024)) << " MB" << std::endl;
            logFile << "Free memory: " << (memInfo.freeram * memInfo.mem_unit / (1024 * 1024)) << " MB" << std::endl;
            logFile << "Uptime: " << (memInfo.uptime / 3600) << " hours" << std::endl;
        }

        std::ifstream osReleaseFile("/etc/os-release");
        std::string line;
        while (std::getline(osReleaseFile, line)) {
            constexpr const char *prettyNamePrefix = "PRETTY_NAME=";
            if (line.rfind(prettyNamePrefix, 0) == 0) {
                logFile << "Distribution: "
                        << StripShellQuotes(line.substr(std::strlen(prettyNamePrefix)))
                        << std::endl;
                break;
            }
        }
#else
        logFile << "Platform: Unknown" << std::endl;
#endif
    }

#ifdef _WIN32
    bool CrashHandlerPlatform::WriteMiniDump(EXCEPTION_POINTERS *exceptionPointers, const std::string &filePath) const {
        HANDLE file = CreateFileA(
            (filePath + ".dmp").c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (file == INVALID_HANDLE_VALUE) {
            return false;
        }

        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{};
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = exceptionPointers;
        exceptionInfo.ClientPointers = FALSE;

        const BOOL result = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            file,
            MiniDumpWithFullMemory,
            &exceptionInfo,
            nullptr,
            nullptr
        );

        CloseHandle(file);
        return result == TRUE;
    }

    void CrashHandlerPlatform::WriteLogFile(const CrashHandler &handler,
                                            EXCEPTION_POINTERS *exceptionPointers,
                                            const std::string &crashReportPath) const {
        std::ofstream logFile(crashReportPath + ".log");
        if (!logFile.is_open()) {
            return;
        }

        handler.WriteSystemInfo(logFile);

        if (exceptionPointers == nullptr) {
            return;
        }

        EXCEPTION_RECORD *record = exceptionPointers->ExceptionRecord;
        logFile << "Exception code: 0x" << std::hex << record->ExceptionCode << std::dec << std::endl;
        logFile << "Exception address: 0x" << std::hex << record->ExceptionAddress << std::dec << std::endl;
        WriteWindowsStackTrace(logFile, exceptionPointers);
    }

    void CrashHandlerPlatform::WriteWindowsStackTrace(std::ofstream &logFile, EXCEPTION_POINTERS *exceptionPointers) {
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();

        if (!SymInitialize(process, nullptr, TRUE)) {
            logFile << "SymInitialize failed with error: " << GetLastError() << std::endl;
            return;
        }

        CONTEXT context = *exceptionPointers->ContextRecord;
        STACKFRAME64 stackFrame{};

#if defined(_M_IX86)
        DWORD machineType = IMAGE_FILE_MACHINE_I386;
        stackFrame.AddrPC.Offset = context.Eip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Ebp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Esp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_X64)
        DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
        stackFrame.AddrPC.Offset = context.Rip;
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Offset = context.Rsp;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Offset = context.Rsp;
        stackFrame.AddrStack.Mode = AddrModeFlat;
#else
        DWORD machineType = 0;
#endif

        logFile << "Stack trace:" << std::endl;
        for (int frame = 0; frame < MaxWindowsStackFrames; ++frame) {
            const BOOL result = StackWalk64(
                machineType,
                process,
                thread,
                &stackFrame,
                &context,
                nullptr,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                nullptr
            );
            if (!result || stackFrame.AddrPC.Offset == 0) {
                break;
            }

            const DWORD64 address = stackFrame.AddrPC.Offset;
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)]{};
            PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            if (SymFromAddr(process, address, nullptr, symbol)) {
                logFile << "  " << frame << ": " << symbol->Name;

                IMAGEHLP_LINE64 line{};
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD displacement = 0;
                if (SymGetLineFromAddr64(process, address, &displacement, &line)) {
                    logFile << " (" << line.FileName << ":" << line.LineNumber << ")";
                }
                logFile << " - 0x" << std::hex << symbol->Address << std::dec << std::endl;
            } else {
                logFile << "  " << frame << ": ??? - 0x" << std::hex << address << std::dec << std::endl;
            }
        }

        SymCleanup(process);
    }

    LONG WINAPI CrashHandlerPlatform::WindowsExceptionHandler(EXCEPTION_POINTERS *exceptionPointers) {
        CrashHandler *handler = CrashHandler::GetInstance();
        const std::string crashReportPath = handler->GenerateCrashReportPath();

        const bool dumpWritten = handler->m_platform->WriteMiniDump(exceptionPointers, crashReportPath);
        if (dumpWritten) {
            handler->m_platform->WriteLogFile(*handler, exceptionPointers, crashReportPath);
        }

        // Deliberately bypasses Utils/Log here: this runs from an exception filter,
        // where taking the logger's mutex could deadlock against the faulting thread.
        std::cerr << "The application has crashed. A crash report has been saved to: "
                  << crashReportPath << ".dmp" << std::endl;

        if (handler->m_platform->m_previousFilter != nullptr) {
            return handler->m_platform->m_previousFilter(exceptionPointers);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }
#elif defined(__APPLE__) || defined(__linux__)
    void CrashHandlerPlatform::InstallSignalHandlers() {
        struct sigaction action{};
        action.sa_sigaction = UnixSignalHandler;
        action.sa_flags = SA_SIGINFO;

        sigaction(SIGSEGV, &action, &m_previousSignalActions[SIGSEGV]);
        sigaction(SIGABRT, &action, &m_previousSignalActions[SIGABRT]);
        sigaction(SIGFPE, &action, &m_previousSignalActions[SIGFPE]);
        sigaction(SIGILL, &action, &m_previousSignalActions[SIGILL]);
        sigaction(SIGBUS, &action, &m_previousSignalActions[SIGBUS]);
    }

    void CrashHandlerPlatform::RestoreSignalHandlers() {
        sigaction(SIGSEGV, &m_previousSignalActions[SIGSEGV], nullptr);
        sigaction(SIGABRT, &m_previousSignalActions[SIGABRT], nullptr);
        sigaction(SIGFPE, &m_previousSignalActions[SIGFPE], nullptr);
        sigaction(SIGILL, &m_previousSignalActions[SIGILL], nullptr);
        sigaction(SIGBUS, &m_previousSignalActions[SIGBUS], nullptr);
    }

    void CrashHandlerPlatform::WriteStackTrace(std::ofstream &logFile) const {
        constexpr int maxStackFrames = 64;
        void *stack[maxStackFrames]{};

        const int frames = backtrace(stack, maxStackFrames);
        char **symbols = backtrace_symbols(stack, frames);

        logFile << "Stack trace:" << std::endl;

        if (symbols != nullptr) {
            for (int index = 0; index < frames; ++index) {
                logFile << "  " << symbols[index] << std::endl;
            }
            free(symbols);
        } else {
            logFile << "  (Failed to obtain stack trace)" << std::endl;
        }
    }

    void CrashHandlerPlatform::WriteUnixCrashReport(const CrashHandler &handler,
                                                    int signalNumber,
                                                    siginfo_t *info,
                                                    const std::string &crashReportPath) const {
        std::ofstream logFile(crashReportPath + ".log");
        if (!logFile.is_open()) {
            return;
        }

        handler.WriteSystemInfo(logFile);
        logFile << "Signal: " << signalNumber << " (" << strsignal(signalNumber) << ")" << std::endl;
        if (info != nullptr) {
            logFile << "Signal code: " << info->si_code << std::endl;
            logFile << "Fault address: " << info->si_addr << std::endl;
        }

        WriteStackTrace(logFile);
    }

    void CrashHandlerPlatform::UnixSignalHandler(int signalNumber, siginfo_t *info, void *context) {
        (void) context;

        CrashHandler *handler = CrashHandler::GetInstance();
        const std::string crashReportPath = handler->GenerateCrashReportPath();
        handler->m_platform->WriteUnixCrashReport(*handler, signalNumber, info, crashReportPath);

        // Deliberately bypasses Utils/Log here: this runs from a signal handler,
        // where taking the logger's mutex is not async-signal-safe.
        std::cerr << "The application has crashed. A crash report has been saved to: "
                  << crashReportPath << ".log" << std::endl;

        ::signal(signalNumber, SIG_DFL);
        raise(signalNumber);
    }
#endif
} // namespace Paingine2D
