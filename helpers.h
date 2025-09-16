#pragma once

extern double deltaTime;

extern double curTime;
extern double startTime;
extern unsigned int frameCount;
extern double lastTime;
extern double fps;


void updateTimeSystem();
void showFPS();

float frand(float a, float b);

int stroke_text_width_units(const char* s);
void draw_stroke_text(const char* s);