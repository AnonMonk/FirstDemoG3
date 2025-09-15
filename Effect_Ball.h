#pragma once

#include "system.h"   // bringt FreeGLUT/GLUT + Windows inkl. OpenGL
#include <cmath>
#include <cstdlib>    // std::rand

class Effect_Ball {
public:
    Effect_Ball(int width = 800, int height = 600)
        : W(width), H(height) {
        reset();
    }

    void reset() {
        radius = 40.0f;
        x = W * 0.5f;
        y = H * 0.75f;
        vx = (std::rand() % 2 == 0 ? 1.0f : -1.0f) * 150.0f;
        vy = 0.0f;
        gravity = -980.0f;
        restitution = 0.95f;   // Flummi
        floorY = 60.0f;
        leftX = radius;
        rightX = W - radius;

        rot = 0.0f;                       // Startwinkel
        spin = (vx >= 0.0f ? 80.0f : -80.0f); // Drehrichtung an Anfangstempo koppeln
    }

    void update(float dt) {
        vy += gravity * dt;
        x += vx * dt;
        y += vy * dt;

        // Bodenprall
        if (y - radius < floorY) {
            y = floorY + radius;
            vy = -vy * restitution;
            // Flummi-Effekt: minimale Restenergie
            if (std::fabs(vy) < 20.0f) vy = (vy < 0 ? -20.0f : 20.0f);
        }

        // Seitenwände + Spin-Richtungswechsel
        if (x < leftX) {
            x = leftX;
            vx = -vx * restitution;
            spin = -spin;                  // <— Drehrichtung umkehren
        }
        else if (x > rightX) {
            x = rightX;
            vx = -vx * restitution;
            spin = -spin;                  // <— Drehrichtung umkehren
        }

        // Rotation fortschreiben
        rot += spin * dt;
        if (rot >= 360.0f) rot -= 360.0f;
        if (rot < 0.0f) rot += 360.0f;
    }

    void render() {
        // PROJECTION sichern und für 2D+Z (Ortho) umschalten
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, W, 0, H, -2000, 2000); // großer Z-Bereich

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
        glRotatef(rot, 0.2f, 1.0f, 0.0f);    // Spin
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

    void onKey(unsigned char key) {
        switch (key) {
        case 'r': case 'R': reset(); break;
        case '+': restitution = std::fmin(0.99f, restitution + 0.05f); break;
        case '-': restitution = std::fmax(0.0f, restitution - 0.05f); break;
        case ' ': vy += 400.0f; break;
        }
    }

private:
    // Portabel statt M_PI
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 6.28318530717958647692f;

    int   W, H;
    float x, y, vx, vy;
    float radius;
    float gravity;
    float restitution;
    float floorY;
    float leftX, rightX;

    // Rotation
    float rot;   // aktueller Winkel [Grad]
    float spin;  // Winkelgeschwindigkeit [Grad/Sek.]

    // ---------- 2D-Untergrund ----------
    void drawFloor() {
        glColor3f(0.15f, 0.15f, 0.18f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(W, 0);
        glVertex2f(W, floorY); glVertex2f(0, floorY);
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
            glVertex2f(x + std::cos(a) * rx, shadowY + std::sin(a) * ry);
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

                /*

                // Patch-Farbe (rot/weiß)
                bool red = ((i + j) & 1) != 0;
                if (red) glColor3f(1.0f, 0.12f, 0.12f);
                else     glColor3f(1.0f, 1.0f, 1.0f);

                */

              // /*

                // Patch-Farbe (blau/weiß)
                bool blue = ((i + j) & 1) != 0;
                if (blue) glColor3f(0.12f, 0.12f, 1.0f);   // Blau
                else      glColor3f(1.0f, 1.0f, 1.0f);     // Weiß
                
                //
                // */

                // vier Eckpunkte (Einheitskugel)
                float x00 = std::cos(theta0) * std::sin(phi0);
                float y00 = std::cos(phi0);
                float z00 = std::sin(theta0) * std::sin(phi0);

                float x10 = std::cos(theta1) * std::sin(phi0);
                float y10 = std::cos(phi0);
                float z10 = std::sin(theta1) * std::sin(phi0);

                float x11 = std::cos(theta1) * std::sin(phi1);
                float y11 = std::cos(phi1);
                float z11 = std::sin(theta1) * std::sin(phi1);

                float x01 = std::cos(theta0) * std::sin(phi1);
                float y01 = std::cos(phi1);
                float z01 = std::sin(theta0) * std::sin(phi1);

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
};
