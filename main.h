#ifndef MAIN_H
#define MAIN_H
#include "iostream"
#include "vector"
#include "SDL2/SDL.h"
#define STD_ARRAY_LENGTH 65536
extern std::vector<void(*)()> function_vector;
extern int argc;
extern char **argv;
extern bool running;
#endif
