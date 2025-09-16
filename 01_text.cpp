#include "system.h"
#include "main.h"
#include "helpers.h"
#include "01_text.h"


const char* introMessage = "My First Demo";
const char* creditMessage = "from Anon Monk";



float fontHeightNDC = 0.10f;   // 0.10-1.00 fontHeight
const float  STROKE_EM_UNITS = 119.0f;  // GLUT-Stroke „Zeilenhöhe“


// Text-Baseline (linke untere Ecke) in NDC
float x_ndc = 0.0f;   // linke Baseline X
float y_ndc = -1.1f;  // Baseline Y (startet unter Bild)



void drawIntroRise() {


    int   units = stroke_text_width_units(introMessage);
    float scale = fontHeightNDC / STROKE_EM_UNITS;
    float textW = (float)units * scale;

    x_ndc = -0.5f * textW; // mittig ausrichten



    glPushMatrix();
    glTranslatef(x_ndc, y_ndc, 0.f);
    glScalef(scale, scale, 1.f);
    glLineWidth(2.0f);
    glColor3f(0.f, 1.f, 0.f);
    draw_stroke_text(introMessage);
    glPopMatrix();

}

float  riseSpeed = 0.020f;
double reachTime = 0.0;


void drawIntroCredit() {


    int   units = stroke_text_width_units(creditMessage);
    float scale = fontHeightNDC / STROKE_EM_UNITS;
    float textW = (float)units * scale;

    x_ndc = -0.5f * textW; // mittig ausrichten



    glPushMatrix();
    glTranslatef(x_ndc, y_ndc, 0.f);
    glScalef(scale, scale, 1.f);
    glLineWidth(2.0f);
    glColor3f(0.f, 1.f, 0.f);
    draw_stroke_text(creditMessage);
    glPopMatrix();

}


void updateIntroRise() {

    float targetY = 0.0f; // Mitte

    if (y_ndc < targetY) { 
        y_ndc += riseSpeed; 
        if (y_ndc > targetY) y_ndc = targetY; 
    }
    
    if (y_ndc >= targetY) { 
        reachTime = curTime; 
    }

}


void updateIntroCredit()  {


}