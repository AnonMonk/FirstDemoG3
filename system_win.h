#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <GL/freeglut.h>
#include "system_win.h"

#pragma comment(lib, "winmm.lib")



void music_start(const char* path);
void music_task();
void music_stop();