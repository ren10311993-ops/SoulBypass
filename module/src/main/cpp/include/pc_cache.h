#pragma once

#include <cstdint>
#include <atomic>
#include <pthread.h>

namespace soulbypass {

// PC range for a single SO library
struct PCRange {
    std::atomic<uintptr_t> start{0};
    std::atomic<uintptr_t> end{0};
    std::atomic<uint32_t> version{0};
    std::atomic<bool> valid{false};
    pthread_rwlock_t lock;
    
    PCRange() { pthread_rwlock_init(&lock, nullptr); }
    ~PCRange() { pthread_rwlock_destroy(&lock); }
    PCRange(const PCRange&) = delete;
    PCRange& operator=(const PCRange&) = delete;
};

// Global cache for target SOs
class PCCache {
public:
    static PCCache& getInstance();
    void updateRange(const char* libName, uintptr_t start, uintptr_t end);
    bool isTargetPC(uintptr_t pc) const;
    uint32_t getVersion(const char* libName) const;

private:
    PCCache() = default;
    static constexpr const char* TARGET_LIBS[] = {
        "libIncite.so",
        "libghost.so",
        nullptr
    };
    static constexpr size_t MAX_TARGETS = 8;
    PCRange ranges_[MAX_TARGETS];
    int findIndex(const char* name) const;
};

} // namespace soulbypass
