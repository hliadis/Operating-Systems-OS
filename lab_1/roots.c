#include <sys/syscall.h>
#include <unistd.h>
#include "roots.h"

int syscall_find_roots(void) {
	return( syscall(__NR_find_roots) );
}
