#include "seccomp_filter.h"
#include "pc_cache.h"
#include "memfd_faker.h"
#include <android/log.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>

#define LOG_TAG "SoulBypass"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace soulbypass {

bool SeccompFilter::installed_ = false;
__thread volatile int SignalHandler::in_handler_ = 0;

bool SeccompFilter::install() {
    if (installed_) return true;
    
    if (!SignalHandler::install()) return false;
    
    static struct sock_filter filter[] = {
        BPF_STMT(BPF_LD(BPF_W) | BPF_ABS, offsetof(seccomp_data, arch)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0xc00000b7, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
        BPF_STMT(BPF_LD(BPF_W) | BPF_ABS, offsetof(seccomp_data, nr)),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscalls::NR_openat, 6, 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscalls::NR_readlinkat, 5, 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscalls::NR_ptrace, 4, 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscalls::NR_prctl, 3, 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, syscalls::NR_fork, 2, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRAP),
    };
    
    static struct sock_fprog prog = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };
    
    int ret = syscall(__NR_seccomp, 1, 1, &prog);
    if (ret < 0) {
        LOGD("[Seccomp] install failed: %d", ret);
        return false;
    }
    
    installed_ = true;
    LOGD("[Seccomp] Filter installed");
    return true;
}

bool SeccompFilter::isInstalled() { return installed_; }
sock_fprog* SeccompFilter::buildFilter() { return nullptr; }

bool SignalHandler::install() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sa.sa_sigaction = handle;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGSYS);
    
    if (sigaction(SIGSYS, &sa, nullptr) != 0) return false;
    LOGD("[SignalHandler] Installed");
    return true;
}

void SignalHandler::handle(int sig, siginfo_t* info, void* ctx) {
    (void)sig;
    if (__atomic_exchange_n(&in_handler_, 1, __ATOMIC_SEQ_CST)) return;
    
    ucontext_t* uc = static_cast<ucontext_t*>(ctx);
    uintptr_t pc = getPC(ctx);
    int syscall_nr = info->si_syscall;
    
    uintptr_t args[6] = {
        uc->uc_mcontext.regs[0], uc->uc_mcontext.regs[1],
        uc->uc_mcontext.regs[2], uc->uc_mcontext.regs[3],
        uc->uc_mcontext.regs[4], uc->uc_mcontext.regs[5]
    };
    
    bool shouldIntercept = false;
    uintptr_t fakeResult = 0;
    
    // Step 1: PC in target range
    if (PCCache::getInstance().isTargetPC(pc)) {
        LOGD("[Handler] PC in target: 0x%lx", pc);
        shouldIntercept = true;
    }
    
    // Step 2: Check sensitive path
    if (!shouldIntercept && (syscall_nr == syscalls::NR_openat || 
                              syscall_nr == syscalls::NR_readlinkat)) {
        const char* path = reinterpret_cast<const char*>(args[1]);
        if (path && MemfdFaker::matchSensitivePath(path)) {
            LOGD("[Handler] Sensitive path: %s", path);
            shouldIntercept = true;
        }
    }
    
    if (shouldIntercept) {
        switch (syscall_nr) {
            case syscalls::NR_openat:
                if (handleOpenat(args, &fakeResult)) {
                    uc->uc_mcontext.regs[0] = fakeResult;
                    uc->uc_mcontext.pc += 4;
                }
                break;
            case syscalls::NR_ptrace:
                handlePtrace(args, &fakeResult);
                uc->uc_mcontext.regs[0] = fakeResult;
                uc->uc_mcontext.pc += 4;
                break;
            case syscalls::NR_prctl:
                if (handlePrctl(args, &fakeResult)) {
                    uc->uc_mcontext.regs[0] = fakeResult;
                    uc->uc_mcontext.pc += 4;
                }
                break;
        }
    }
    
    __atomic_store_n(&in_handler_, 0, __ATOMIC_SEQ_CST);
}

uintptr_t SignalHandler::getPC(void* ctx) {
    return static_cast<ucontext_t*>(ctx)->uc_mcontext.pc;
}

int SignalHandler::handleOpenat(uintptr_t* args, uintptr_t* result) {
    const char* pathname = reinterpret_cast<const char*>(args[1]);
    if (!pathname) return 0;
    
    int fd = MemfdFaker::getInstance().getFakeFd(pathname);
    if (fd >= 0) {
        *result = static_cast<uintptr_t>(fd);
        return 1;
    }
    
    if (MemfdFaker::matchSensitivePath(pathname)) {
        *result = static_cast<uintptr_t>(-2);  // ENOENT
        return 1;
    }
    return 0;
}

int SignalHandler::handleReadlinkat(uintptr_t* args, uintptr_t* result) {
    (void)args;
    *result = static_cast<uintptr_t>(-2);
    return 1;
}

int SignalHandler::handlePtrace(uintptr_t* args, uintptr_t* result) {
    (void)args;
    *result = 0;
    return 1;
}

int SignalHandler::handlePrctl(uintptr_t* args, uintptr_t* result) {
    int option = static_cast<int>(args[0]);
    if (option == PR_GET_DUMPABLE || option == PR_GET_SECCOMP) {
        *result = 0;
        return 1;
    }
    return 0;
}

} // namespace soulbypass
