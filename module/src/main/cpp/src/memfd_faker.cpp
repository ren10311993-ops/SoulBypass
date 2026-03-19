#include "memfd_faker.h"
#include <android/log.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LOG_TAG "SoulBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace soulbypass {

constexpr SensitivePath MemfdFaker::SENSITIVE_PATHS[];
constexpr const char* MemfdFaker::SENSITIVE_KEYWORDS[];

MemfdFaker& MemfdFaker::getInstance() {
    static MemfdFaker instance;
    return instance;
}

bool MemfdFaker::initialize() {
    bool ok = true;
    ok &= generateFakeMaps();
    ok &= generateFakeStatus();
    ok &= generateFakeCmdline();
    ok &= generateFakeMounts();
    ok &= generateFakeMountinfo();
    ok &= generateFakeWchan();
    LOGD("[MemfdFaker] Initialized: %s", ok ? "OK" : "PARTIAL");
    return ok;
}

bool MemfdFaker::shouldFake(const char* path) {
    return path && matchSensitivePath(path);
}

bool MemfdFaker::matchSensitivePath(const char* path) {
    if (!path) return false;
    
    for (size_t i = 0; SENSITIVE_PATHS[i].path != nullptr; i++) {
        const char* pattern = SENSITIVE_PATHS[i].path;
        MatchType type = SENSITIVE_PATHS[i].type;
        
        switch (type) {
            case MatchType::EXACT:
                if (strcmp(path, pattern) == 0) return true;
                break;
            case MatchType::PREFIX:
                if (strncmp(path, pattern, strlen(pattern)) == 0) return true;
                break;
            case MatchType::CONTAINS:
                if (strstr(path, pattern) != nullptr) return true;
                break;
        }
    }
    return false;
}

int MemfdFaker::getFakeFd(const char* path) {
    if (!path) return -1;
    
    if (strstr(path, "maps")) return fake_maps_.valid ? fake_maps_.fd : -1;
    if (strstr(path, "status") && !strstr(path, "task")) 
        return fake_status_.valid ? fake_status_.fd : -1;
    if (strstr(path, "cmdline")) return fake_cmdline_.valid ? fake_cmdline_.fd : -1;
    if (strstr(path, "mounts") && !strstr(path, "mountinfo"))
        return fake_mounts_.valid ? fake_mounts_.fd : -1;
    if (strstr(path, "mountinfo")) return fake_mountinfo_.valid ? fake_mountinfo_.fd : -1;
    if (strstr(path, "wchan")) return fake_wchan_.valid ? fake_wchan_.fd : -1;
    
    return -1;
}

bool MemfdFaker::refresh() {
    if (fake_maps_.valid) close(fake_maps_.fd);
    if (fake_status_.valid) close(fake_status_.fd);
    if (fake_cmdline_.valid) close(fake_cmdline_.fd);
    if (fake_mounts_.valid) close(fake_mounts_.fd);
    if (fake_mountinfo_.valid) close(fake_mountinfo_.fd);
    if (fake_wchan_.valid) close(fake_wchan_.fd);
    
    fake_maps_.valid = fake_status_.valid = fake_cmdline_.valid = false;
    fake_mounts_.valid = fake_mountinfo_.valid = fake_wchan_.valid = false;
    
    return initialize();
}

bool MemfdFaker::generateFakeMaps() {
    const char* minimal = "00000000-ffffffff r-xp 00000000 00:00 0 [anonymous]\n";
    fake_maps_.fd = createMemfd("fake_maps", minimal, strlen(minimal));
    fake_maps_.data = strdup(minimal);
    fake_maps_.size = strlen(minimal);
    fake_maps_.valid = (fake_maps_.fd >= 0);
    return fake_maps_.valid;
}

bool MemfdFaker::generateFakeStatus() {
    const char* data = "Name:\tsoul\nState:\tR (running)\nTracerPid:\t0\n";
    fake_status_.fd = createMemfd("fake_status", data, strlen(data));
    fake_status_.data = strdup(data);
    fake_status_.size = strlen(data);
    fake_status_.valid = (fake_status_.fd >= 0);
    return fake_status_.valid;
}

