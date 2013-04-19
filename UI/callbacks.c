/*
 * callbacks.c: defines all the callbacks, ie the functions called when the buttons get clicked
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdk.h>
#include <gtksourceview/gtksourceview.h>

#include "UI/CLE.h"
#include "core/exercise.h"
#include "core/lesson.h"
#include "core/world.h"
#include "core/log.h"
#include "core/mark.h"

G_MODULE_EXPORT void
cb_run_clicked(GtkButton *button) {
	CLE_clear_worlds_mark();
	CLE_clear_worlds_log();
	char *source = CLE_get_sourcecode();
	global_data->state = RUN;
	GtkAdjustment *adj = CH_GET_OBJECT(global_data->builder, adjustment, GTK_ADJUSTMENT);
	global_data->speed = (int)gtk_adjustment_get_value(adj);

	/* Switch the notebook to the first page (which is #0), where the student code runs */
	gtk_notebook_set_current_page(global_data->world_views,0);

	printf("Execution\n");
	(*(global_data->lesson->e_curr->w_curr[0]->exercise_run))(global_data->lesson->e_curr,source);

	free(source);
}

G_MODULE_EXPORT void
cb_debug_clicked(GtkButton *button) {
//   printf("\n\n");
  CLE_clear_worlds_mark();
  CLE_clear_worlds_log();
  char *source = CLE_get_sourcecode();
  global_data->state = DEBUG;
  GtkAdjustment *adj = CH_GET_OBJECT(global_data->builder, adjustment, GTK_ADJUSTMENT);
  global_data->speed = (int)gtk_adjustment_get_value(adj);
  
  /* Switch the notebook to the first page (which is #0), where the student code runs */
  gtk_notebook_set_current_page(global_data->world_views,0);
  
  printf("Debug Execution\n");
  (*(global_data->lesson->e_curr->w_curr[0]->exercise_run))(global_data->lesson->e_curr,source);

  free(source);
}



G_MODULE_EXPORT void
cb_stop_clicked(GtkButton *button) {
	printf("Stop clicked\n");
	global_data->state = STOP;
	(*(global_data->lesson->e_curr->w_curr[0]->exercise_stop))(global_data->lesson);
}

G_MODULE_EXPORT void
cb_demo_clicked(GtkButton *button) {
	CLE_clear_worlds_mark();
	CLE_clear_worlds_log();
	global_data->state = DEMO;
	/* Switch the notebook to the second page (which is #1), where the demo is */
	gtk_notebook_set_current_page(global_data->world_views,1);
	(*(global_data->lesson->e_curr->w_curr[0]->exercise_demo))(global_data->lesson->e_curr);
}

G_MODULE_EXPORT void
cb_step_by_step_clicked(GtkButton *button) {
	printf("Step by step clicked\n");
	/*
	gtk_notebook_set_current_page(global_data->world_views,0);
	
	global_data->step_by_step = 1;
	global_data->stop = 0;
	global_data->run = 0;
	
	if(!global_data->isrunning){
	  CLE_clear_worlds_log();
		char *source = CLE_get_sourcecode();

		// BEGINKILL
		global_data->isrunning = 1;
		(*(global_data->lesson->e_curr->w_curr[0]->exercise_run))(global_data->lesson->e_curr,source);
	}*/
}


G_MODULE_EXPORT void
world_selection_change(GtkComboBox *arg0, gpointer   user_data){
  if(gtk_combo_box_get_active(arg0) ==-1)
    return;
  CLE_log_clear();
  CLE_clear_mark(global_data->current_world_expose);
  
  global_data->current_world_expose = gtk_combo_box_get_active(arg0);
  world_ask_repaint(global_data->lesson->e_curr->w_curr[global_data->current_world_expose]);
  world_ask_repaint(global_data->lesson->e_curr->w_goal[global_data->current_world_expose]);
  if(global_data->worlds_log[global_data->current_world_expose])
    CLE_log_append(strdup(global_data->worlds_log[global_data->current_world_expose]));
  CLE_show_mark();
}

/* The about dialog window */
static GtkAboutDialog * diag = NULL;

G_MODULE_EXPORT void
cb_menu_about(GtkButton *button) {

	diag = CH_GET_OBJECT(global_data->builder, aboutdialog, GTK_ABOUT_DIALOG);
	gtk_widget_show(GTK_WIDGET(diag));
}

G_MODULE_EXPORT void
cb_scale_speed_value_changed(GtkAdjustment *adj, gpointer value) {
	//printf("Adj : %f\n",gtk_adjustment_get_value(adj));
	global_data->speed= (int)gtk_adjustment_get_value(adj);
}

G_MODULE_EXPORT void
on_aboutdialog_close(GtkDialog *arg0, gpointer   user_data){
	gtk_widget_destroy(GTK_WIDGET(diag));
}



