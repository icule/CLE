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
}exo_content;

#endif