#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "fork/exercise.h"
#include "fork/fork.h"
#include "fork/world.h"
#include "fork/entity.h"

/* We use here the fact that C strings get automatically concatenated by the preprocessor
 * Ie, "aze" "rty" is changed into "azerty" by the preprocessor
 */
exercise_t fork_what_the_fork_create(void) {
	printf("Construction de l'exercise what_the_fork\n");
	core_world_t w = world_new(400,400);
	world_entity_add(w,entity_new(10.0, 300.0, 0.));
	exercise_t res = exercise_new(
			"Dans cet exercice, prie!!!!!!!!!!!!!\n",

			"void run() {\n"
			"  fork();\n"
			"  fork();\n"
			"  fork();\n"
			"  fork();\n"
			"}",
			"void run() {\n"
			"  fork();\n"
			"  fork();\n"
			"  fork();\n"
			"  fork();\n"
			"}");
			
	exercise_add_world(res, w);

	return res;
}
