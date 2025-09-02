#include "system_g3.h"



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