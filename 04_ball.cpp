#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "system.h"
#include "main.h"
#include "helpers.h"
#include "04_ball.h"

using namespace std;

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = PI * 2;

float x, y, ball_vx, ball_vy;
float radius;
float gravity;
float restitution;
float floorY;
float leftX, rightX;

// Rotation
float rot;   // aktueller Winkel [Grad]
float spin;  // Winkelgeschwindigkeit [Grad/Sek.]


void resetBall() {

    radius = 40.0f;
    x = ScreenWidth * 0.5f;
    y = ScreenHeight * 0.75f;
    ball_vx = (rand() % 2 == 0 ? 1.0f : -1.0f) * 150.0f;
    ball_vy = 0.0f;
    gravity = -980.0f;
    restitution = 0.95f;   // Flummi
    floorY = 60.0f;
    leftX = radius;
    rightX = ScreenWidth - radius;

    rot = 0.0f;                       // Startwinkel
    spin = (ball_vx >= 0.0f ? 80.0f : -80.0f); // Drehrichtung an Anfangstempo koppeln

}


void drawFloor() {

    glColor3f(0.15f, 0.15f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(ScreenWidth, 0);
    glVertex2f(ScreenWidth, floorY); glVertex2f(0, floorY);
    glEnd();
}

void drawShadow() {

    const float shadowY = floorY + 2.0f;
    const float rx = radius * 0.9f;
    const float ry = radius * 0.35f;

    glColor4f(0.0f, 0.0f, 0.0f, 0.35f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, shadowY);
    const int segs = 40;
    for (int i = 0; i <= segs; ++i) {
        float a = (float)i / segs * TWO_PI;
        glVertex2f(x + cos(a) * rx, shadowY + sin(a) * ry);
    }
    glEnd();

}

// ---------- Licht anschalten ----------
void setLights3D(bool on) {
    if (on) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        const float Lpos[4] = { 3.0f, 4.0f, 6.0f, 1.0f };
        const float Ldiff[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        const float Lamb[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        const float Lspec[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, Lpos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, Ldiff);
        glLightfv(GL_LIGHT0, GL_AMBIENT, Lamb);
        glLightfv(GL_LIGHT0, GL_SPECULAR, Lspec);

        const float MatSpec[4] = { 0.95f, 0.95f, 0.95f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, MatSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);
    }
    else {
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);
    }
}

// ---------- Kugel mit Schachbrett ----------
void drawBoingSphere3D(float R, int slices, int stacks) {
    const float dTheta = TWO_PI / slices; // Längengrade
    const float dPhi = PI / stacks; // Breitengrade

    for (int i = 0; i < stacks; ++i) {
        float phi0 = i * dPhi;
        float phi1 = (i + 1) * dPhi;

        for (int j = 0; j < slices; ++j) {

            float theta0 = j * dTheta;
            float theta1 = (j + 1) * dTheta;

          

            // Patch-Farbe (rot/weiß)
            bool red = ((i + j) & 1) != 0;
            if (red) glColor3f(1.0f, 0.12f, 0.12f);
            else     glColor3f(1.0f, 1.0f, 1.0f);


            // vier Eckpunkte (Einheitskugel)
            float x00 = cos(theta0) * sin(phi0);
            float y00 = cos(phi0);
            float z00 = sin(theta0) * sin(phi0);

            float x10 = cos(theta1) * sin(phi0);
            float y10 = cos(phi0);
            float z10 = sin(theta1) * sin(phi0);

            float x11 = cos(theta1) * sin(phi1);
            float y11 = cos(phi1);
            float z11 = sin(theta1) * sin(phi1);

            float x01 = cos(theta0) * sin(phi1);
            float y01 = cos(phi1);
            float z01 = sin(theta0) * sin(phi1);

            // zwei Dreiecke pro Patch
            glBegin(GL_TRIANGLES);
            glNormal3f(x00, y00, z00); glVertex3f(R * x00, R * y00, R * z00);
            glNormal3f(x10, y10, z10); glVertex3f(R * x10, R * y10, R * z10);
            glNormal3f(x11, y11, z11); glVertex3f(R * x11, R * y11, R * z11);

            glNormal3f(x00, y00, z00); glVertex3f(R * x00, R * y00, R * z00);
            glNormal3f(x11, y11, z11); glVertex3f(R * x11, R * y11, R * z11);
            glNormal3f(x01, y01, z01); glVertex3f(R * x01, R * y01, R * z01);
            glEnd();
        }
    }
}


void drawBall() {

	 // PROJECTION sichern und für 2D+Z (Ortho) umschalten
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, ScreenWidth, 0, ScreenHeight, -2000, 2000); // großer Z-Bereich

    // MODELVIEW sichern
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 2D-Boden + Schatten
    drawFloor();
    drawShadow();

    // 3D-Kugel mit Licht
    glEnable(GL_DEPTH_TEST);
    setLights3D(true);

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(-18.0f, 1.0f, 0.0f, 0.0f);     // leichte Schräglage
    glRotatef(rot, 0.2f, 1.0f, 0.0f);   	 // Spin
    drawBoingSphere3D(radius, 16, 8);        // 16×8 = klassischer Look
    glPopMatrix();

    setLights3D(false);
    glDisable(GL_DEPTH_TEST);

    // Zustände wiederherstellen
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glFlush();

}


void updateBall() {

    static double prev = 0.0; 
    if (prev == 0.0) prev = deltaTime;
    float dt = (float)(deltaTime - prev); prev = deltaTime;

    ball_vy += gravity * dt;
    x += ball_vx * dt;
    y += ball_vy * dt;

    // Bodenprall
    if (y - radius < floorY) {
        y = floorY + radius;
        ball_vy = -ball_vy * restitution;
        // Flummi-Effekt: minimale Restenergie
        if (fabs(ball_vy) < 20.0f) ball_vy = (ball_vy < 0 ? -20.0f : 20.0f);
    }

    // Seitenwände + Spin-Richtungswechsel
    if (x < leftX) {
        x = leftX;
        ball_vx = -ball_vx * restitution;
        spin = -spin;                  // <— Drehrichtung umkehren
    }
    else if (x > rightX) {
        x = rightX;
        ball_vx = -ball_vx * restitution;
        spin = -spin;                  // <— Drehrichtung umkehren
    }

    // Rotation fortschreiben
    rot += spin * dt;
    if (rot >= 360.0f) rot -= 360.0f;
    if (rot < 0.0f) rot += 360.0f;

}