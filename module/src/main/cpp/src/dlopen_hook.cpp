#include "dlopen_hook.h"
#include "pc_cache.h"
#include <android/log.h>
#include <string.h>
#include <link.h>
#include <elf.h>

#define LOG_TAG "SoulBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace soulbypass {

constexpr const char* DlopenHook::TARGET_LIBS[];
bool DlopenHook::installed_ = false;

bool DlopenHook::install() {
    if (installed_) return true;
    // TODO: Implement Dobby-based inline hook
    // For now, rely on dl_iterate_phdr in postAppSpecialize
    LOGD("[DlopenHook] Placeholder - using dl_iterate_phdr");
    installed_ = true;
    return true;
}

bool DlopenHook::isTarget(const char* filename) {
    if (!filename) return false;
    for (size_t i = 0; TARGET_LIBS[i] != nullptr; i++) {
        if (strstr(filename, TARGET_LIBS[i])) return true;
    }
    return false;
}

void DlopenHook::onTargetSOLoaded(const char* name, void* handle) {
    (void)handle;
    LOGD("[DlopenHook] Target loaded: %s", name);
    
    // Use dl_iterate_phdr to find base and text segment
    struct TargetInfo {
        const char* name;
        uintptr_t base;
        uintptr_t text_start;
        uintptr_t text_end;
        bool found;
    } info = {name, 0, 0, 0, false};
    
    dl_iterate_phdr([](struct dl_phdr_info* i, size_t, void* data) {
        auto* target = static_cast<TargetInfo*>(data);
        if (!i->dlpi_name || !strstr(i->dlpi_name, target->name)) return 0;
        
        target->base = i->dlpi_addr;
        target->found = true;
        
        for (int j = 0; j < i->dlpi_phnum; j++) {
            const ElfW(Phdr)* ph = &i->dlpi_phdr[j];
            if (ph->p_type == PT_LOAD && (ph->p_flags & PF_X)) {
                uintptr_t start = i->dlpi_addr + ph->p_vaddr;
                uintptr_t end = start + ph->p_memsz;
                if (!target->text_start || start < target->text_start) 
                    target->text_start = start;
                if (end > target->text_end) target->text_end = end;
            }
        }
        return 1;
    }, &info);
    
    if (info.found) {
        PCCache::getInstance().updateRange(name, info.text_start, info.text_end);
    }
}

} // namespace soulbypass
