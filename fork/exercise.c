

#include <string.h> // KILLIT
#include <stdlib.h>
#include <stdio.h>  // KILLIT
#include <unistd.h> // KILLIT
#include <sys/types.h> // KILLIT
#include <sys/wait.h> // KILLIT
#include <errno.h>


#include "core/exercise.h"
#include "core/lesson.h"
#include "fork/exercise.h"
#include "fork/entity.h"
#include "fork/world.h"
#include "fork/entity_userside.h"
#include "UI/CLE.h"

#define MAX_ENTITY 500

/* Prototypes of the exercises composing this lesson */
exercise_t fork_1fork_create(void);

void* exercise_demo_runner(void *exo);
void exercise_run_one_entity(entity_t t);
int exercise_demo_is_running(void* ex);
void exercise_demo_stop(void* ex);
void exercise_run_stop(void* ex);
exercise_t exercise_new(const char *mission, const char *template,const char *prof_solution, void* wo);
void exercise_free(exercise_t e);

static GMutex *demo_runner_running;
static GMutex* run_runner_running;
static char *binary; // The name of the binary
static pid_t*pids;
tree_fork *tree_c,*tree_t;
int end_goal;

/* Function launched in a separate thread to run the demo without locking the UI
 * It is in charge of starting a thread for each turtle to animate, and wait for their completion
 */
void* exercise_demo_runner(void *exo) {
  	printf("Launch of demo\n");
	entity_t t;
	
	end_goal = 0;
	exercise_t e = exo;
	demo_runner_running = (GMutex *)e->demo_runner_running;
	if(e->s_filename==NULL){
	  char *filename= strdup("/tmp/CLEs.XXXXXX");
	  char* binary_t = strdup("/tmp/CLEb.XXXXXX");
	  int ignored =mkstemp(binary_t); // avoid the useless warning on mktemp dangerousness
	  close(ignored);
	  int fd = mkstemp(filename);

	  /* Copy stringified version of userside to file */
	  char *p = userside;
	  int todo = strlen(p);
	  while (todo>0)
		  todo -= write(fd,p,todo);

	  p = strdup(e->prof_solution);
	  todo = strlen(e->prof_solution);
	  while (todo>0)
		  todo -= write(fd,p,todo);

	  close(fd);

	  /* Fork a process to compile it */
	  int status;
	  int gcc[2];
	  pipe(gcc);
	  int pid = fork();
	  switch (pid) {
	  case -1:
		  CLE_log_append(strdup("Error while forking the compiler"));
		  CLE_log_append(strdup(strerror(errno)));
		  break;
	  case 0: // Child: run gcc
		  close(gcc[0]);
		  close(1);
		  close(2);
		  dup2(gcc[1],1);
		  dup2(gcc[1],2);
		  execlp("gcc","gcc","-g","-x","c",filename, "-o",binary_t,"-Wall",NULL);
	  default: // Father: listen what gcc has to tell
		  close(gcc[1]);
		  char buff[1024];
		  int got;
		  while ((got = read(gcc[0],&buff,1023))>0) {
			  buff[got] = '\0';
			  CLE_log_append(strdup(buff));
		  }
		  waitpid(pid,&status,0);
	  }
	  e->s_filename = binary_t;
	  unlink(filename);
	  free(filename);
	}
	
	/* Reset the goal world */
	printf("End of compiling of teacher's sources\n");
	world_free(e->w_goal);
	e->w_goal = world_copy(e->w_init);
	world_set_step_delay(e->w_goal,50); /* FIXME: should be configurable from UI */
	printf("Goal world rebuild\n");
	
	t=world_entity_geti(e->w_goal,0);
	entity_set_world(t,e->w_goal);
	printf("End of building begining turtles\n");
	
	if (pids)
		free(pids);
	pids=malloc (sizeof(pid_t)*world_get_amount_entity(e->w_goal));
	
	int fd[2];
	pipe(fd);
	if(!fork()){
		close(fd[0]);
		execute_proc(e->s_filename,fd[1]);
		close(fd[1]);
		printf("End of strace\n");
	}
	else{
		close(fd[1]);
		if (pids)
			free(pids);
		pids=malloc (sizeof(pid_t)*world_get_amount_entity(e->w_curr));
		printf("Launch all turtles\n");
		/* Launch all the runners */
		param_runner *pr= allocate_param_runner(t,fd[0],e->w_goal);
		entity_fork_run(pr);
		if(tree_t)
			free_tree_fork(tree_t);
		tree_t=pr->racine;
		//printf("Execution end\n");
		/* Re-enable the run running button */
		free_param_runner(pr);
		world_set_step_delay(e->w_goal,0);
		g_mutex_unlock(demo_runner_running);
		end_goal=1;
		printf("goal end!!\n");
	}
	return NULL;
}

