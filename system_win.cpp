#include "system_win.h"


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