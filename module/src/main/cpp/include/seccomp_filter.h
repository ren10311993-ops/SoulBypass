#pragma once

#include <stdint.h>

namespace soulbypass {

#define SECCOMP_RET_ALLOW 0x7fff0000U
#define SECCOMP_RET_TRAP 0x00030000U
#define SECCOMP_RET_KILL 0x00000000U

struct sock_filter {
    uint16_t code;
    uint8_t jt;
    uint8_t jf;
    uint32_t k;
};

struct sock_fprog {
    uint16_t len;
    struct sock_filter* filter;
};

struct seccomp_data {
    int nr;
    uint32_t arch;
    uint64_t instruction_pointer;
    uint64_t args[6];
};

// BPF instruction macros
#define BPF_LD(mode) ((mode) << 13)
#define BPF_W 0x00
#define BPF_ABS 0x20
#define BPF_JMP 0x05
#define BPF_JEQ 0x10
#define BPF_K 0x00
#define BPF_RET 0x06

namespace syscalls {
    constexpr int NR_openat = 56;
    constexpr int NR_readlinkat = 78;
    constexpr int NR_ptrace = 117;
    constexpr int NR_prctl = 167;
    constexpr int NR_fork = 220;
}

constexpr int PR_GET_DUMPABLE = 3;
constexpr int PR_GET_SECCOMP = 21;

class SeccompFilter {
public:
    static bool install();
    static bool isInstalled();
private:
    static bool installed_;
    static sock_fprog* buildFilter();
};

class SignalHandler {
public:
    static bool install();
    static void handle(int sig, siginfo_t* info, void* ctx);
private:
    static __thread volatile int in_handler_;
    static uintptr_t getPC(void* ctx);
    static int handleOpenat(uintptr_t* args, uintptr_t* result);
    static int handleReadlinkat(uintptr_t* args, uintptr_t* result);
    static int handlePtrace(uintptr_t* args, uintptr_t* result);
    static int handlePrctl(uintptr_t* args, uintptr_t* result);
};

} // namespace soulbypass
