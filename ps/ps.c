#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include "procfs.h"

#define SIMPLE_HDR "%5s  %-10s  %s\n", "PID", "STATE", "NAME"
#define SIMPLE_FMT "%5i  %-10s  %s\n", pid, state, name

void do_process( int pid )
{
	char *name, *state;

	//TODO: Filter processes according to settings

	name  = proc_get_proc_name( pid );
	state = proc_get_proc_state( pid );

	//TODO: Handle multiple formats
	printf( SIMPLE_FMT );

	if ( name )
		free( name );

	if ( state )
		free( state );
}



int main ( int argc, char **argv, char **envp )
{

	//TODO: Handle arguments

	//TODO: Handle multiple formats
	printf( SIMPLE_HDR );

	proc_scan( do_process );


}

