#ifndef __procfs_h__
#define __procfs_h__

#include <stdint.h>
#include <sys/types.h>
#include <sys/mcontext.h>
#include <sys/procfs.h>

typedef pid_t tid_t;

char  *proc_read_proc_file( int pid, const char *file );
char  *proc_read_task_file( int pid, int tid, const char *file );
int    proc_parse_pid_str( const char *str );
void   proc_scan( void (*do_process)(pid_t) );
tid_t  proc_get_first_task   ( pid_t pid );
char * proc_get_proc_name    ( pid_t pid );
char * proc_get_proc_state   ( pid_t pid );
char * proc_get_task_state   ( pid_t pid, tid_t tid );
char * proc_get_task_syscall ( pid_t pid, tid_t tid );
int    proc_get_task_mcontext( pid_t pid, tid_t tid, mcontext_t *ctx );
int    proc_read_mem( int pid, void *out, off_t pos, size_t size );
int    proc_debug_attach( pid_t pid );
int    proc_debug_break( int fd );
int    proc_debug_continue( int fd );
int    proc_debug_waitsig( int fd, procsig_t *sig );
int    proc_debug_deliversig( int fd, const procsig_t *sig );
int    proc_debug_detach( int fd );
#endif
