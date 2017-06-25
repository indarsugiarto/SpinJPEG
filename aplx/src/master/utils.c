#include "mSpinJPEG.h"

/* We emulate the C library function int fgetc(FILE *stream) to get the next character (an unsigned char)
 * from the specified stream and advances the position indicator for the stream.
 * Here, stream is a buffer in sdram. It checks the size of JPEG file in sdram and raise EOF error if
 * the last character from the buffers has already been read.
 */
int fgetc(uchar *stream)
{
    uchar c;
    if(nCharRead < szImgFile) {
        sark_mem_cpy((void *)&c, (void *)streamPtr, 1);
        streamPtr++;    // advance one character
        nCharRead++;
        return (int)c;
    }
    else {
        return EOF;
    }
}

/* Returns ceil(N/D). */
int ceil_div(int N, int D)
{
  int i = N/D;
  if (N > D*i) i++;
  return i;
}


/* Returns floor(N/D). */
int floor_div(int N, int D)
{
  int i = N/D;
  if (N < D*i) i--;
  return i;
}


/* For all components reset DC prediction value to 0. */
void reset_prediction(void)
{
  int i;
  for (i=0; i<3; i++) comp[i].PRED = 0;
}
