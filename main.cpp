#include <cstdlib>
#include <cstdio>
#include <math.h>
#include "main.h"
#include "system.h"
#include "helpers.h"
#include "01_text.h"
#include "02_particles.h"
#include "03_picture.h"
#include "04_ball.h"


Phase phase;


void display() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();


    if (phase == PHASE_INTRO_RISE) drawIntroRise();
    if (phase == PHASE_CREDIT) drawIntroCredit();
    if (phase == PHASE_PICTURE) drawPicture();
    if (phase == PHASE_BALL) drawBall();
    if (phase == PHASE_PARTICLES) drawParticles();

    showFPS();

    glutSwapBuffers();
}


void timer(int) {

    music_task();

    updateTimeSystem();


    if (deltaTime < 4) phase = PHASE_INTRO_RISE;
    if ((deltaTime >= 4) && (deltaTime < 8)) phase = PHASE_CREDIT;
    if ((deltaTime >= 8) && (deltaTime < 16)) phase = PHASE_PICTURE;
    if ((deltaTime >= 16) && (deltaTime < 26)) phase = PHASE_BALL;
    if ((deltaTime >= 26) && (deltaTime < 36)) phase = PHASE_PARTICLES;
    if (deltaTime >= 36) demo_quit();



    if (phase == PHASE_INTRO_RISE) updateIntroRise();
    if (phase == PHASE_CREDIT) updateIntroCredit(); 
    if (phase == PHASE_PICTURE) updatePicture();
    if (phase == PHASE_BALL) updateBall();
    if (phase == PHASE_PARTICLES) updateParticles();


    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}


int main(int argc, char** argv) {
    
    resetBall();


    init_system();

    loadImage();         // muss mach init sein, denn es benutzt OpenGL
    
    music_start("./Stuff/music/music.wav");
    startTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    glutMainLoop();

 
    return 0;

}
