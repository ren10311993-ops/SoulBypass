#pragma once

#include <stddef.h>
#include <stdint.h>

namespace soulbypass {

#ifndef __NR_memfd_create
#define __NR_memfd_create 279
#endif

enum class MatchType { EXACT, PREFIX, CONTAINS };

struct SensitivePath {
    const char* path;
    MatchType type;
};

struct FakeFile {
    int fd;
    char* data;
    size_t size;
    bool valid;
};

class MemfdFaker {
public:
    static MemfdFaker& getInstance();
    bool initialize();
    int getFakeFd(const char* path);
    bool refresh();
    static bool shouldFake(const char* path);
    static bool matchSensitivePath(const char* path);

private:
    MemfdFaker() = default;
    FakeFile fake_maps_, fake_status_, fake_cmdline_;
    FakeFile fake_mounts_, fake_mountinfo_, fake_wchan_;
    
    bool generateFakeMaps();
    bool generateFakeStatus();
    bool generateFakeCmdline();
    bool generateFakeMounts();
    bool generateFakeMountinfo();
    bool generateFakeWchan();
    int createMemfd(const char* name, const char* data, size_t size);
    char* readAndFilter(const char* path, size_t* outSize);
    char* filterSensitiveLines(char* data, size_t size, size_t* outSize);
    
    static constexpr SensitivePath SENSITIVE_PATHS[] = {
        {"/proc/self/maps", MatchType::EXACT},
        {"/proc/self/status", MatchType::EXACT},
        {"/proc/self/cmdline", MatchType::EXACT},
        {"/proc/thread-self/maps", MatchType::EXACT},
        {"/proc/thread-self/status", MatchType::EXACT},
        {"/proc/self/task/", MatchType::PREFIX},
        {"/proc/thread-self/task/", MatchType::PREFIX},
        {"/proc/self/fd/", MatchType::PREFIX},
        {"/proc/self/mounts", MatchType::EXACT},
        {"/proc/self/mountinfo", MatchType::EXACT},
        {"/proc/self/wchan", MatchType::EXACT},
        {"/data/local/tmp/", MatchType::PREFIX},
        {"/system/bin/su", MatchType::EXACT},
        {"/system/xbin/su", MatchType::EXACT},
        {"/vendor/bin/su", MatchType::EXACT},
        {"/sbin/su", MatchType::EXACT},
        {"/su/bin/su", MatchType::EXACT},
        {"/magisk", MatchType::PREFIX},
        {"/.magisk", MatchType::PREFIX},
        {nullptr, MatchType::EXACT}
    };
    
    static constexpr const char* SENSITIVE_KEYWORDS[] = {
        "zygisk", "magisk", "lsposed", "xposed", "edxp", "riru",
        "incite", "ghost", "frida", "gmain", "ptrace_stop", nullptr
    };
};

} // namespace soulbypass
