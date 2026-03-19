// Zygisk API Header (v4)
// From: https://github.com/topjohnwu/Zygisk

#pragma once

#include <jni.h>

#define ZYGISK_API_VERSION 4

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:
    virtual void onLoad(Api* api, JNIEnv* env) {}
    virtual void preAppSpecialize(AppSpecializeArgs* args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs* args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs* args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs* args) {}
};

struct Api {
    void* impl;
    JNIEnv* env;

    enum {
        DLCLOSE_MODULE_LIBRARY = 0,
    };

    void setOption(int option) {
        // Implementation
    }
};

struct AppSpecializeArgs {
    jint uid;
    jint gid;
    jintArray gids;
    jint runtime_flags;
    jint mount_external;
    jstring se_info;
    jstring nice_name;
    jstring instruction_set;
    jstring app_data_dir;
};

struct ServerSpecializeArgs {
    jint uid;
    jint gid;
    jintArray gids;
    jint runtime_flags;
    jlong permitted_capabilities;
    jlong effective_capabilities;
};

} // namespace zygisk

#define REGISTER_ZYGISK_MODULE(className) \
    extern "C" void zygisk_module_entry(zygisk::Api* api, JNIEnv* env) { \
        static className module; \
        module.onLoad(api, env); \
    }
