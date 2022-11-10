#ifndef PLUM_DEFS

#define PLUM_DEFS

#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

#if defined(PLUM_NO_STDINT) || defined(PLUM_NO_ANON_MEMBERS) || defined(PLUM_NO_VLA)
  #error libplum feature-test macros must not be defined when compiling the library.
#elif defined(__cplusplus)
  #error libplum cannot be compiled with a C++ compiler.
#elif __STDC_VERSION__ < 201710L
  #error libplum requires C17 or later.
#elif SIZE_MAX < 0xffffffffu
  #error libplum requires size_t to be at least 32 bits wide.
#endif

#ifdef noreturn
  #undef noreturn
#endif
#define noreturn _Noreturn void

#define alignto(amount) alignas(((amount) < alignof(max_align_t)) ? (amount) : alignof(max_align_t))

#define bytematch(address, ...) (!memcmp((address), (unsigned char []) {__VA_ARGS__}, sizeof (unsigned char []) {__VA_ARGS__}))
#define bytewrite(address, ...) (memcpy(address, (unsigned char []) {__VA_ARGS__}, sizeof (unsigned char []) {__VA_ARGS__}))
#define byteoutput(context, ...) (bytewrite(append_output_node((context), sizeof (unsigned char []) {__VA_ARGS__}), __VA_ARGS__))
#define byteappend(address, ...) (bytewrite(address, __VA_ARGS__), sizeof (unsigned char []) {__VA_ARGS__})

#define swap(T, first, second) do {T temp = first; first = second; second = temp;} while (false)

#endif
