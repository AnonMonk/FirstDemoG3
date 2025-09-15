#include <cmath>
#include <ctime>
#include "system.h"
#include "main.h"

#ifdef _WIN32

void music_start(const char* path) {
    // WAV-Datei im selben Ordner wie EXE; Endlosschleife
    PlaySoundA(path, NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
}

void music_task() {
    // nichts nötig unter Windows
}

void music_stop() {
    PlaySoundA(NULL, NULL, 0);
}

static void key(unsigned char k, int, int) {
#ifndef __APPLE__
    if (k == 'f' || k == 'F') { glutFullScreenToggle(); return; } // freeglut: Vollbild toggeln
#endif
    if (k == 27) demo_quit();
}

void init_system() {

    std::srand((unsigned)std::time(0));

    int argc = 0;
    char** argv = NULL;
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Bildschirmgröße abfragen und als Fenstergröße setzen
    int screenW = glutGet(GLUT_SCREEN_WIDTH);
    int screenH = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(screenW, screenH);

#ifdef __APPLE__
    glutCreateWindow("FirstDemoG3 (Tiger/GLUT)");
#else
    glutCreateWindow("FirstDemoG3 (FreeGLUT)");
#endif

    // Vollbild aktivieren
    glutFullScreen();

    glClearColor(0.0f, 0.0f, 0.18f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutTimerFunc(16, timer, 0);

    // --- Fenster-Schließen-Callback anmelden (freeglut) ---
#if defined(FREEGLUT) || defined(FREEGLUT_VERSION)
    glutCloseFunc([]() { demo_quit(); });
#endif

    lastTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;
}

#endif // _WIN32

void demo_quit() {
    music_stop();
    std::exit(0);
}
