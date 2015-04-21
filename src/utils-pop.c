#include <common-pop.h>

void fatal(char *msg)
{
	fprintf(stderr, "%s", "[FATAL] ");
	fprintf(stderr, "%s.\n", msg);
	exit(EXIT_FAILURE);
}
