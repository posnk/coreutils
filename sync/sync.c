#include <stdio.h>

void sync();

int main ( int argc, char **argv, char **envp )
{

	printf("Synchronizing filesystems...");
	sync();
	printf("OK\n");

}
