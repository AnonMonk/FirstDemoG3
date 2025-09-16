#include <cstdio>
#include "system.h"
#include "helpers.h"


double deltaTime;
double curTime;
double startTime;
unsigned int frameCount = 0;
double lastTime = 0.0;
double fps = 0.0;



inline float px_to_ndc_x(float px) { return (px / (float)ScreenWidth) * 2.0f - 1.0f; }
inline float px_to_ndc_y(float py) { return (py / (float)ScreenHeight) * 2.0f - 1.0f; }

int bitmap_width_px(const char* s, void* font) {
    int w = 0; 
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) w += glutBitmapWidth(font, *p);
    return w;
}

void draw_bitmap_ndc(float x_left_ndc, float y_base_ndc, const char* s, void* font) {
    glRasterPos2f(x_left_ndc, y_base_ndc);
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) glutBitmapCharacter(font, *p);
}


int stroke_text_width_units(const char* s) {
    int w = 0; for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        w += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    return w;
}
void draw_stroke_text(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
}

float frand(float a, float b) { return a + (b - a) * (float)rand() / (float)RAND_MAX; }

void updateTimeSystem() {

    curTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;

    frameCount++;
    
    if (curTime - lastTime >= 1.0) { 
        fps = frameCount / (curTime - lastTime); 
        lastTime = curTime; 
        frameCount = 0; 
    }
 
    deltaTime = curTime - startTime;

}



void showFPS() {

    char fpsBuf[32]; 
    std::snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %d", (int)fps);

    int fw_px = bitmap_width_px(fpsBuf, GLUT_BITMAP_9_BY_15);
    float fps_x = px_to_ndc_x((float)ScreenWidth - (float)fw_px - 5.f);
    float fps_y = px_to_ndc_y((float)ScreenHeight - 20.f);

    glColor3f(0.f, 1.f, 0.f);
    draw_bitmap_ndc(fps_x, fps_y, fpsBuf, GLUT_BITMAP_9_BY_15);

}
