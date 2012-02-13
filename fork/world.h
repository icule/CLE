/*
 * world.h: world state modification
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "fork/entity.h"

/* Type definition */
/* The type world_t is defined in entity.h to break a cyclic dependency */

/* memory management */
world_t world_new(double sizeX, double sizeY);
void world_free(world_t w);
world_t world_copy(world_t w);
int world_eq(world_t w1, world_t w2);

/* easy getters/setters */
int world_get_amount_entity(world_t w);
void world_set_step_delay(world_t w, int step_delay);
double world_get_sizeX(world_t w);
double world_get_sizeY(world_t w);


/* state modifications from the turtle (protected functions) */
void world_entity_add(world_t w, entity_t t);
void world_line_add(world_t w, double x1, double y1, double x2, double y2);

void world_redraw(void* w, void *cr,int sizeX,int sizeY);

entity_t world_entity_geti(world_t w,int i); /* helper for the foreach, don't use directly */
#define world_foreach_entity(w,it,t)\
	for (it=0; (t=world_entity_geti(w,it)) ; it++)



#endif /* WORLD_H_ */
