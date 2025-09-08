#pragma once



#define ScreenWidth 640
#define ScreenHeight 480




#ifdef _WIN32

#include <windows.h>
#include <mmsystem.h>
#include <GL/freeglut.h>

#pragma comment(lib, "winmm.lib")

#endif





void music_start(const char* path);
void music_task();
void music_stop();


void init_system();
void demo_quit();