#include <cmath>
#include <ctime>
#include <cstdio>
#include "system.h"
#include "helpers.h"
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


#endif // _WIN32



#ifdef __linux__


    void music_start(const char* path) {
    }

    void music_task() {
    }

    void music_stop() {
    }


#endif 



#ifdef __APPLE__


    Movie      gMovie = NULL;
    short      gMovieResRef = 0;

    OSErr posixToFSSpec(const char* path, FSSpec* outSpec) {

        FSRef ref; OSErr err = FSPathMakeRef((const UInt8*)path, &ref, NULL);
        
        if (err != noErr) return err;

        return FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, outSpec, NULL);

    }


    void music_start(const char* path) {
         
        EnterMovies();

        FSSpec spec; 
        OSErr err = noErr;

        err = posixToFSSpec(path, &spec); 
        if (err != noErr) { fprintf(stderr, "FSSpec %d\n", err); return; }

        err = OpenMovieFile(&spec, &gMovieResRef, fsRdPerm);
        if (err != noErr) { fprintf(stderr, "OpenMovieFile %d\n", err); return; }

        short resID = 0;
        err = NewMovieFromFile(&gMovie, gMovieResRef, &resID, NULL, newMovieActive, NULL);
        if (err != noErr || !gMovie) { fprintf(stderr, "NewMovieFromFile %d\n", err); return; }

        SetMoviePlayHints(gMovie, hintsLoop, hintsLoop);
        StartMovie(gMovie);

    }

    void music_task() {

        if (gMovie) MoviesTask(gMovie, 0);

    }

    void music_stop() {

        if (gMovie) { 
            StopMovie(gMovie); 
            DisposeMovie(gMovie); 
            gMovie = NULL; 
        }

        if (gMovieResRef) {
            CloseMovieFile(gMovieResRef);
            gMovieResRef = 0;
        }

        ExitMovies();

    }


#endif




void key(unsigned char k, int, int) {

    if (k == 'f' || k == 'F') { glutFullScreenToggle(); }
    if (k == 27) demo_quit();

}


void init_system() {

    std::srand((unsigned)std::time(0));

    int argc = 0;
    char** argv = NULL;
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Bildschirmgröße abfragen und als Fenstergröße setzen
//    int screenW = glutGet(GLUT_SCREEN_WIDTH);
//    int screenH = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(ScreenWidth, ScreenHeight);

    glutCreateWindow("FirstDemoG3");

    glutFullScreen();

    glClearColor(0.0f, 0.0f, 0.18f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutTimerFunc(16, timer, 0);

    lastTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;

}



void demo_quit() {

    music_stop();

    std::exit(0);
    
}
