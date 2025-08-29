// FirstDemoG3.cpp — Windows (freeglut) + Mac OS X 10.4 (GLUT)
// Ein-Datei-Version mit Musik (ohne Drittanbieter-Libs). Ein Asset-Name: "music.wav" auf beiden Plattformen.
//  - Windows:   winmm PlaySound (WAV, Loop)
//  - Mac (Tiger): QuickTime C-API (WAV wird unterstützt; Loop via PlayHints)
//  => Eine Quelle, zwei Builds. Keine zusätzlichen Libraries installieren.

#ifdef __APPLE__
#include <GLUT/glut.h>     // Apple GLUT Framework (Tiger)
#else
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>       // vor GL-Headers (für freeglut & winmm)
#endif
#include <GL/freeglut.h>   // FreeGLUT unter Windows
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>

// ======================= Musik: plattformübergreifende Wrapper =======================
static void music_start(const char* path);
static void music_task();
static void music_stop();

#ifdef _WIN32
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
static void music_start(const char* path) {
    // WAV-Datei im selben Ordner wie EXE; Endlosschleife
    PlaySoundA(path, NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
}
static void music_task() {
    // nichts nötig unter Windows
}
static void music_stop() {
    PlaySoundA(NULL, NULL, 0);
}
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
static Movie      gMovie = NULL;
static short      gMovieResRef = 0;

static OSErr posixToFSSpec(const char* path, FSSpec* outSpec) {
    FSRef ref; OSErr err = FSPathMakeRef((const UInt8*)path, &ref, NULL);
    if (err != noErr) return err;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, outSpec, NULL);
#pragma clang diagnostic pop
}
static void music_start(const char* path) {
    OSErr err = noErr; EnterMovies();
    FSSpec spec; err = posixToFSSpec(path, &spec); if (err != noErr) { fprintf(stderr, "FSSpec %d\n", err); return; }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    err = OpenMovieFile(&spec, &gMovieResRef, fsRdPerm);
#pragma clang diagnostic pop
    if (err != noErr) { fprintf(stderr, "OpenMovieFile %d\n", err); return; }
    short resID = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    err = NewMovieFromFile(&gMovie, gMovieResRef, &resID, NULL, newMovieActive, NULL);
#pragma clang diagnostic pop
    if (err != noErr || !gMovie) { fprintf(stderr, "NewMovieFromFile %d\n", err); return; }
    SetMoviePlayHints(gMovie, hintsLoop, hintsLoop); // Loop an
    StartMovie(gMovie);
}
static void music_task() {
    if (gMovie) MoviesTask(gMovie, 0); // pro Frame pumpen
}
static void music_stop() {
    if (gMovie) { StopMovie(gMovie); DisposeMovie(gMovie); gMovie = NULL; }
    if (gMovieResRef) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        CloseMovieFile(gMovieResRef);
#pragma clang diagnostic pop
        gMovieResRef = 0;
    }
    ExitMovies();
}
#endif

// ---------------- Fenstergröße ----------------
static int winW = 640, winH = 480;

// --------- Texte & Stroke-Konfiguration ---------
static const char* introMessage = "My First Demo";
static const char* creditMessage = "from Anon Monk";
static const char* mainMessage = "Happy Birthday Corvus";

static float        fontHeightNDC = 0.10f;   // 0.10 .. 1.00
static const float  STROKE_EM_UNITS = 119.0f;  // GLUT-Stroke „Zeilenhöhe“

// -------------- Phasen ---------------
enum Phase {
    PHASE_INTRO_RISE,   // My First Demo
    PHASE_INTRO_PAUSE,
    PHASE_CREDIT,       // from Anon Monk
    PHASE_RISE,         // Happy Birthday: reinfahren
    PHASE_PAUSE,
    PHASE_BOUNCE        // Happy Birthday: DVD-Bounce
};
static Phase phase = PHASE_INTRO_RISE;

// Text-Baseline (linke untere Ecke) in NDC
static float x_ndc = 0.0f;   // linke Baseline X
static float y_ndc = -1.1f;  // Baseline Y (startet unter Bild)

// Bewegung
static float  vx = 0.008f;  // DVD-Tempo X
static float  vy = 0.006f;  // DVD-Tempo Y
static float  riseSpeed = 0.020f;
static float  pauseTime = 0.8f;
static double reachTime = 0.0;

// ---------------- FPS ----------------
static int    frameCount = 0;
static double lastTime = 0.0;
static double fps = 0.0;

// ---------------- Bounce-Timer ----------------
static double bounceStartTime = 0.0; // merkt sich, wann Bounce beginnt

// ---------- Hilfsfunktionen Pixel <-> NDC ----------
static inline float px_to_ndc_x(float px) { return (px / (float)winW) * 2.0f - 1.0f; }
static inline float px_to_ndc_y(float py) { return (py / (float)winH) * 2.0f - 1.0f; }

