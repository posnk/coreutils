#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "procfs.h"


void dbg_calltrace( pid_t pid, void *stack, void *ip )
{
#ifdef __i386__
	int s;
	uintptr_t c_ebp, c_eip, l_ebp;
	c_ebp = stack;
	c_eip = ip;
	for (;;) {
		l_ebp = c_ebp;
		if ( c_ebp <= 0x1000 || c_ebp >= 0xc0000000 ) {
			printf("        STACK CORRUPT\n");
			return;
		}
		printf("       0x%x() 0x%x\n",
			c_eip,
			c_ebp);
		s = proc_read_mem( pid, &c_eip, c_ebp + 4, sizeof(uintptr_t));
		if ( s < 0 ){
			perror("read error in proc mem");
			return;
		}
		s = proc_read_mem( pid, &c_ebp, c_ebp    , sizeof(uintptr_t));
		if ( s < 0 ){
			perror("read error in proc mem");
			return;
		}

	}
#endif
}

procsig_t halt_sig;
pid_t target_pid;
tid_t target_tid;
int   debug_fd;

void attach_target( void ) {
	int s;

	fprintf( stderr, "attaching to %i\n", target_pid );

	target_tid = proc_get_first_task( target_pid );

	s = proc_debug_attach( target_pid );
	if ( s < 0 ) {
		fprintf( stderr, "could not attach to target %i\n", target_pid );
		exit( EXIT_FAILURE );
	} else
		debug_fd = s;

	/* Stop the target */
	proc_debug_break( debug_fd );

	/* and we should now be able to resume the target */
	kill( target_pid, SIGCONT );

}

void dump_target_state( void )
{
	mcontext_t mctx;
	char *syscall;
	char *tstate, *pstate;
	tstate  = proc_get_task_state( target_pid, target_tid );
	syscall = proc_get_task_syscall( target_pid, target_tid );
	pstate  = proc_get_proc_state( target_pid );
	printf( "  proc state   : %s\n", pstate );
	printf( "  task state   : %s\n", tstate );
	printf( "  syscall      : %s\n", syscall );
	printf( "  mcontext     : \n");
	proc_get_task_mcontext( target_pid, target_tid, &mctx );
#ifdef __i386__
	printf( "          eax: %08lX edx: %08lX ecx: %08lX ebx: %08lX\n",
		mctx.reg_eax, mctx.reg_edx, mctx.reg_ecx, mctx.reg_ebx );
	printf( "          esp: %08lX ebp: %08lX esi: %08lX edi: %08lX\n",
		mctx.reg_esp, mctx.reg_ebp, mctx.reg_esi, mctx.reg_edi );
	printf( "          eip: %08lX efl: %08lX cs: %02lX ss: %02lX ds: %02lX\n",
		mctx.reg_eip, mctx.reg_eflags, mctx.reg_cs, mctx.reg_ss, mctx.reg_ds );
	printf( "  stack trace  : \n");
	dbg_calltrace( target_pid, (void *) mctx.reg_ebp, (void *) mctx.reg_eip );
#endif
	free( tstate  );
	free( pstate  );
	free( syscall );
}

void run_target( void ) {
	int s;

	/* Resume target */
	proc_debug_continue( debug_fd );

	do {
		s = proc_debug_waitsig( debug_fd, &halt_sig );
		if ( s < 0 ) {
			fprintf( stderr, "failed to wait for break reason!\n");
			exit( EXIT_FAILURE );
		}
	} while ( s == 0 );

	if ( halt_sig.signal != SIGCONT && halt_sig.signal != SIGSTOP ) {
		fprintf(stderr, "target stopped by signal %i\n", halt_sig.signal );
		dump_target_state();
		fprintf(stderr, "delivering signal\n");
	}
	proc_debug_deliversig( debug_fd, &halt_sig );

}


void start_target( const char *path, char *const argv[] )
{
	pid_t child_pid;

	child_pid = fork();
	if ( child_pid == -1 ) {
		perror("could not fork target");
		exit( EXIT_FAILURE );
	}

	if ( child_pid > 0 ) {
		/* We're the parent */
		target_pid = child_pid;

		attach_target();
	} else {
		/* We're the child */

		/* Stop ourselves so the debugger has a chance to attach */
		raise( SIGSTOP );

		/* When we get here we should have the debugger attached */
		execvp( path, argv );

		perror("Failed to execute target");
	}
}

int main ( int argc, char **argv, char **envp )
{
	char *name;
	start_target( argv[1], argv + 1 );
	while ( 1 )
		run_target();
}