bool MemfdFaker::generateFakeCmdline() {
    const char* data = "soul";
    fake_cmdline_.fd = createMemfd("fake_cmdline", data, strlen(data));
    fake_cmdline_.data = strdup(data);
    fake_cmdline_.size = strlen(data);
    fake_cmdline_.valid = (fake_cmdline_.fd >= 0);
    return fake_cmdline_.valid;
}

bool MemfdFaker::generateFakeMounts() {
    const char* data = 
        "tmpfs /dev tmpfs rw,seclabel,nosuid,relatime,mode=755 0 0\n"
        "proc /proc proc rw,relatime,hidepid=2 0 0\n"
        "/dev/block/by-name/system /system ext4 ro,seclabel,relatime 0 0\n"
        "/dev/block/by-name/data /data ext4 rw,seclabel,relatime 0 0\n";
    fake_mounts_.fd = createMemfd("fake_mounts", data, strlen(data));
    fake_mounts_.data = strdup(data);
    fake_mounts_.size = strlen(data);
    fake_mounts_.valid = (fake_mounts_.fd >= 0);
    return fake_mounts_.valid;
}

bool MemfdFaker::generateFakeMountinfo() {
    const char* data = "...";
    fake_mountinfo_.fd = createMemfd("fake_mountinfo", data, strlen(data));
    fake_mountinfo_.data = strdup(data);
    fake_mountinfo_.size = strlen(data);
    fake_mountinfo_.valid = (fake_mountinfo_.fd >= 0);
    return fake_mountinfo_.valid;
}

bool MemfdFaker::generateFakeWchan() {
    const char* data = "0\n";
    fake_wchan_.fd = createMemfd("fake_wchan", data, strlen(data));
    fake_wchan_.data = strdup(data);
    fake_wchan_.size = strlen(data);
    fake_wchan_.valid = (fake_wchan_.fd >= 0);
    return fake_wchan_.valid;
}

int MemfdFaker::createMemfd(const char* name, const char* data, size_t size) {
    int fd = syscall(__NR_memfd_create, name, MFD_CLOEXEC);
    if (fd < 0) return -1;
    
    if (write(fd, data, size) != static_cast<ssize_t>(size)) {
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);
    return fd;
}

char* MemfdFaker::readAndFilter(const char* path, size_t* outSize) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return nullptr;
    
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if (size <= 0) { close(fd); return nullptr; }
    
    char* buffer = static_cast<char*>(malloc(size + 1));
    if (!buffer) { close(fd); return nullptr; }
    
    ssize_t readBytes = read(fd, buffer, size);
    close(fd);
    
    if (readBytes < 0) { free(buffer); return nullptr; }
    
    buffer[readBytes] = '\0';
    *outSize = readBytes;
    return buffer;
}

char* MemfdFaker::filterSensitiveLines(char* data, size_t size, size_t* outSize) {
    char* out = static_cast<char*>(malloc(size + 1));
    if (!out) { *outSize = 0; return nullptr; }
    
    char* outPtr = out;
    char* lineStart = data;
    
    while (lineStart < data + size) {
        char* lineEnd = strchr(lineStart, '\n');
        if (!lineEnd) lineEnd = data + size;
        else lineEnd++;
        
        size_t lineLen = lineEnd - lineStart;
        bool sensitive = false;
        
        for (size_t i = 0; SENSITIVE_KEYWORDS[i] != nullptr; i++) {
            if (memmem(lineStart, lineLen, SENSITIVE_KEYWORDS[i], 
                       strlen(SENSITIVE_KEYWORDS[i]))) {
                sensitive = true;
                break;
            }
        }
        
        if (!sensitive) {
            memcpy(outPtr, lineStart, lineLen);
            outPtr += lineLen;
        }
        lineStart = lineEnd;
    }
    
    *outPtr = '\0';
    *outSize = outPtr - out;
    return out;
}

} // namespace soulbypass
