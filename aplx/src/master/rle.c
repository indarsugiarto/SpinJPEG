
#include "mSpinJPEG.h"

typedef struct {
    uint startAddr;
    uint endAddr;
} img_region_t;

/* raster() will:
 * 1. encode the raw image data in sdram buffer imgIn
 *    first, it creates a new sdram buffer imgOut based on the given szImg
 *    but, how big is szImg??? how does lzss work?
 * 2. send the encoded image as chunks via SDP
 * NOTE: to speed up, this raster() should be implemented on several cores.
 *       In this case, imgIn is a region
 * */
void raster(uint imgAddr, uint regionInfo)
{

    infile = (uchar *)imgIn;
    outfile = sark_xalloc(sv->sdram_heap, szImg, 0, ALLOC_LOCK);
    encode();
}
