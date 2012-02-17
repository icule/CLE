#include "core/exercise.h"
#include "logo/logo.h"
#include "logo/world.h"
#include "logo/entity.h"

exercise_t logo_one_create(void) {
	world_t w = world_new(200,200);
	world_entity_add(w, entity_new(100,100,0.000000));
	exercise_t res = exercise_new(
"dessiner un carre de 40 de cote\n"
,"\n"
"\n"
,"\n"
"\n"
"\n"
"void square(int size) {\n"
"	int i;\n"
"	for (i=0;i<4;i++) {\n"
"		forward(size);\n"
"		right(90);\n"
"	}\n"
"}\n"
"void run (){\n"
"	square(40);\n"
"}\n"
"\n"
"\n"
, w);
	return res;
}
