#include "mSpinJPEG.h"
/*----------------------------------------------------------------*/
/* find next marker of any type, returns it, positions just after */
/* EOF instead of marker if end of file met while searching ...	  */
/*----------------------------------------------------------------*/

uint get_next_MK(uchar *fi)
{
    unsigned int c;
    int ffmet = 0;
    int locpassed = -1;

    passed--;	/* as we fetch one anyway */

    while ((c = fgetc(fi)) != (unsigned int) EOF) {
        switch (c) {
        case 0xFF:
            ffmet = 1;
            break;
        case 0x00:
            ffmet = 0;
            break;
        default:
            if (locpassed > 1)
                io_printf(IO_BUF, "[NOTE] passed %d bytes\n", locpassed);
            if (ffmet)
                return (0xFF00 | c);
            ffmet = 0;
            break;
        }
        locpassed++;
        passed++;
    }

    return (unsigned int) EOF;
}

uint get_size(uchar *fi)
{
    uchar aux;

    aux = (uchar)fgetc(fi);
    return (aux << 8) | fgetc(fi);	/* big endian */
}


/*----------------------------------------------------------*/
/* initialise MCU block descriptors	                    */
/*----------------------------------------------------------*/

int init_MCU(void)
{
    int i, j, k, n, hmax = 0, vmax = 0;

    for (i = 0; i < 10; i++)
        MCU_valid[i] = -1;

    k = 0;

    for (i = 0; i < n_comp; i++) {
        if (comp[i].HS > hmax)
            hmax = comp[i].HS;
        if (comp[i].VS > vmax)
            vmax = comp[i].VS;
        n = comp[i].HS * comp[i].VS;

        comp[i].IDX = k;
        for (j = 0; j < n; j++) {
            MCU_valid[k] = i;
            MCU_buff[k] = (PBlock *) sark_alloc(1, sizeof(PBlock));
            if (MCU_buff[k] == NULL) {
#if(DEBUG_MODE>0)
                io_printf(IO_BUF, "[ERROR] Could not allocate MCU buffers!\n");
#endif
                return -1;
            }
            k++;
            if (k == 10) {
#if(DEBUG_MODE>0)
                io_printf(IO_BUF, "[ERROR] Max subsampling exceeded!\n");
#endif
                return -1;
            }
        }
    }

    MCU_sx = 8 * hmax;
    MCU_sy = 8 * vmax;
    for (i = 0; i < n_comp; i++) {
        comp[i].HDIV = (hmax / comp[i].HS > 1);	/* if 1 shift by 0 */
        comp[i].VDIV = (vmax / comp[i].VS > 1);	/* if 2 shift by one */
    }

    mx_size = ceil_div(x_size,MCU_sx);
    my_size = ceil_div(y_size,MCU_sy);
    rx_size = MCU_sx * floor_div(x_size,MCU_sx);
    ry_size = MCU_sy * floor_div(y_size,MCU_sy);

    return 0;
}


/*----------------------------------------------------------*/
/* loading and allocating of quantization table             */
/* table elements are in ZZ order (same as unpack output)   */
/*----------------------------------------------------------*/

int load_quant_tables(uchar *fi)
{
  char aux;
  unsigned int size, n, i, id, x;

  size = get_size(fi); /* this is the tables' size */
  n = (size - 2) / 65;

  for (i = 0; i < n; i++) {
    aux = fgetc(fi);
    if (first_quad(aux) > 0) {
#if(DEBUG_MODE>0)
      io_printf(IO_BUF, "[ERROR] Bad QTable precision!\n");
#endif
      return -1;
    }
    id = second_quad(aux);
#if(DEBUG_MODE>0)
      io_printf(IO_BUF, "[INFO] Loading table %d\n", id);
#endif
    QTable[id] = (PBlock *) sark_alloc(1, sizeof(PBlock));
    if (QTable[id] == NULL) {
#if(DEBUG_MODE>0)
      io_printf(IO_BUF, "[ERROR] Could not allocate table storage!\n");
#endif
      aborted_stream(ON_EXIT);
    }
    QTvalid[id] = 1;
    for (x = 0; x < 64; x++)
      QTable[id]->linear[x] = fgetc(fi);
      /*
         -- This is useful to print out the table content --
         for (x = 0; x < 64; x++)
         fprintf(stderr, "%d\n", QTable[id]->linear[x]);
      */
  }
  return 0;
}


void skip_segment(uchar *fi)	/* skip a segment we don't want */
{
  unsigned int size;
  char	tag[5];
  int i;

  size = get_size(fi);
  if (size > 5) {
    for (i = 0; i < 4; i++)
      tag[i] = fgetc(fi);
    tag[4] = '\0';
#if(DEBUG_MODE>0)
      io_printf(IO_BUF, "[INFO] Tag is %s\n", tag);
#endif
    size -= 4;
  }
  //fseek(fi, size-2, SEEK_CUR);
  size -= 2;
  streamPtr += size;
  nCharRead += size;

}
