/* Indar:
 *    Modifikasi dari main.c untuk membaca raw data dari .ra1
 *    */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "jpec.h"

/* Global variables */
const char *pname;

/* Function prototypes */
int main(int argc, char **argv);
//uint8_t *load_image(const char *path, int *width, int *height);
unsigned char *load_image(const char *fImg, int w, int h);

/* implementation */
int main(int argc, char** argv) {
  pname = argv[0];
  if (argc == 4) {
    int w, h;
    w = atoi(argv[2]);
    h = atoi(argv[3]);
    printf("fname = %s, width = %d, heigth = %d\n", argv[1], w, h);
    unsigned char *img = load_image(argv[1], w, h);
    jpec_enc_t *e = jpec_enc_new(img, w, h);
    int len;
    const uint8_t *jpeg = jpec_enc_run(e, &len);
    FILE *file = fopen("result.jpg", "wb");
    fwrite(jpeg, sizeof(uint8_t), len, file);
    fclose(file);
    printf("Done: result.jpg (%d bytes)\n", len);
    jpec_enc_del(e);
    free(img);
  }
  else {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s raw_image_file width height\n", pname);
    exit(1);
  }
  return 0;
}

unsigned char *load_image(const char *fName, int w, int h)
{
    unsigned char *data = (unsigned char*)malloc(w*h*sizeof(unsigned char));
    FILE *fImg = fopen(fName, "rb");
    fread(data, w*h, 1, fImg);
    fclose(fImg);
    return data;
}


