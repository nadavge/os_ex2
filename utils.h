#ifndef _UTILS_THREADS_H
#define _UTILS_THREADS_H

//================================MACROS===============================

#define HANDLE_SYSTEM_ERROR(MSG) cerr << SYSTEM_ERROR MSG << endl; exit(1)
#define HANDLE_LIBRARY_ERROR(MSG) cerr << LIBRARY_ERROR MSG <<endl;
#define START_TIMER() setitimer(ITIMER_VIRTUAL, &gTvQuanta, nullptr)
#define STOP_TIMER() setitimer(ITIMER_VIRTUAL, &gTvDisable, nullptr)

//================================DEFINITIONS==========================

#define MAIN_ID 0
#define ERROR -1
#define MICRO 1000000

#define SYSTEM_ERROR "system error: "
#define LIBRARY_ERROR "thread library error: "
#define TIMER_ERROR "Setting timer failed"
#define TOO_MANY_THREADS_ERROR "Too many threads"
#define ACCESS_NULL_THREAD_ERROR "Accessing non-existent thread"
#define SIGNAL_ERROR "Error in handling signals"
#define INVALID_CALL_ERROR "Invalid call to function"

/**
* @brief Used to describe whether to return to run the current thread
* or switch a thread
*/
enum JumpType
{
	/*
	 * SWITCHING should be first, to act by the spec of sigsetjmp
	 * since sigsetjmp returns 0 on first call
	 */
	SWITCHING,
	RETURN
};

/**
* @brief What action to perform while switching
*/
enum SwitchAction
{
	DEF_SWITCH,
	SUSPEND,
	TERMINATE
};

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
				 "rol    $0x11,%0\n"
				 : "=g" (ret)
				 : "0" (addr));
	return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%gs:0x18,%0\n"
				 "rol    $0x9,%0\n"
				 : "=g" (ret)
				 : "0" (addr));
	return ret;
}

#endif

#endif //_UTILS_THREADS_H