// ---------- Stroke-Metriken ----------
static int stroke_text_width_units(const char* s) {
    int w = 0; for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        w += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    return w;
}
static void draw_stroke_text(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
}

// ---------------- Konfetti-Partikel ----------------
struct Particle {
    float x, y;     // Position (NDC)
    float vx, vy;   // Geschwindigkeit
    float life;     // [0..1]
    float r, g, b;  // Farbe
    float size;     // Kantenlänge (NDC)
};
static const int MAX_P = 250;   // G3-freundlich
static Particle P[MAX_P];

static float frand(float a, float b) { return a + (b - a) * (float)rand() / (float)RAND_MAX; }

static void spawn_particle_in_box(float left, float bottom, float w, float h) {
    Particle& p = P[(int)frand(0.f, (float)MAX_P)];
    p.x = frand(left, left + w);
    p.y = frand(bottom, bottom + h);

    float speedScale = (phase == PHASE_BOUNCE) ? 1.0f : 0.5f;
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

static void update_particles(float dt) {
    const float gravity = -0.15f; // NDC/s^2

    // Intro & Credit: kein Konfetti
    int spawnCount = 0;
    if (phase == PHASE_RISE)        spawnCount = 8;
    else if (phase == PHASE_PAUSE)  spawnCount = 8;
    else if (phase == PHASE_BOUNCE) spawnCount = 12;

    // Emitter-Box = aktuell sichtbarer Text
    const char* emsg =
        (phase == PHASE_INTRO_RISE || phase == PHASE_INTRO_PAUSE) ? introMessage :
        (phase == PHASE_CREDIT) ? creditMessage :
        mainMessage;

    int   units = stroke_text_width_units(emsg);
    float scale = fontHeightNDC / STROKE_EM_UNITS;
    float w = (float)units * scale;
    float h = STROKE_EM_UNITS * scale;
    float left = x_ndc, bottom = y_ndc;

    for (int i = 0; i < spawnCount; ++i) spawn_particle_in_box(left, bottom, w, h);

    for (int i = 0; i < MAX_P; ++i) {
        Particle& p = P[i];
        if (p.life <= 0.f) continue;
        p.vy += gravity * dt;
        p.x += p.vx;
        p.y += p.vy;
        p.life -= dt * 0.6f;
        if (p.life < 0.f) p.life = 0.f;
    }
}

static void draw_particles() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    for (int i = 0; i < MAX_P; ++i) {
        Particle& p = P[i];
        if (p.life <= 0.f) continue;
        float a = p.life;
        glColor4f(p.r, p.g, p.b, a);
        float s = p.size * (0.8f * p.life + 0.2f);
        float x0 = p.x - s, y0 = p.y - s;
        float x1 = p.x + s, y1 = p.y + s;
        glVertex2f(x0, y0); glVertex2f(x1, y0);
        glVertex2f(x1, y1); glVertex2f(x0, y1);
    }
    glEnd();
    glDisable(GL_BLEND);
}

// ---------------- Bitmap-Text (für FPS) ----------------
static int bitmap_width_px(const char* s, void* font) {
    int w = 0; for (const unsigned char* p = (const unsigned char*)s; *p; ++p) w += glutBitmapWidth(font, *p);
    return w;
}
static void draw_bitmap_ndc(float x_left_ndc, float y_base_ndc, const char* s, void* font) {
    glRasterPos2f(x_left_ndc, y_base_ndc);
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) glutBitmapCharacter(font, *p);
}

// Vorwärtsdeklaration
static void demo_quit(int code = 0);

// ---------------- GLUT Callbacks ----------------
static void reshape(int w, int h) {
    winW = (w > 0 ? w : 1); winH = (h > 0 ? h : 1);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
}