void exercise_demo(void* exo) {
  	printf("Lock : %p\n", &binary);
	exercise_t e = exo;
	demo_runner_running = (GMutex *)e->demo_runner_running;
	int res = g_mutex_trylock(demo_runner_running);
	if (!res) {
		printf("Not restarting the demo (it's already running)\n");
		return;
	}

	/* Launch the demo (in a separate thread waiting for the completion of all turtles before re-enabling the button) */
	g_thread_create(exercise_demo_runner,e,0,NULL);
}

void exercise_stop(void* lesson){
  	lesson_t l = lesson;
  	if(exercise_demo_is_running(l->e_curr))
      	exercise_demo_stop(l->e_curr);
  	else
      	exercise_run_stop(l->e_curr);
}


int exercise_demo_is_running(void* exo) {
	exercise_t e = exo;
	demo_runner_running = (GMutex *)e->demo_runner_running;
	int res = g_mutex_trylock(demo_runner_running);
	if (res)
		g_mutex_unlock(demo_runner_running);
	printf("Demo is %srunning\n",(!res?"":"NOT "));
	return !res;
}

void exercise_demo_stop(void* ex) {
	/* Actually, we don't stop the demo since we *need* it to compute the goal world.
	 * Instead, we stop the animation and get it computing as fast as possible.
	 * That's not what we want to do for exercise_run_stop (or whatever you call it). Instead we want to kill the child doing it.
	 */
	exercise_t e = ex;
	world_set_step_delay(e->w_goal,0);
}


/* Small thread in charge of listening everything that the user's turtle printf()s,
 * and add it to the log console */
void *exercise_run_log_listener(void *pipe) {
	int fd = *(int*)pipe;
    char buff[1024];
    int got;
    while ((got = read(fd,&buff,1023))>0) {
      buff[got] = '\0';
      CLE_log_append(strdup(buff));
    }
    return NULL;
}


/* Function in charge of running a particular turtle */
void exercise_run_one_entity(entity_t t) {
}
/* Function launched in a separate thread to run the demo without locking the UI
 * It is in charge of starting a thread for each turtle to animate, and wait for their completion
 */

tree_fork *allocate_tree_fork(tree_fork *tff){
	tree_fork *tf = malloc(sizeof(tree_fork));
	tf->f = (struct tree_fork *)tff;
	tf->s = malloc(MAX_ENTITY*sizeof(struct tree_fork *));
	tf->nb_son = 0;
	tf->pos=-1;
	if(tff!=NULL)
		tf->pos=tff->nb_son;
	return tf;
}

void tree_fork_add_son(tree_fork *tf,param_runner *pr,int pid){
	tree_fork *tfs = allocate_tree_fork(tf);
	tfs->pid=pid;
	tf->s[tf->nb_son]=(struct tree_fork *)tfs;
	pr->list_nodes_tree[pr->nb_t]=tfs;
	tf->nb_son++;
}

void free_tree_fork(tree_fork *tf){
	int i;
	for(i=0;i<tf->nb_son;i++){
		printf("branche %d coupee\n",i);
		free_tree_fork((tree_fork *)tf->s[i]);
	}
	free(tf);
}

int tree_fork_nb_branch_up(tree_fork *tf){
	if(tf==NULL)
		return 0;
	return 1+tf->pos+tree_fork_nb_branch_up((tree_fork *)tf->f);
}

param_runner *allocate_param_runner(entity_t t,int fd,world_t w){
	param_runner *pr=malloc(sizeof(param_runner));
	pr->fd =fd;
	pr->racine = allocate_tree_fork(NULL);
	pr->list_nodes_tree = malloc(MAX_ENTITY*sizeof(tree_fork *));
	pr->list_nodes_tree[0]=pr->racine;
	pr->nb_t=0;
	pr->list_t = malloc(MAX_ENTITY*sizeof(entity_t));
	pr->list_t[0]=t;
	pr->list_pid = malloc(MAX_ENTITY*sizeof(int));
	pr->w = w;
	return pr;
}

