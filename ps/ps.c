#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

#define PROC_PATH  "/proc"
#define SIMPLE_HDR "%5s  %-10s  %s\n", "PID", "STATE", "NAME"
#define SIMPLE_FMT "%5i  %-10s  %s\n", pid, state, name

char *read_file( const char *path )
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

char *read_procfile( int pid, const char *file ) {
	char namebuf[64];
	snprintf( namebuf, sizeof namebuf, "%s/%i/%s", PROC_PATH, pid, file );

	return read_file( namebuf );
}

char *read_taskfile( int pid, int tid, const char *file ) {
	char namebuf[64];
	snprintf(
		namebuf,
		sizeof namebuf,
		"%s/%i/tasks/%i/%s",
		PROC_PATH, pid, tid, file );

	return read_file( namebuf );
}

int parse_pidstr( const char *str )
{
	int r;
	char *endp;

	r = strtol( str, &endp, 10 );

	if ( (!endp) || *endp != '\0' )
		return -1;

	return r;
}

void do_process( int pid )
{
	char *name, *state;

	//TODO: Filter processes according to settings



	name  = read_procfile( pid, "name" );
	state = read_procfile( pid, "state" );

	//TODO: Handle multiple formats
	printf( SIMPLE_FMT );

	if ( name )
		free( name );

	if ( state )
		free( state );
}

void scan_proc( void )
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

		pid = parse_pidstr( proc_ent->d_name );

		/* Filter directories that did not correctly parse as a pid */
		if ( pid != -1 )
			do_process( pid );
	}

	closedir( proc_dir );

}

int main ( int argc, char **argv, char **envp )
{

	//TODO: Handle arguments

	//TODO: Handle multiple formats
	printf( SIMPLE_HDR );

	scan_proc();


}

