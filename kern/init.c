#include <inc/string.h>
#include <inc/stdio.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/pmap.h>
#include <kern/env.h>
#include <kern/trap.h>

void
i386_init(void)
{
	extern char edata[], end[];

	// Before doing anything else, complete the ELF loading process.
	// Clear the unitialized global data (BSS) section of our program.
	// This ensures that all static/global variables start out zero.
	memset(edata, 0, end - edata);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	cprintf("Hello, I'm Yu-OS\n");

	mem_init();

	env_init();
	trap_init();

//	ENV_CREATE(user_hello, ENV_TYPE_USER);
//	ENV_CREATE(user_divzero, ENV_TYPE_USER);
//	ENV_CREATE(user_dumbfork, ENV_TYPE_USER);
//	ENV_CREATE(user_faultread, ENV_TYPE_USER);
//	ENV_CREATE(user_faultdie, ENV_TYPE_USER);
//	ENV_CREATE(user_faultalloc, ENV_TYPE_USER);
//	ENV_CREATE(user_faultallocbad, ENV_TYPE_USER);
	ENV_CREATE(user_forktree, ENV_TYPE_USER);
//	cprintf("env_create success\n");

	env_run(&envs[0]);
//	cprintf("finish envs[0]");

	// Drop into the kernel monitor.
	while (1) {
		monitor(NULL);
	}
}

/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void
_panic(const char *file, int line, const char *fmt,...)
{
	va_list	ap;

	if (panicstr) {
		goto dead;
	}
	panicstr = fmt;

	// Be extra sure that the machine is in as reasonable state
	__asm __volatile("cli; cld");

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1) {
		monitor(NULL);
	}
}