void free_param_runner(param_runner *pr){
	free(pr->list_pid);
	free(pr->list_t);
	free(pr->list_nodes_tree);
	free(pr);
}

void add_entity(param_runner *pr,int pos_f,int pid_s){
	pr->list_t[pr->nb_t] = entity_copy(pr->list_t[pos_f]);
	world_entity_add(pr->w,pr->list_t[pr->nb_t]);
	pr->list_pid[pr->nb_t] = pid_s;
	entity_set_world(pr->list_t[pr->nb_t],entity_get_world(pr->list_t[pos_f]));
	pr->nb_t++;
}

int find_pos_pid(int *list_pid,int size,int pid){
	int i;
	for(i=0;i<size;i++){
		if(list_pid[i]==pid)
			return i;
	}
	return -1;
}

typedef struct{
	entity_t t;
	int length;
}data_forward;

void *forward_one_entity(void *df){
	data_forward *data = (data_forward *)df;
	entity_forward(data->t, data->length);
	free(df);
	return NULL;
}

void forward_all_entities(entity_t *list, int size, int length){
	int i;
	GThread **thread= malloc(size*sizeof(GThread *));
	/*g_thread_init(NULL);
	gdk_threads_init();*/
	for(i=0;i<size;i++){
		data_forward *data = malloc(sizeof(data_forward));
		data->t = list[i];
		data->length = length;
		//forward_one_entity((void*)data);
		//gdk_threads_enter();
		thread[i] = g_thread_create(forward_one_entity,(void*)data,1,NULL);
		//gdk_threads_leave();
	}
	for(i=0;i<size;i++){
     	g_thread_join(thread[i]);
  	}
  	free(thread);
}