static void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Hintergrund: schwarz
    glBegin(GL_QUADS);
    glColor3f(0.f, 0.f, 0.f);
    glVertex2f(-1.f, -1.f); glVertex2f(1.f, -1.f);
    glVertex2f(1.f, 1.f); glVertex2f(-1.f, 1.f);
    glEnd();

    // Text-Rendering (Intro / Credit / Birthday)
    const char* currentMessage =
        (phase == PHASE_INTRO_RISE || phase == PHASE_INTRO_PAUSE) ? introMessage :
        (phase == PHASE_CREDIT) ? creditMessage :
        mainMessage;

    int   units = stroke_text_width_units(currentMessage);
    float scale = fontHeightNDC / STROKE_EM_UNITS;
    float textW = (float)units * scale;

    if (phase == PHASE_INTRO_RISE || phase == PHASE_INTRO_PAUSE ||
        phase == PHASE_CREDIT || phase == PHASE_RISE ||
        phase == PHASE_PAUSE) {
        x_ndc = -0.5f * textW; // mittig ausrichten
    }

    if (phase >= PHASE_RISE) draw_particles();

    glPushMatrix();
    glTranslatef(x_ndc, y_ndc, 0.f);
    glScalef(scale, scale, 1.f);
    glLineWidth(2.0f);
    glColor3f(0.f, 1.f, 0.f);
    draw_stroke_text(currentMessage);
    glPopMatrix();

    // FPS oben rechts
    char fpsBuf[32]; std::snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %d", (int)fps);
    int fw_px = bitmap_width_px(fpsBuf, GLUT_BITMAP_9_BY_15);
    float fps_x = px_to_ndc_x((float)winW - (float)fw_px - 5.f);
    float fps_y = px_to_ndc_y((float)winH - 20.f);
    glColor3f(0.f, 1.f, 0.f);
    draw_bitmap_ndc(fps_x, fps_y, fpsBuf, GLUT_BITMAP_9_BY_15);

    glutSwapBuffers();
}

static void timer(int) {
    double t = glutGet(GLUT_ELAPSED_TIME) * 0.001;

    // Musik pumpen (Mac QuickTime)
    music_task();

    // FPS berechnen
    frameCount++;
    if (t - lastTime >= 1.0) { fps = frameCount / (t - lastTime); lastTime = t; frameCount = 0; }

    // dt für Partikel
    static double prev = 0.0; if (prev == 0.0) prev = t;
    float dt = (float)(t - prev); prev = t;

    switch (phase) {
        // Intro: von unten in die Mitte
    case PHASE_INTRO_RISE: {
        float targetY = 0.0f; // Mitte
        if (y_ndc < targetY) { y_ndc += riseSpeed; if (y_ndc > targetY) y_ndc = targetY; }
        if (y_ndc >= targetY) { reachTime = t; phase = PHASE_INTRO_PAUSE; }
        break;
    }

    case PHASE_INTRO_PAUSE:
        if (t - reachTime >= 3.0) {     // Intro-Pause
            phase = PHASE_CREDIT;
            reachTime = t;
            y_ndc = 0.0f;               // Credit steht exakt in der Mitte
        }
        break;

        // Credit 3 s in der Mitte halten
    case PHASE_CREDIT:
        if (t - reachTime >= 3.0) {
            y_ndc = -1.1f;              // danach wieder von unten starten
            phase = PHASE_RISE;
        }
        break;

        // Haupttext: Rise → Pause → Bounce
    case PHASE_RISE: {
        float targetY = -0.1f;          // leicht unter Mitte
        if (y_ndc < targetY) { y_ndc += riseSpeed; if (y_ndc > targetY) y_ndc = targetY; }
        if (y_ndc >= targetY) { reachTime = t; phase = PHASE_PAUSE; }
        break;
    }

    case PHASE_PAUSE:
        if (t - reachTime >= pauseTime) phase = PHASE_BOUNCE;
        break;

    case PHASE_BOUNCE: {
        int   units = stroke_text_width_units(mainMessage);
        float scale = fontHeightNDC / STROKE_EM_UNITS;
        float textW = (float)units * scale;
        float textH = STROKE_EM_UNITS * scale;

        if (bounceStartTime == 0.0) bounceStartTime = t;

        x_ndc += vx; y_ndc += vy;
        if (x_ndc <= -1.0f) { x_ndc = -1.0f;        vx = -vx; }
        if (x_ndc + textW >= 1.0f) { x_ndc = 1.0f - textW; vx = -vx; }
        if (y_ndc <= -1.0f) { y_ndc = -1.0f;        vy = -vy; }
        if (y_ndc + textH >= 1.0f) { y_ndc = 1.0f - textH; vy = -vy; }

        // Nach 20 Sekunden Bounce die Demo beenden
        if (t - bounceStartTime >= 20.0) { demo_quit(0); return; }
        break;
    }
    }

    update_particles(dt);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

static void demo_quit(int code) {
    music_stop();
    std::exit(code);
}

static void key(unsigned char k, int, int) {
#ifndef __APPLE__
    if (k == 'f' || k == 'F') { glutFullScreenToggle(); return; } // freeglut: Vollbild toggeln
#endif
    if (k == 27) demo_quit(0);
}

int main(int argc, char** argv) {
    std::srand((unsigned)std::time(0));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

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
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);
    glutTimerFunc(16, timer, 0);

    lastTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;

    // === Musik starten (einheitlicher Asset-Name) ===
    // Lege eine "music.wav" neben das Binary (Windows & Mac lesen denselben Dateinamen).
    music_start("music.wav");

    glutMainLoop();
    return 0;
}
