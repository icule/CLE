/* logo.c: main of the Logo lesson */

#include "fork/fork.h"
#include "fork/exercise_header.h"

lesson_t lesson_main() {
	return lesson_new("Fork",1,
			"1fork", fork_1fork_create);
}
