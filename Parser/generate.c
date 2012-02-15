#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"


void printInclude(exo_content *ex, FILE* src)
{
  fprintf(src, "#include \"core/exercise.h\"\n");
  fprintf(src, "#include \"%s/%s.h\"\n", ex->lesson_name, ex->lesson_name);
  fprintf(src, "#include \"%s/world.h\"\n", ex->lesson_name);
  fprintf(src, "#include \"%s/entity.h\"\n", ex->lesson_name);
  fprintf(src, "\n");
}

void printLine(char** lines, int amount, FILE* src)
{
  int i;
  for(i=0; i<amount; ++i)
  {
    fprintf(src, "\"%s\\n\"\n", lines[i]);
  }
}


void printExercise(exo_content *ex, FILE* src)
{
  fprintf(src, "exercise_t %s_%s_create(void) {\n", ex->lesson_name, ex->exercise_name);
  fprintf(src, "\tworld_t w = world_new(%d,%d);\n", ex->w.x, ex->w.y);
  fprintf(src, "\tworld_entity_add(w, entity_new(%d,%d,%lf));\n", ex->e.x, ex->e.y, ex->e.ang);
  fprintf(src, "\texercise_t res = exercise_new(\n");
  printLine(ex->description, ex->descriptionSize, src);
  fprintf(src, ",");
  printLine(ex->codeEleve, ex->codeEleveSize, src);
  fprintf(src, ",");
  printLine(ex->codeProf, ex->codeProfSize, src);
  fprintf(src, ", w);\n");
  fprintf(src, "\treturn res;\n}\n");
}


int generateExerciseFile(exo_content *ex, lesson_content *lesson)
{
  int i;
  for(i=0; i< lesson->amount; ++i)
  {
    if(!strcmp(lesson->exercises[i]->exerciseConstructor, ex->descriptor->exerciseConstructor))
      printf("L'exercice %s existe déjà dans la leçon %s\n", ex->exercise_name, ex->lesson_name);
  }
  char* filename = malloc(sizeof(char)*(strlen(ex->exercise_name)+strlen(ex->lesson_name)*2+5));
  sprintf(filename, "%s/%s_%s.c",ex->lesson_name, ex->lesson_name, ex->exercise_name);
  printf("Nom du fichier source a créer %s\n", filename);
  
  FILE* exerciseSrc = fopen(filename, "w");
  printInclude(ex, exerciseSrc);
  printExercise(ex, exerciseSrc);
  fclose(exerciseSrc);
  return 0;
}

int generateLessonFile(lesson_content* lesson)
{
  FILE* file = fopen(lesson->filename, "w");
  lesson->lesson_name[0] = tolower(lesson->lesson_name[0]);
  fprintf(file, "#include \"%s/%s.h\"\n", lesson->lesson_name, lesson->lesson_name);
  fprintf(file, "#include \"%s/exercise_header.h\"\n\n", lesson->lesson_name);
  fprintf(file, "lesson_t lesson_main() {\n");
  lesson->lesson_name[0] = toupper(lesson->lesson_name[0]);
  fprintf(file, "\treturn lesson_new(\"%s\",%d", lesson->lesson_name, lesson->amount);
  int i=0;
  for(; i< lesson->amount; ++i)
  {
    fprintf(file, "\n,\t\t\t\"%s\", %s", lesson->exercises[i]->exerciseName, lesson->exercises[i]->exerciseConstructor);
  }
  fprintf(file, ");\n}");
  fclose(file);
  return 0;
}

int generateExerciseHeader(lesson_content* lesson)
{
  lesson->lesson_name[0] = tolower(lesson->lesson_name[0]);
  char* filename = malloc(sizeof(char)*(strlen(lesson->lesson_name)+strlen("/exercise_headers.h")+1));
  sprintf(filename, "%s/exercise_header.h", lesson->lesson_name);
  printf("%s\n", filename);
  FILE* file = fopen(filename, "w");
  fprintf(file, "#ifndef LOGO_EXERCISE_HEADER_H\n");
  fprintf(file, "#define LOGO_EXERCISE_HEADER_H\n\n");
  fprintf(file, "#include \"core/exercise.h\"\n\n");
  int i=0;
  for(; i< lesson->amount; ++i)
  {
    fprintf(file, "\nexercise_t %s(void);", lesson->exercises[i]->exerciseConstructor);
  }
  fprintf(file, "\n\n#endif\n");
  fclose(file);
  free(filename);
  return 0;
}