void *entity_fork_run(void *param){
	param_runner *pr = param;
    char* buf=malloc(512*sizeof(char));
	int got = 0,first=1;
	action *action;
	creat("res/CLE2.txt",0666);
	/*int fd1= open("res/CLE2.txt",O_WRONLY);*/
	
	/*Reading line by line the file descriptor who recieves the commands to draw the line
	 * One line is only one command of a processus*/
	do{
		/*Waiting to have something to read*/
		if ((got=readline(pr->fd,buf,511)) < 0){
      		got=1;
        }
        
        /*Reading ...*/
		else if(got>0){
			buf[got]='\0';
			
			/*Line for the debug file res/CLE2.txt*/
			/*write(fd1,"--------------\n",15);
			write(fd1,buf,strlen(buf));*/
			
			/*New turn each entity moves to make the chronology*/
			if(!strncmp(buf,"new turn",strlen("new turn"))){
				forward_all_entities(pr->list_t,pr->nb_t,10);
      		}
      		
      		/*We recieve a real command*/
      		else{
      			action = build_again_action(buf);
					
				/*Initialize the following lists*/
      			if(first){
      				pr->list_pid[0]=action->pid_father;
      				pr->racine->pid=action->pid_father;
      				pr->nb_t++;
      				first=0;
      			}
      				
      			/*A processus made a fork()*/
      			if(!strcmp(action->call,"clone")){
						
					/*Searching of the entity who gets the number of pid pid_father */
      				int pos_f = find_pos_pid(pr->list_pid,pr->nb_t,action->pid_father);
      				if(pos_f == -1 ){
      					printf("Error pid %d not found\n%s\n",action->pid_father,buf);
      					free_action(action);
      					continue;
      				}
      					
      				/*Create a new entity who'll become a child of the previous entity*/
      				tree_fork_add_son(pr->list_nodes_tree[pos_f],pr,action->pid_son);
      				add_entity(pr, pos_f,action->pid_son);
      				printf("New turtle number %d pid %d\n",pr->nb_t-1,action->pid_son);
      				entity_left(pr->list_t[pr->nb_t-1], 90);
      				int nb_branch = tree_fork_nb_branch_up(pr->list_nodes_tree[pr->nb_t-1]);
      					
      				/*End of creation of entity each entity moves to have a good alignment and to do the form in stair
      				 * So it's just for the design*/
      				entity_forward(pr->list_t[pr->nb_t-1], world_get_sizeY(pr->w)*0.3/pow(2,nb_branch-1));
      				entity_right(pr->list_t[pr->nb_t-1], 90);
      				forward_all_entities(pr->list_t,pr->nb_t,5);
      			}
      				
      			/*A processus made a wait() so we change the color (purple) of the trace*/
      			if(!strcmp(action->call,"wait4") && action->begin){
      				int pos_f = find_pos_pid(pr->list_pid,pr->nb_t,action->pid_father);
      				int *color = malloc(3*sizeof(int));
      				color[0]=1;
      				color[1]=0;
      				color[2]=1;
      				entity_set_color(pr->list_t[pos_f],color);
      				free(color);
      			}
      				
      			/*A processus left the wait() so we change the color (white) of the trace*/
      			if(!strcmp(action->call,"wait4") && action->end){
      				int pos_f = find_pos_pid(pr->list_pid,pr->nb_t,action->pid_father);
      				
      				int *color = malloc(3*sizeof(int));
      				color[0]=0;
      				color[1]=0;
      				color[2]=0;
      				entity_set_color(pr->list_t[pos_f],color);
      				if(action->pid_son>0){
      					int pos_s = find_pos_pid(pr->list_pid,pr->nb_t,action->pid_son);	
      					entity_t s = pr->list_t[pos_s];
      					if(entity_get_end(s)==2){
      						color[0]=1;
      						color[1]=1;
      						color[2]=1;
      						entity_set_color(s,color);
      						entity_set_end(s,1);
      					}
      				}
      				free(color);
      			}
      				
      			/*A processus made an exit()*/
      			if(!strcmp(action->call,"exit_group") && action->end){
      				int pos_f = find_pos_pid(pr->list_pid,pr->nb_t,action->pid_father);
      				int *color = malloc(3*sizeof(int));
      				int end;
     				int pos_gf;
      				tree_fork *gf;
      				if(pos_f == -1 ){
      					printf("Error pid %d not found\n%s\n",action->pid_father,buf);
      					free(color);
      					free_action(action);
      					continue;
      				}
      				
      				/*Determine if the processus is the first processus
      				 * if that's true we change the color of trace (white)
      				 * otherwise we save the state of his father to change the color of trace (red) */
      				if(pr->list_nodes_tree[pos_f]->f != NULL){
      					gf = (tree_fork *)pr->list_nodes_tree[pos_f]->f;
      					pos_gf = find_pos_pid(pr->list_pid,pr->nb_t,gf->pid);
      					end = entity_get_end(pr->list_t[pos_gf]);
      				}
      				else{
      					pos_gf = -1;
      					end = 1;
      					color[0]=1;
      					color[1]=1;
      					color[2]=1;
      					entity_set_color(pr->list_t[pos_f],color);
      					entity_set_end(pr->list_t[pos_f],1);
      				}
      				if(end!=1){
      					color[0]=1;
      					color[1]=0;
      					color[2]=0;
      					entity_set_color(pr->list_t[pos_f],color);
      					entity_set_end(pr->list_t[pos_f],2);
      				}
      				free(color);
      			}
				free_action(action);
      		}
		}
	}
	while(got>0);
    free(buf);
    return NULL;
}
 
void* exercise_run_runner(void *exo) {
	entity_t t;
	exercise_t e = exo;
	while(!end_goal);
	printf("goal end : %d\n",end_goal);

	/* Reset the goal world */
	
	world_free(e->w_curr);
	e->w_curr = world_copy(e->w_init);
	world_set_step_delay(e->w_curr,50);  /* FIXME: should be configurable from UI */
	
	t=world_entity_geti(e->w_curr,0);
	entity_set_world(t,e->w_curr);

	/*Test strace*/
	int fd[2];
	pipe(fd);
	if(!fork()){
		close(fd[0]);
		execute_proc(binary,fd[1]);
		close(fd[1]);
		printf("End of strace\n");
	}
	else{
		close(fd[1]);
		if (pids)
			free(pids);
		pids=malloc (sizeof(pid_t)*world_get_amount_entity(e->w_curr));
		printf("Launch all turtles\n");
		
		/* Launch all the runners */
		param_runner *pr= allocate_param_runner(t,fd[0],e->w_curr);
		entity_fork_run(pr);
		if(tree_c)
			free_tree_fork(tree_c);
		tree_c = pr->racine;
		
		/* Re-enable the run running button */
		free_param_runner(pr);
		world_set_step_delay(e->w_curr,0);
		printf("End of execution\n");

		if (world_eq(tree_c,tree_t,e->w_curr,e->w_goal))
			CLE_dialog_success();
		else
			CLE_dialog_failure("Your world differs from the goal");
		g_mutex_unlock(run_runner_running);
		free(pids);
		pids=NULL;
		unlink(binary);
		free(binary);
	}
	return NULL;
}

