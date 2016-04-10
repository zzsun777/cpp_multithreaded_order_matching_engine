#ifndef _CPU_ARCHITECTURE_
#define _CPU_ARCHITECTURE_

#if defined(_MSC_VER)
#ifdef _M_IX86
#define ARCH_X86
#else
#define ARCH_X64
#endif
#elif defined(__GNUC__)
#ifdef __i386__
#define ARCH_X86
#else
#define ARCH_X64
#endif
#else
#error "cpu_architecture.h : Non supported compiler"
#endif

#endif