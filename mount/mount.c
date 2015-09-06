#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

int mount(char *dev, char *mp, char *fs, uint32_t flags);

void usage ( const char *reason )
{
	fprintf( stderr, "%s\nUsage: mount <fs> <device> <mountpoint>\n", reason );	
	exit( EXIT_FAILURE );
}

int main ( int argc, char **argv, char **envp )
{

	if ( argc < 4 )
		usage ( "Invalid argument count" );
	
	if ( mount ( argv[2], argv[3], argv[1], 0 ) ) {
		
		fprintf( stderr, "mount: %s\n", strerror(errno) );
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;

}
