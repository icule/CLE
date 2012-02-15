#ifndef PARSER_H
#define PARSER_H 
 
 
typedef struct world{
	int x;
	int y;
}world;

typedef struct entity{
	int x;
	int y;
	double ang;
}entity;

typedef struct exercise_desc{
  char* exerciseName;
  char* exerciseConstructor;
}exercise_desc;

typedef struct exo_content{
	world w;
	entity e;
	char* lesson_name;
	char* exercise_name;
	int descriptionSize;
	char** description;
	int codeEleveSize;
	char** codeEleve;
	int codeProfSize;
	char** codeProf;
	exercise_desc* descriptor;
}exo_content;

/*Structure who contains informations of the lesson in which we try to insert the parder exercise*/
typedef struct lesson_content{
	char* lesson_name; //contain the name of lesson (extract of lesson_main function)
	char* lesson_name_file; //contain the name of lesson which is used in filesytstem ( in lowercase)
	char* filename; //contains the name of the file whiwh contains the lesson_main function
	int amount; //Amount of exercise in lesson
	exercise_desc** exercises; //array of exercise descriptor of the lesson's exercise
}lesson_content;

#endif