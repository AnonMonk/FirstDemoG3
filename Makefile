
EXE := demo

CC := $(TOOLCHAIN)g++

CFLAGS := 
LDFLAGS := -lglut -lGLU -lGL -lm

SRC := ./main.cpp ./system.cpp ./helpers.cpp ./01_text.cpp ./02_particles.cpp ./03_picture.cpp ./04_ball.cpp
OBJ := $(SRC:.c=.o)


all: $(EXE)

$(EXE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f ./demo

.PHONY: all clean
