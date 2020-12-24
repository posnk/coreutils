#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "procfs.h"
#define PROC_PATH  "/proc"

static char *read_file( const char *path )
{
	long size;
	int s;
	char *buf;
	size_t rsz;
	FILE *file;

	file = fopen( path, "r" );
	if ( !file ) {
		perror("could not open file");
		goto error_open;
	}

	s = fseek( file, 0, SEEK_END );
	if ( s < 0 ) {
		perror("could not determine file size");
		goto error_size;
	}

	size = ftell( file );
	if ( size < 0 ) {
		perror("could not determine file size");
		goto error_size;
	}

	rewind( file );

	buf = malloc( (size_t) size + 1);
	if ( !buf ) {
		perror("could not allocate buffer");
		goto error_size;
	}

	rsz = fread( buf, 1, (size_t) size, file );
	if ( rsz < 0 ) {
		perror("could not read file");
		goto error_read;
	}

	/* Trim trailing newline */
	if ( rsz > 0 && buf[ rsz - 1 ] == '\n' )
		buf[ rsz - 1 ] = 0;

	buf[ rsz ] = 0;
	buf[ size ] = 0;

	fclose( file );

	return buf;

error_read:
	free( buf );
error_size:
	fclose( file );
error_open:
	return NULL;

}

static int read_file_into( const char *path, void *buffer, size_t size )
{
	size_t rsz;
	FILE *file;

	file = fopen( path, "r" );
	if ( !file ) {
		perror("could not open file");
		goto error_open;
	}

	rsz = fread( buffer, 1, (size_t) size, file );
	if ( rsz < 0 ) {
		perror("could not read file");
		goto error_read;
	}

	fclose( file );

	return 0;

error_read:
	fclose( file );
error_open:
	return -1;

}

static int read_file_at( const char *path, off_t off, void *buffer, size_t size )
{
	int s;
	size_t rsz;
	int file;

	file = open( path, O_RDONLY );
	if ( !file ) {
		perror("could not open file");
		goto error_open;
	}

	s = lseek( file, off, SEEK_SET );
	if ( s == -1 ) {
		perror("could not seek");
		goto error_read;
	}

	rsz = read( file, buffer, (size_t) size );
	if ( rsz < 0 ) {
		perror("could not read file");
		goto error_read;
	}

	close( file );

	return 0;

error_read:
	close( file );
error_open:
	return -1;

}

int    proc_debug_attach( pid_t pid )
{
	int fd;
	char namebuf[64];
	snprintf( namebuf, sizeof namebuf, "%s/%i/sig", PROC_PATH, pid );

	fd = open( namebuf, O_RDWR );
	if ( fd < 0 ) {
		perror("could not attach to process");
		return -1;
	}

	return fd;
}

int    proc_debug_break( int fd )
{
	int s;
	char v = 0;
	s = write( fd, &v, 1 );
	if ( s != 1 ) {
		perror("could not deliver resume process");
		return -1;
	}
	return 0;
}


int    proc_debug_continue( int fd )
{
	int s;
	char v = 1;
	s = write( fd, &v, 1 );
	if ( s != 1 ) {
		perror("could not deliver resume process");
		return -1;
	}
	return 0;
}

int    proc_debug_waitsig( int fd, procsig_t *sig )
{
	int s, xs;
	xs = sizeof( procsig_t );
	s = read( fd, sig, xs );
	if ( s != xs ) {
		if ( errno == EAGAIN )
			return 0;
		perror("could not receive signal");
		return -1;
	}
	return 1;
}

int    proc_debug_deliversig( int fd, const procsig_t *sig )
{
	int s, xs;
	xs = sizeof( procsig_t );
	s = write( fd, sig, xs );
	if ( s != xs ) {
		perror("could not deliver signal");
		return -1;
	}
	return 0;
}

int    proc_debug_detach( int fd )
{
	return close( fd );
}

int proc_read_mem( int pid, void *out, off_t pos, size_t size )
{
	char namebuf[64];
	snprintf( namebuf, sizeof namebuf, "%s/%i/mem", PROC_PATH, pid );

	return read_file_at( namebuf, pos, out, size );
}

char *proc_read_proc_file( int pid, const char *file )
{
	char namebuf[64];
	snprintf( namebuf, sizeof namebuf, "%s/%i/%s", PROC_PATH, pid, file );

	return read_file( namebuf );
}

char *proc_read_task_file( int pid, int tid, const char *file )
{
	char namebuf[64];
	snprintf(
		namebuf,
		sizeof namebuf,
		"%s/%i/tasks/%i/%s",
		PROC_PATH, pid, tid, file );

	return read_file( namebuf );
}

int proc_parse_pid_str( const char *str )
{
	int r;
	char *endp;

	r = strtol( str, &endp, 10 );

	if ( (!endp) || *endp != '\0' )
		return -1;

	return r;
}

void proc_scan( void (*do_process)(pid_t) )
{
	DIR *proc_dir;
	struct dirent *proc_ent;
	int pid;

	/* Get a handle to the procfs root */
	proc_dir = opendir( PROC_PATH );
	if ( !proc_dir ) {
		perror("opendir on proc");
		exit( EXIT_FAILURE );
	}

	/* Iterate over entries in proc */
	while ( ( proc_ent = readdir( proc_dir ) ) != NULL ) {

		pid = proc_parse_pid_str( proc_ent->d_name );

		/* Filter directories that did not correctly parse as a pid */
		if ( pid != -1 )
			do_process( pid );
	}

	closedir( proc_dir );

}

tid_t  proc_get_first_task   ( pid_t pid ) {
	DIR *tasks_dir;
	struct dirent *tasks_ent;
	int tid;
	char namebuf[64];
	snprintf(
		namebuf,
		sizeof namebuf,
		"%s/%i/tasks",
		PROC_PATH, pid );

	/* Get a handle to the tasks dir */
	tasks_dir = opendir( namebuf );
	if ( !tasks_dir ) {
		perror("opendir on tasks");
		exit( EXIT_FAILURE );
	}

	/* Iterate over entries in proc */
	while ( ( tasks_ent = readdir( tasks_dir ) ) != NULL ) {

		tid = proc_parse_pid_str( tasks_ent->d_name );

		/* Filter directories that did not correctly parse as a pid */
		if ( tid != -1 )
			return tid;
	}

	closedir( tasks_dir );

	return 0;
}

char * proc_get_proc_name    ( pid_t pid )
{
	return proc_read_proc_file( pid, "name" );
}

char * proc_get_proc_state   ( pid_t pid )
{
	return proc_read_proc_file( pid, "state" );
}

char * proc_get_task_state   ( pid_t pid, tid_t tid )
{
	return proc_read_task_file( pid, tid, "state" );
}

char * proc_get_task_syscall ( pid_t pid, tid_t tid )
{
	return proc_read_task_file( pid, tid, "syscall" );
}

int   proc_get_task_mcontext( pid_t pid, tid_t tid, mcontext_t *ctx )
{
	char namebuf[64];
	snprintf(
		namebuf,
		sizeof namebuf,
		"%s/%i/tasks/%i/mcontext",
		PROC_PATH, pid, tid );
	return read_file_into( namebuf, ctx, sizeof(mcontext_t) );
}
