/* logo.c: main of the Logo lesson */

#include "fork/fork.h"
#include "fork/exercise_header.h"

lesson_t lesson_main() {
	if(access("/usr/bin/strace", F_OK)){
		printf("/usr/bin/strace not found\n");
		return NULL;
	}
	return lesson_new("Fork",3,
			"1 fork", fork_1fork_create,
			"what the fork?!", fork_what_the_fork_create);
}
