#include <zygisk.h>
#include <android/log.h>
#include "pc_cache.h"
#include "seccomp_filter.h"
#include "memfd_faker.h"
#include "dlopen_hook.h"

#define LOG_TAG "SoulBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace soulbypass;

class SoulBypassModule : public zygisk::ModuleBase {
public:
    void onLoad(Api* api, JNIEnv* env) override {
        this->api_ = api;
        this->env_ = env;
        LOGD("[SoulBypass] Module loaded");
    }

    void preAppSpecialize(AppSpecializeArgs* args) override {
        const char* package_name = env_>->GetStringUTFChars(args-> nice_name, nullptr);
        bool isTarget = (strcmp(package_name, "cn.soulapp.android") == 0);
        env_>->ReleaseStringUTFChars(args-> nice_name, package_name);
        
        if (!isTarget) {
            api_>->setOption(Api::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        LOGD("[SoulBypass] Target app detected: cn.soulapp.android");
        enabled_ = true;
    }

    void postAppSpecialize(const AppSpecializeArgs* args) override {
        (void)args;
        if (!enabled_) return;
        
        LOGD("[SoulBypass] Initializing bypass...");
        
        // Initialize components
        if (!MemfdFaker::getInstance().initialize()) {
            LOGD("[SoulBypass] Failed to initialize memfd faker");
        }
        
        if (!DlopenHook::install()) {
            LOGD("[SoulBypass] Failed to install dlopen hook");
        }
        
        if (!SeccompFilter::install()) {
            LOGD("[SoulBypass] Failed to install seccomp filter");
        } else {
            LOGD("[SoulBypass] Seccomp-BPF filter active");
        }
        
        LOGD("[SoulBypass] Initialization complete");
    }

    void preServerSpecialize(ServerSpecializeArgs* args) override {
        (void)args;
        api_>->setOption(Api::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api* api_ = nullptr;
    JNIEnv* env_ = nullptr;
    bool enabled_ = false;
};

REGISTER_ZYGISK_MODULE(SoulBypassModule)
