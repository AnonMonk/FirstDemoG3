#pragma once

#include "system.h"   // bringt bereits Windows.h + FreeGLUT oder Mac-GLUT rein
#include <cmath>

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
        vx = (rand() % 2 == 0 ? 1.0f : -1.0f) * 150.0f; // zufällige Richtung links/rechts
        vy = 0.0f;
        gravity = -980.0f;
        restitution = 0.95f;//Flummi
        floorY = 60.0f;
        leftX = radius;
        rightX = W - radius;
    }

    void update(float dt) {
        vy += gravity * dt;
        x += vx * dt;
        y += vy * dt;

        if (y - radius < floorY) {
            y = floorY + radius;
            vy = -vy * restitution;

            // Flummi bleibt immer aktiv
            if (std::fabs(vy) < 20.0f) vy = (vy < 0 ? -20.0f : 20.0f);
        }
        if (x < leftX) {
            x = leftX; vx = -vx * restitution;
        }
        else if (x > rightX) {
            x = rightX; vx = -vx * restitution;
        }
    }

    void render() {
        //glClear(GL_COLOR_BUFFER_BIT); //FPS wenn aktiv dan FPS weg

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, W, 0, H, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        drawFloor();
        drawShadow();
        drawBall();

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
    int   W, H;
    float x, y, vx, vy;
    float radius;
    float gravity;
    float restitution;
    float floorY;
    float leftX, rightX;

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
            float a = (float)i / segs * 6.2831853f;
            glVertex2f(x + std::cos(a) * rx, shadowY + std::sin(a) * ry);
        }
        glEnd();
    }

    void drawBall() {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.0f, 0.0f, 1.0f); // Blau
        glVertex2f(x, y);

        const int segs = 64;
        for (int i = 0; i <= segs; ++i) {
            float a = (float)i / segs * 6.2831853f;
            float px = x + std::cos(a) * radius;
            float py = y + std::sin(a) * radius;

            glColor3f(0.0f, 0.0f, 1.0f); // ebenfalls Blau
            glVertex2f(px, py);
        }
        glEnd();

        glPointSize(6.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0f, 0.9f, 0.8f);
        glVertex2f(x - radius * 0.35f, y + radius * 0.35f);
        glEnd();
    }
};
