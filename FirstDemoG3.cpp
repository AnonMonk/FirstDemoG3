#include "system.h"
#include "main.h"

int main(int argc, char** argv) {


	init_system();

	

	music_start("music.wav");
	glutMainLoop();

 
    return 0;
}
