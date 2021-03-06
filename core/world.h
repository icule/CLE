#ifndef CORE_EXERCISE_H
#define CORE_EXERCISE_H

#include <gtk/gtk.h>

typedef struct s_core_world *core_world_t;

#include "core/exercise.h"
#include "core/lesson.h"

struct s_core_world{
    double sizeX, sizeY;
    int step_delay; /* amount of milliseconds to sleep after each redraw */
    int rank; /*Rank of world in exercise*/
    void (*world_repaint)(struct s_core_world*, cairo_t*, int, int); /* Function which repaint the world */
    void (*exercise_run)(exercise_t , char*); /* Function which run the student code*/
    void (*exercise_stop)(struct s_lesson*); /* Function to stop current execution*/
    void (*exercise_demo)(exercise_t); /* Function used to run demo*/
};

#endif