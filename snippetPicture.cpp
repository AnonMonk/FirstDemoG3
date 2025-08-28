// FirstDemoG3.cpp — Windows (freeglut) + Mac OS X 10.4 (GLUT) — Ein-Datei-Version

#ifdef __APPLE__
#include <GLUT/glut.h>     // Apple GLUT Framework (Tiger)
#else
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>     // vor GL-Headers
#endif
#include <GL/freeglut.h>   // FreeGLUT unter Windows
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <vector>

// ---------------- Fenstergröße ----------------
static int winW = 640, winH = 480;

// --------- Texte & Stroke-Konfiguration ---------
static const char* introMessage = "My First Demo";
static const char* creditMessage = "from Anon Monk";
static const char* mainMessage = "Happy Birthday Corvus";

static float fontHeightNDC = 0.10f;
static const float STROKE_EM_UNITS = 119.0f;

// -------------- Phasen ---------------
enum Phase {
    PHASE_INTRO_RISE, //My First Demo
    PHASE_INTRO_PAUSE,
    PHASE_CREDIT,     // Anon Monk
    PHASE_IMAGE,      // Bild
    PHASE_RISE,       // Rise HBDC
    PHASE_PAUSE,
    PHASE_BOUNCE      // Bounce HBDC
};
static Phase phase = PHASE_INTRO_RISE;

// Text-Baseline (linke untere Ecke) in NDC
static float x_ndc = 0.0f;
static float y_ndc = -1.1f;

// Bewegung
static float vx = 0.008f;
static float vy = 0.006f;
static float riseSpeed = 0.020f;
static float pauseTime = 0.8f;
static double reachTime = 0.0;

/*
static void draw_particles();
static int bitmap_width_px(const char* s, void* font);
static void draw_bitmap_ndc(float x_left_ndc, float y_base_ndc, const char* s, void* font);
*/
// ---------------- FPS ----------------
static int    frameCount = 0;
static double lastTime = 0.0;
static double fps = 0.0;

// ---------------- Szene-Bild ----------------
GLuint sceneTex = 0;
static const char* scenePath = "images/image1.bmp";     // TODO:   file name im selben verzeichniss BITTE
static double imageStartTime = 0.0;
static double imageDuration = 5.0;

// ---------------- Bounce-Timer ----------------
static double bounceStartTime = 0.0;

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

// ---------------- BMP-Loader ----------------
GLuint loadBMP(const char* filename) {


    //  TODO:  check if file exist


    FILE* f = nullptr;
#ifdef _WIN32
    if (fopen_s(&f, filename, "rb") != 0 || !f) return 0;
#else
    f = fopen(filename, "rb");
    if (!f) return 0;
#endif

    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) { fclose(f); return 0; }
    if (header[0] != 'B' || header[1] != 'M') { fclose(f); return 0; }

    int dataOffset = *(int*)&header[10];
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    short bpp = *(short*)&header[28];
    int compression = *(int*)&header[30];

    if (bpp != 24 || compression != 0 || width <= 0 || height == 0) { fclose(f); return 0; }

    int bytesPerPixel = 3;
    int strideRaw = width * bytesPerPixel;
    int rowPadding = (4 - (strideRaw % 4)) & 3;
    int strideFile = strideRaw + rowPadding;

    fseek(f, dataOffset, SEEK_SET);

    std::vector<unsigned char> fileBuf((size_t)strideFile * (size_t)std::abs(height));
    if (fread(fileBuf.data(), 1, fileBuf.size(), f) != fileBuf.size()) { fclose(f); return 0; }
    fclose(f);

    std::vector<unsigned char> rgb((size_t)strideRaw * (size_t)std::abs(height));
    bool bottomUp = (height > 0);
    int absH = std::abs(height);

    for (int y = 0; y < absH; ++y) {
        const unsigned char* srcRow = fileBuf.data() + (size_t)y * (size_t)strideFile;
        int dstY = bottomUp ? (absH - 1 - y) : y;
        unsigned char* dstRow = rgb.data() + (size_t)dstY * (size_t)strideRaw;
        for (int x = 0; x < width; ++x) {
            dstRow[x * 3 + 0] = srcRow[x * 3 + 2]; // R
            dstRow[x * 3 + 1] = srcRow[x * 3 + 1]; // G
            dstRow[x * 3 + 2] = srcRow[x * 3 + 0]; // B
        }
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
#ifdef __APPLE__
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, absH, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);




    return tex;
}

// ---------------- Konfetti-Partikel ----------------
// (dein alter Partikel-Code bleibt gleich)

// Fügt die fehlende Funktionsdefinition hinzu
static int bitmap_width_px(const char* s, void* font) {
    int w = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        w += glutBitmapWidth(font, *p);
    return w;
}

// ---------------- GLUT Callbacks ----------------
static void reshape(int w, int h) {
    winW = (w > 0 ? w : 1); winH = (h > 0 ? h : 1);
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
}

static void display() {
    glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    /*
    // Hintergrund: schwarz
    glBegin(GL_QUADS);
    glColor3f(0.f, 0.f, 0.f);
    glVertex2f(-1.f, -1.f); glVertex2f(1.f, -1.f);
    glVertex2f(1.f, 1.f); glVertex2f(-1.f, 1.f);
    glEnd();
    */
    // Bildszene
//    if (phase == PHASE_IMAGE) {








        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, sceneTex);
        glColor3f(1.f, 1.f, 1.f);
   

        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(-1.f, -1.f);
        glTexCoord2f(1.f, 0.f); glVertex2f(1.f, -1.f);
        glTexCoord2f(1.f, 1.f); glVertex2f(1.f, 1.f);
        glTexCoord2f(0.f, 1.f); glVertex2f(-1.f, 1.f);
        glEnd();















    
//        glutSwapBuffers();
//        return;
//    }
        /*
    // Text-Rendering (Intro, Credit, Birthday)
    const char* currentMessage =
        (phase == PHASE_INTRO_RISE || phase == PHASE_INTRO_PAUSE) ? introMessage :
        (phase == PHASE_CREDIT) ? creditMessage : mainMessage;

    int units = stroke_text_width_units(currentMessage);
    float scale = fontHeightNDC / STROKE_EM_UNITS;
    float textW = (float)units * scale;

    if (phase == PHASE_INTRO_RISE || phase == PHASE_INTRO_PAUSE ||
        phase == PHASE_CREDIT || phase == PHASE_RISE || phase == PHASE_PAUSE)
        x_ndc = -0.5f * textW;

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
    */
    glutSwapBuffers();
}

// Korrigierte timer-Funktion mit switch-Anweisung
static void timer(int) {
    double t = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    // … dein Timer-Code …
    switch (phase) {
        // ... andere Phasen ...
        case PHASE_BOUNCE: {
            // … Bewegung …
            if (bounceStartTime == 0.0) bounceStartTime = t;
            if (t - bounceStartTime >= 20.0) std::exit(0); // nach 20 Sek. beenden
            break;
        }
        // ... ggf. weitere cases ...
    }
}


int main(int argc, char** argv) {
    std::srand((unsigned)std::time(0));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_FLOAT | GLUT_RGB);

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
  //  glutKeyboardFunc(key);
    glutTimerFunc(16, timer, 0);

    lastTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;

    // Szene-Bild laden
    sceneTex = loadBMP("image1.bmp");

    glutMainLoop();
    return 0;
}