#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if(argc != 2) {
	    printf("Please provide .ras as input argument!\n");
	    exit(0);
    }

    char fnamei[256];
    char fnameo[256];
    char *p;
    FILE *fi;
    FILE *fo;

    /* Check for presence of .jpg file extension: */
    if ((p = strrchr(argv[1], '.')) && !strcmp(p, ".ras"))
        /* Indeed such extension; remove it: */
        *p = '\0';
    sprintf(fnamei, "%s.ras", argv[1]);
    sprintf(fnameo, "%s.ra1", argv[1]);

    fi = fopen(fnamei, "rb");
    if (fi == NULL) {
	    printf("Could not open input file %s!\n", fnamei);
	    exit(0);
    }
    fo = fopen(fnameo, "wb");
    if (fo == NULL) {
	    printf("Could not open output file %s!\n", fnameo);
	    fclose(fi);
	    exit(0);
    }
    unsigned int c;
    while ((c = fgetc(fi)) != (unsigned int)EOF) {
	    fputc(c, fo); fgetc(fi); fgetc(fi);
    }
    fclose(fi); fclose(fo);
    printf("Done! The output file is %s\n", fnameo);
    return 0;
}
