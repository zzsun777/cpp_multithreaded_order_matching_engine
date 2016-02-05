#ifndef _HARDWARE_BREAKPOINT_
#define _HARDWARE_BREAKPOINT_

#ifdef __linux__
#include <cstdlib>
#include <cstddef>
#include <syscall.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/user.h>
#elif _WIN32
#include <cassert>
#include <windows.h>
#endif

namespace memory
{
namespace debugging
{

// Original source code : https://github.com/mmorearty/hardware-breakpoints
#ifdef _WIN32
inline int insert_memory_breakpoint(void* address, int len, bool write = true)
{
	CONTEXT cxt;
	HANDLE thisThread = GetCurrentThread();

	auto SetBits = [&](unsigned long& dw, int lowBit, int bits, int newValue)
	{
		int mask = (1 << bits) - 1; // e.g. 1 becomes 0001, 2 becomes 0011, 3 becomes 0111
		dw = (dw & ~(mask << lowBit)) | (newValue << lowBit);
	};

	switch (len)
	{
		case 1: len = 0; break;
		case 2: len = 1; break;
		case 4: len = 3; break;
		default: assert(false); // invalid length
	}

	// The only registers we care about are the debug registers
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	// Read the register values
	if (!GetThreadContext(thisThread, &cxt))
		assert(false);

	// Find an available hardware register
	int breakpoint_register_index = -1;
	for (breakpoint_register_index = 0; breakpoint_register_index < 4; ++breakpoint_register_index)
	{
		if ((cxt.Dr7 & (1 << (breakpoint_register_index * 2))) == 0)
			break;
	}
	assert(m_index < 4); // All hardware breakpoint registers are already being used

	switch (breakpoint_register_index)
	{
		case 0: cxt.Dr0 = (DWORD)address; break;
		case 1: cxt.Dr1 = (DWORD)address; break;
		case 2: cxt.Dr2 = (DWORD)address; break;
		case 3: cxt.Dr3 = (DWORD)address; break;
		default: assert(false); // m_index has bogus value
	}

	SetBits(cxt.Dr7, 16 + (breakpoint_register_index * 4), 2, write?3:1);
	SetBits(cxt.Dr7, 18 + (breakpoint_register_index * 4), 2, len);
	SetBits(cxt.Dr7, breakpoint_register_index * 2, 1, 1);

	// Write out the new debug registers
	if (!SetThreadContext(thisThread, &cxt))
		assert(false);

	return breakpoint_register_index;
}
#elif __linux__

// Source : http://stackoverflow.com/questions/8941711/is-is-possible-to-set-a-gdb-watchpoint-programatically
enum {
    DR7_BREAK_ON_EXEC  = 0,
    DR7_BREAK_ON_WRITE = 1,
    DR7_BREAK_ON_RW    = 3,
};

enum {
    DR7_LEN_1 = 0,
    DR7_LEN_2 = 1,
    DR7_LEN_4 = 3,
};

typedef struct {
    char l0:1;
    char g0:1;
    char l1:1;
    char g1:1;
    char l2:1;
    char g2:1;
    char l3:1;
    char g3:1;
    char le:1;
    char ge:1;
    char pad1:3;
    char gd:1;
    char pad2:2;
    char rw0:2;
    char len0:2;
    char rw1:2;
    char len1:2;
    char rw2:2;
    char len2:2;
    char rw3:2;
    char len3:2;
} dr7_t;

typedef void sighandler_t(int, siginfo_t*, void*);// Params : signal_no info context

inline int insert_memory_breakpoint(void* address, sighandler_t handler)
{
    pid_t child;
    pid_t parent = getpid();
    struct sigaction trap_action;
    int child_stat = 0;

    sigaction(SIGTRAP, NULL, &trap_action);
    trap_action.sa_sigaction = handler;
    trap_action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
    sigaction(SIGTRAP, &trap_action, NULL);

    if ((child = fork()) == 0)
    {
        int retval = EXIT_SUCCESS;

        dr7_t dr7 = {0};
        dr7.l0 = 1;
        dr7.rw0 = DR7_BREAK_ON_WRITE;
        dr7.len0 = DR7_LEN_4;

        if (ptrace(PTRACE_ATTACH, parent, NULL, NULL))
        {
            exit(EXIT_FAILURE);
        }

        sleep(1);

        if (ptrace(PTRACE_POKEUSER, parent, offsetof(struct user, u_debugreg[0]), address))
        {
            retval = EXIT_FAILURE;
        }

        if (ptrace(PTRACE_POKEUSER, parent, offsetof(struct user, u_debugreg[7]), dr7))
        {
            retval = EXIT_FAILURE;
        }

        if (ptrace(PTRACE_DETACH, parent, NULL, NULL))
        {
            retval = EXIT_FAILURE;
        }

        exit(retval);
    }

    waitpid(child, &child_stat, 0);
    if (WEXITSTATUS(child_stat))
    {
        return 1;
    }

    return 0;
}

#endif

}//namespace
}//namespace

#endif