void exercise_run_stop(void* ex) {
	/* actually kill all the processes */
	int it;
	exercise_t e = ex;
	if (pids)
		for (it=0;it< world_get_amount_entity(e->w_curr); it++)
			kill(pids[it],SIGTERM);
}


void exercise_run(void* ex, char *source) {
	// BEGINKILL
	int status; // test whether they were compilation errors
	exercise_t e = ex;
	
	run_runner_running = (GMutex *)e->run_runner_running;
	int res = g_mutex_trylock(run_runner_running);
	
	if (!res) {
		printf("Not restarting the execution (it's already running)\n");
		return;
	}

	/* clear the logs */
	CLE_log_clear();

	/* create 2 filenames */
	char *filename= strdup("/tmp/CLEs.XXXXXX");
	binary = strdup("/tmp/CLEb.XXXXXX");
	int ignored =mkstemp(binary); // avoid the useless warning on mktemp dangerousness
	close(ignored);
	int fd = mkstemp(filename);

	/* Copy stringified version of userside to file */
	char *p = userside;
	int todo = strlen(p);
	while (todo>0)
		todo -= write(fd,p,todo);

	p = source;
	todo = strlen(source);
	while (todo>0)
		todo -= write(fd,p,todo);

	close(fd);

	/* Fork a process to compile it */
	int gcc[2];
	pipe(gcc);
	int pid = fork();
	switch (pid) {
	case -1:
		CLE_log_append(strdup("Error while forking the compiler"));
		CLE_log_append(strdup(strerror(errno)));
		break;
	case 0: // Child: run gcc
		close(gcc[0]);
		close(1);
		close(2);
		dup2(gcc[1],1);
		dup2(gcc[1],2);
		execlp("gcc","gcc","-g","-x","c",filename, "-o",binary,"-Wall",NULL);
	default: // Father: listen what gcc has to tell
		close(gcc[1]);
		char buff[1024];
		int got;
		while ((got = read(gcc[0],&buff,1023))>0) {
			buff[got] = '\0';
			printf("gcc error : %s", buff);
			CLE_log_append(strdup(buff));
		}
		waitpid(pid,&status,0);
	}
	exercise_set_binary(e, binary);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		/* Launch the exercise (in a separate thread waiting for the completion of all turtles before re-enabling the button) */
		g_thread_create(exercise_run_runner,e,0,NULL);
	} else  {
		g_mutex_unlock(run_runner_running);

		if (WIFEXITED(status)) {
			CLE_log_append(strdup("Compilation error. Abording code execution\n"));
		} else if (WIFSIGNALED(status)) {
			CLE_log_append(strdup("The compiler got a signal. Weird.\n"));
		}
	}

	unlink(filename);
	free(filename);
	//ENDKILL
}


exercise_t exercise_new(const char *mission, const char *template,
	const char *prof_solution, void* wo) {
	world_t w = (world_t)wo;
	exercise_t result = malloc(sizeof(struct s_exercise));
	result->mission = mission;
	result->template = template;
	result->prof_solution = prof_solution;
	result->s_filename = NULL;
	result->binary=NULL;
	result->demo_runner_running = g_mutex_new ();
	result->run_runner_running = g_mutex_new ();
	result->w_init = (void*)w;
	result->w_curr = (void*)world_copy(w);
	result->w_goal = world_copy(w);
	(*(global_data->lesson->exercise_demo))(result);
	return result;
}

void exercise_free(exercise_t e) {
	world_free(e->w_init);
	world_free(e->w_curr);
	world_free(e->w_goal);
	free(e);
}
