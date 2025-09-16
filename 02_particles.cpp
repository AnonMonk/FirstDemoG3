#include <math.h>
#include <cstdio>
#include "system.h"
#include "main.h"
#include "helpers.h"
#include "02_particles.h"

const char* mainMessage = "Happy Birthday Corvus";

const int MAX_PARTICLES = 250; 
const float gravity = -0.15f; // NDC/s^2
const int spawnCount = 12;

// Richtung der Bewegung
float  vx = 0.008f;
float  vy = 0.008f;

struct Particle {
    float x, y;     // Position (NDC)
    float vx, vy;   // Geschwindigkeit
    float life;     // [0..1]
    float r, g, b;  // Farbe
    float size;     // Kantenlänge (NDC)
};
Particle P[MAX_PARTICLES];



// Text-Baseline (linke untere Ecke) in NDC
float part_x_ndc = 0.0f;   // linke Baseline X
float part_y_ndc = -1.1f;  // Baseline Y (startet unter Bild)

float scale;




const float part_fontHeightNDC = 0.10f;   // 0.10-1.00 fontHeight
const float STROKE_EM_UNITS = 119.0f;  // GLUT-Stroke „Zeilenhöhe“






void spawn_particle_in_box(float left, float bottom, float w, float h) {

    Particle& p = P[(int)frand(0.f, (float)MAX_PARTICLES)];
    p.x = frand(left, left + w);
    p.y = frand(bottom, bottom + h);

    float speedScale = 0.5f;
    p.vx = frand(-0.01f, 0.01f) * speedScale;
    p.vy = frand(0.01f, 0.035f) * speedScale;

    float hue = frand(0.f, 1.f);
    float r = fabsf(hue * 6.f - 3.f) - 1.f; if (r < 0) r = 0; if (r > 1) r = 1;
    float g = 2.f - fabsf(hue * 6.f - 2.f); if (g < 0) g = 0; if (g > 1) g = 1;
    float b = 2.f - fabsf(hue * 6.f - 4.f); if (b < 0) b = 0; if (b > 1) b = 1;
    p.r = r; p.g = g; p.b = b;

    p.life = 1.0f;
    p.size = frand(0.006f, 0.012f);

}


void drawParticles() {

	// Particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    
    for (int i = 0; i < MAX_PARTICLES; ++i) {

        Particle& p = P[i];
        if (p.life <= 0.f) continue;

        float a = p.life;

        glColor4f(p.r, p.g, p.b, a);
        
        float s = p.size * (0.8f * p.life + 0.2f);
        float x0 = p.x - s, y0 = p.y - s;
        float x1 = p.x + s, y1 = p.y + s;

        glVertex2f(x0, y0); 
        glVertex2f(x1, y0);
        glVertex2f(x1, y1); 
        glVertex2f(x0, y1);

    }

    glEnd();
    glDisable(GL_BLEND);


    // Text
    glPushMatrix();
    glTranslatef(part_x_ndc, part_y_ndc, 0.f);
    glScalef(scale, scale, 1.f);
    glLineWidth(2.0f);
    glColor3f(0.f, 1.f, 0.f);
    draw_stroke_text(mainMessage);
    glPopMatrix();

}


void updateParticles() {

    int units = stroke_text_width_units(mainMessage);

    scale = part_fontHeightNDC / STROKE_EM_UNITS;
    float textW = (float)units * scale;
    float textH = STROKE_EM_UNITS * scale;

    part_x_ndc += vx; part_y_ndc += vy;

    if (part_x_ndc <= -1.0f) { part_x_ndc = -1.0f;        vx = -vx; }
    if (part_x_ndc + textW >= 1.0f) { part_x_ndc = 1.0f - textW; vx = -vx; }
    if (part_y_ndc <= -1.0f) { part_y_ndc = -1.0f;        vy = -vy; }
    if (part_y_ndc + textH >= 1.0f) { part_y_ndc = 1.0f - textH; vy = -vy; }


    // dt für Partikel
    static double prev = 0.0; 
    if (prev == 0.0) prev = deltaTime;
    float dt = (float)(deltaTime - prev); prev = deltaTime;
    
    float left = part_x_ndc, bottom = part_y_ndc;

    for (int i = 0; i < spawnCount; ++i) spawn_particle_in_box(left, bottom, textW, textH);

    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle& p = P[i];
        if (p.life <= 0.f) continue;
        p.vy += gravity * dt;
        p.x += p.vx;
        p.y += p.vy;
        p.life -= dt * 0.6f;
        if (p.life < 0.f) p.life = 0.f;
    }

}