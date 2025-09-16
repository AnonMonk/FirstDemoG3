#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "system.h"
#include "main.h"
#include "helpers.h"
#include "03_picture.h"

void *pictureData;
int pictureWidth, pictureHeight;
unsigned int textureID;



#pragma pack(push,1)   // make sure struct is packed
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} MY_BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} MY_BITMAPINFOHEADER;
#pragma pack(pop)

unsigned char *loadBMP(const char *filename,
                       int *width, int *height)
{
    FILE* f;
#ifdef _WIN32
    fopen_s(&f, filename, "rb");
#else
    FILE *f = fopen(filename, "rb");
#endif
    if (!f) { perror("fopen"); return NULL; }

    MY_BITMAPFILEHEADER fileHdr;
    fread(&fileHdr, sizeof(fileHdr), 1, f);
    if (fileHdr.bfType != 0x4D42) { // 'BM'
        fprintf(stderr, "Not a BMP file\n");
        fclose(f);
        return NULL;
    }

    MY_BITMAPINFOHEADER infoHdr;
    fread(&infoHdr, sizeof(infoHdr), 1, f);

    if (infoHdr.biBitCount != 24 || infoHdr.biCompression != 0) {
        fprintf(stderr, "Only uncompressed 24-bit BMP supported\n");
        fclose(f);
        return NULL;
    }

    *width  = infoHdr.biWidth;
    *height = infoHdr.biHeight;

    // Each row is padded to 4-byte boundary
    size_t rowSize = ((*width * 3 + 3) / 4) * 4;
    size_t dataSize = rowSize * (*height);

    unsigned char *data = (unsigned char *)malloc(dataSize);
    if (!data) { perror("malloc"); fclose(f); return NULL; }

    fseek(f, fileHdr.bfOffBits, SEEK_SET);
    fread(data, 1, dataSize, f);
    fclose(f);

    return data; // BGR order, bottom-to-top rows
}


void loadImage() {

	pictureData = loadBMP("./Stuff/images/image1.bmp", &pictureWidth, &pictureHeight);


	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pictureWidth, pictureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pictureData);

	free(pictureData);

}


void drawPicture() {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);

    //theoretisch achwarze balken hacken pic größe 
	glBegin(GL_QUADS);
	    glTexCoord2f(0,0); glVertex2f(-1,-1);
	    glTexCoord2f(1,0); glVertex2f( 1,-1);
	    glTexCoord2f(1,1); glVertex2f( 1, 1);
	    glTexCoord2f(0,1); glVertex2f(-1, 1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

}


void updatePicture() {

}