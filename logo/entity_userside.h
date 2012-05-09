/* This file was automatically generated from logo/turtle_userside.c */
/* DO NOT EDIT */

#ifndef LOGO_ENTITY_USERSIDE
#define LOGO_ENTITY_USERSIDE
static char *userside = 
    "/*\n"
    " * entity_userside.c: functions callable from user code\n"
    " */\n"
    "#include <stdio.h>\n"
    "\n"
    "// BEGINKILL\n"
    "#include <stdarg.h>\n"
    "\n"
    "static FILE *in = NULL;\n"
    "static FILE *out = NULL;\n"
    "static void cmd(char *fmt, ...) {\n"
    "	va_list va;\n"
    "	va_start(va,fmt);\n"
    "	vfprintf(out, fmt, va);\n"
    "}\n"
    "\n"
    "double response(char* fmt, ...)\n"
    "{\n"
    "  double res=0;\n"
    "  va_list va;\n"
    "  va_start(va,fmt);\n"
    "  /* The next line causes crash, do not decomment */\n"
    "  //vfscanf(in, fmt, va);\n"
    "  return res;\n"
    "}\n"
    "\n"
    "\n"
    "/* easy getters */\n"
    "double get_x(void) {\n"
    "	cmd(\"100 GETX\\n\");\n"
    "	double res;\n"
    "	return response(\"%lf\", &res);\n"
    "}\n"
    "double get_y(void) {\n"
    "	cmd(\"101 GETY\\n\");\n"
    "	double res;\n"
    "	return response(\"%lf\", &res);\n"
    "}\n"
    "double get_heading(void) {\n"
    "	cmd(\"102 GETHEADING\\n\");\n"
    "	double res;\n"
    "	return response(\"%lf\", &res);\n"
    "}\n"
    "/* User API */\n"
    "void forward(double steps) {\n"
    "	cmd(\"103 %f FORWARD %f\\n\",steps,steps);\n"
    "}\n"
    "void backward(double steps) {\n"
    "	cmd(\"104 %f BACKWARD %f\\n\",steps,steps);\n"
    "}\n"
    "void left(double angle) {\n"
    "	cmd(\"105 %f LEFT %f\\n\",angle, angle);\n"
    "}\n"
    "void right(double angle) {\n"
    "	cmd(\"106 %f RIGHT %f\\n\",angle, angle);\n"
    "}\n"
    "void pen_up(void) {\n"
    "	cmd(\"107 UP\\n\");\n"
    "}\n"
    "void pen_down(void) {\n"
    "	cmd(\"108 DOWN\\n\");\n"
    "}\n"
    "void run();\n"
    "int main(int argc,char *argv[]) {\n"
    "	in = fdopen(3,\"r\");\n"
    "	out = fdopen(4,\"w\");\n"
    "	run();\n"
    "	fflush(out);\n"
    "	fflush(in);\n"
    "	fclose(in);\n"
    "	fclose(out);\n"
    "	return 0;\n"
    "}\n"
    "// ENDKILL\n"
;
#endif /* LOGO_ENTITY_USERSIDE */
/* This file was automatically generated from logo/turtle_userside.c */
/* DO NOT EDIT */
