#include "pc_cache.h"
#include <string.h>
#include <android/log.h>

#define LOG_TAG "SoulBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace soulbypass {

constexpr const char* PCCache::TARGET_LIBS[];

PCCache& PCCache::getInstance() {
    static PCCache instance;
    return instance;
}

int PCCache::findIndex(const char* name) const {
    if (!name) return -1;
    for (size_t i = 0; TARGET_LIBS[i] != nullptr; i++) {
        if (strstr(name, TARGET_LIBS[i]) != nullptr) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void PCCache::updateRange(const char* libName, uintptr_t start, uintptr_t end) {
    int idx = findIndex(libName);
    if (idx < 0 || idx >= MAX_TARGETS) return;
    
    PCRange& range = ranges_[idx];
    pthread_rwlock_wrlock(&range.lock);
    
    range.start.store(start, std::memory_order_seq_cst);
    range.end.store(end, std::memory_order_seq_cst);
    range.version.fetch_add(1, std::memory_order_seq_cst);
    range.valid.store(true, std::memory_order_seq_cst);
    
    pthread_rwlock_unlock(&range.lock);
    
    LOGD("[PCCache] Updated %s: [0x%lx, 0x%lx] v%d",
         libName, start, end, range.version.load());
}

bool PCCache::isTargetPC(uintptr_t pc) const {
    for (size_t i = 0; i < MAX_TARGETS && TARGET_LIBS[i] != nullptr; i++) {
        const PCRange& range = ranges_[i];
        if (!range.valid.load(std::memory_order_acquire)) continue;
        
        uintptr_t start = range.start.load(std::memory_order_relaxed);
        uintptr_t end = range.end.load(std::memory_order_relaxed);
        
        if (pc >= start && pc <= end) {
            return true;
        }
    }
    return false;
}

uint32_t PCCache::getVersion(const char* libName) const {
    int idx = findIndex(libName);
    if (idx < 0 || idx >= MAX_TARGETS) return 0;
    return ranges_[idx].version.load(std::memory_order_acquire);
}

} // namespace soulbypass
