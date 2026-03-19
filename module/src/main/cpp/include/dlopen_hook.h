#pragma once

namespace soulbypass {

class DlopenHook {
public:
    static bool install();
    static void onTargetSOLoaded(const char* name, void* handle);
private:
    static bool installed_;
    static constexpr const char* TARGET_LIBS[] = {
        "libIncite.so", "libghost.so", 
        "libumeng-spy.so", "libzxprotect.so",
        nullptr
    };
    static bool isTarget(const char* filename);
};

} // namespace soulbypass
