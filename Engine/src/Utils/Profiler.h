#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Define PROFILING=0 in the build to compile every profiling macro out entirely.
#ifndef PROFILING
#define PROFILING 1
#endif

struct ProfileResult {
    std::string Name;
    long long Start, End;
    uint32_t ThreadID;
};

struct InstrumentationSession {
    std::string Name;
};

class Instrumentor {
private:
    void EndSessionUnlocked();

    std::unique_ptr<InstrumentationSession> m_CurrentSession;
    std::ofstream m_OutputStream;
    int m_ProfileCount;
    std::mutex m_Mutex;
    bool m_Active;

public:
    Instrumentor();
    ~Instrumentor();

    void BeginSession(const std::string &name, const std::string &filepath = "results.json");
    void EndSession();
    void WriteProfile(const ProfileResult &result);
    void WriteHeader();
    void WriteFooter();

    static Instrumentor &Get();
};

class InstrumentationTimer {
public:
    InstrumentationTimer(const char *name);
    ~InstrumentationTimer();

    void Stop();

private:
    const char *m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    bool m_Stopped;
};

// Backs PROFILE_BEGIN / PROFILE_END. Scopes nest: Begin pushes and End pops,
// so an inner region no longer terminates the region that encloses it.
class ManualInstrumentationScope {
private:
    static std::vector<std::unique_ptr<InstrumentationTimer> > s_TimerStack;

public:
    static void Begin(const char *name);
    static void End();
    static void EndAll();
};

#if PROFILING
#define PROFILE_CONCAT_INTERNAL(x, y) x##y
#define PROFILE_CONCAT(x, y) PROFILE_CONCAT_INTERNAL(x, y)

#define PROFILE_BEGIN_SESSION(name, filepath) Instrumentor::Get().BeginSession(name, filepath)
#define PROFILE_END_SESSION() Instrumentor::Get().EndSession()
#define PROFILE_SCOPE(name) InstrumentationTimer PROFILE_CONCAT(timer, __LINE__)(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)

#if defined(__GNUC__) || defined(__clang__)
#define PROFILE_FUNCTION_SIG() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
#define PROFILE_FUNCTION_SIG() PROFILE_SCOPE(__FUNCSIG__)
#else
#define PROFILE_FUNCTION_SIG() PROFILE_FUNCTION()
#endif

#define PROFILE_BEGIN(name) ManualInstrumentationScope::Begin(name)
#define PROFILE_END() ManualInstrumentationScope::End()

#define PROFILE_AUTO_SESSION(name, filepath) \
    class ProfilerSessionGuard { \
    public: \
        ProfilerSessionGuard(const std::string& name, const std::string& filepath) { \
            Instrumentor::Get().BeginSession(name, filepath); \
        } \
        ~ProfilerSessionGuard() { \
            Instrumentor::Get().EndSession(); \
        } \
    }; \
    static ProfilerSessionGuard PROFILE_CONCAT(autoProfilerGuard, __LINE__)(name, filepath)

#else
#define PROFILE_BEGIN_SESSION(name, filepath)
#define PROFILE_END_SESSION()
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#define PROFILE_FUNCTION_SIG()
#define PROFILE_BEGIN(name)
#define PROFILE_END()
#define PROFILE_AUTO_SESSION(name, filepath)
#endif
