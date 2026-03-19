// Dobby Hook Framework
// Placeholder - replace with actual libdobby.a

#ifndef DOBBY_H
#define DOBBY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void* dobby_dummy_t;

// Placeholder macros - actual Dobby has these functions:
// int DobbyHook(void *function_address, void *replace_call, void **origin_call);
// int DobbyInstrument(void *address, void *handler);

#define DobbyHook(addr, repl, orig) (-1)
#define DobbyInstrument(addr, hnd) (-1)

#ifdef __cplusplus
}
#endif

#endif // DOBBY_H
