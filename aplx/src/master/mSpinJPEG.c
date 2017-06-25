/* spinjpeg.c is the main part that works as a master
 * In this file, the main decoder loop is implemented.
 * It will broadcast messages to workers when an intense computation is
 * found.
 *
 * */
#include "mSpinJPEG.h"

/*--------------------------- MAIN FUNCTION -----------------------------*/
/*-----------------------------------------------------------------------*/
void c_main()
{
    app_init();

    //spin1_callback_on (MCPL_PACKET_RECEIVED, count_packets, -1);
    spin1_callback_on (DMA_TRANSFER_DONE, hDMA, 0);
    spin1_callback_on(USER_EVENT, hUEvent, 2);

	// then go in the event-loop
    spin1_start (SYNC_NOWAIT);
}
/*-----------------------------------------------------------------------*/











/*-------------------------- DECODER MAIN LOOP --------------------------*/
/*-----------------------------------------------------------------------*/
/* Based on jpeg.c, main for jfif decoder by Pierre Guerrier
 * decode() will be called when the first chunk has arrived.
 * See hDMA() and hUEvent()
 */

/*--- Function prototypes ---*/
void get_SOF();
void get_DHT();
void get_DQT();
void get_DRI(int *ri);
void get_SOS(int ri);
void get_EOI();
void decode(uint arg0, uint arg1)
{
    uint aux, mark;
    int restart_interval; /* RST check */
    int i,j;

    // at this point, the sdramImgBuf is already created
    uchar *fi = sdramImgBuf;

    /* First find the SOI marker: */
    aux = get_next_MK(fi);
    if (aux != SOI_MK) aborted_stream(ON_ELSE);
#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Found the SOI marker at %d!\n", nCharRead);
#endif

    in_frame = 0;
    restart_interval = 0;
    for (i = 0; i < 4; i++)
      QTvalid[i] = 0;

    /* Now process segments as they appear: */
    do {
        mark = get_next_MK(fi);
        switch (mark) {
        case SOF_MK: /* start of frame marker */
            get_SOF(); break;
        case DHT_MK: /* Define Huffman Table */
            get_DHT(); break;
        case DQT_MK: /* Define Quantization Table */
            get_DQT(); break;
        case DRI_MK: /* Define Restart Interval */
            get_DRI(&restart_interval); break;
        case SOS_MK:
            get_SOS(restart_interval); break;
        case EOI_MK:
            get_EOI(); break;
        case COM_MK:
#if(DEBUG_MODE>0)
            fprintf(stderr, "[INFO] Skipping comments\n");
#endif
            skip_segment(fi); break;
        case EOF:
#if(DEBUG_MODE>0)
            io_printf(IO_BUF, "[ERROR] Ran out of input data!\n");
#endif
            aborted_stream(ON_ELSE); break;
        default:
            if ((mark & MK_MSK) == APP_MK) {
#if(DEBUG_MODE>0)
                io_printf(IO_BUF, "[INFO] Skipping application data\n");
#endif
                skip_segment(fi); break;
            }
            if (RST_MK(mark)) {
                reset_prediction(); break;
            }
            /* if all else has failed ... */
#if(DEBUG_MODE>0)
            io_printf(IO_BUF, "[WARNING] Lost Sync outside scan, %d!\n", mark);
#endif
            aborted_stream(ON_ELSE); break;
        }
    }
    while (1);

    // TODO: decoding is finish, what now?
    io_printf(IO_STD, "Decoding is done!\n");
}


/*--- Function implementation ---*/
void get_SOF()
{
    uint aux;
#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Found the SOF marker at-%d!\n", nCharRead);
#endif
    in_frame = 1;
    get_size(fi);	/* header size, don't care */
    fgetc(fi);	/* precision, 8bit, don't care */
    y_size = get_size(fi); x_size = get_size(fi); /* Video frame size ??? */
    n_comp = fgetc(fi);	/* # of components */
#if(DEBUG_MODE)
    io_printf(IO_BUFF, "[INFO] Image size is %d by %d\n", x_size, y_size);
    uchar clrStr[12];
    switch (n_comp){
    case 1: io_printf(clrStr, "Monochrome"); break;
    case 3: io_printf(clrStr, "Color"); break;
    default: io_printf(clrstr, "Not a"); break;
    }
    io_printf(IO_BUF, "[INFO] %s JPEG image!\n", clrStr);
#endif

    for (i = 0; i < n_comp; i++) {
        /* component specifiers */
        comp[i].CID = fgetc(fi);
        aux = fgetc(fi);
        comp[i].HS = first_quad(aux);
        comp[i].VS = second_quad(aux);
        comp[i].QT = fgetc(fi);
    }
#if(DEBUG_MODE>0)
    if (n_comp > 1)
        io_printf(IO_BUF, "[INFO] Color format is %d:%d:%d, H=%d\n",
                            comp[0].HS * comp[0].VS,
                            comp[1].HS * comp[1].VS,
                            comp[2].HS * comp[2].VS,
                            comp[1].HS);
#endif
    if (init_MCU() == -1)
        aborted_stream(ON_ELSE);

    /* dimension scan buffer for YUV->RGB conversion */
    uint sz;
    sz = x_size * y_size * n_comp;
    FrameBuffer = (uchar *)sark_xalloc(sv->sdram_heap, sz, 0, ALLOC_LOCK);
    sz = MCU_sx * MCU_sy * n_comp;
    ColorBuffer = (uchar *)sark_xalloc(sv->sdram_heap, sz, 0, ALLOC_LOCK);
    FBuff = (FBlock *) sark_alloc(1, sizeof(FBlock));
    PBuff = (PBlock *) sark_alloc(1, sizeof(PBlock));

    if ((FrameBuffer == NULL) || (ColorBuffer == NULL) ||
        (FBuff == NULL) || (PBuff == NULL) ) {
#if(DEBUG_MODE>0)
            io_printf(IO_BUF, "[ERROR] Could not allocate pixel storage!\n");
#endif
            aborted_stream(ON_EXIT);
    }
}

void get_DHT()
{
#if(DEBUG_MODE)
    io_printf(IO_BUF, "[INFO] Defining Huffman Tables found at-%d\n", nCharRead);
#endif
    if (load_huff_tables(fi) == -1)
        aborted_stream(ON_ELSE);
}

void get_DQT()
{
#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Defining Quantization Tables at-%d\n", nCharRead);
#endif
    if (load_quant_tables(fi) == -1)
        aborted_stream(ON_ELSE);
}

void get_DRI(int *ri)
{
    get_size(fi);	/* skip size */
    *ri = get_size(fi);
#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Defining Restart Interval %d\n", restart_interval);
#endif
}

void get_SOS(int restart_interval)
{
    uint aux;
    int n_restarts, leftover; /* RST check */

#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Found the SOS marker at-%d!\n", nCharRead);
#endif
    get_size(fi); /* don't care */
    aux = fgetc(fi);
    if (aux != (unsigned int) n_comp) {
#if(DEBUG_MODE>0)
        io_printf(IO_BUF, "[ERROR] Bad component interleaving!\n");
#endif
        aborted_stream(ON_ELSE);
    }

    for (i = 0; i < n_comp; i++) {
        aux = fgetc(fi);
        if (aux != comp[i].CID) {
#if(DEBUG_MODE>0)
            io_printf(IO_BUF, "[ERROR] Bad Component Order!\n");
#endif
            aborted_stream(ON_ELSE);
        }
        aux = fgetc(fi);
        comp[i].DC_HT = first_quad(aux);
        comp[i].AC_HT = second_quad(aux);
    }
    get_size(fi); fgetc(fi);	/* skip things */

    MCU_column = 0;
    MCU_row = 0;
    clear_bits();
    reset_prediction();

    /* main MCU processing loop here */
    if (restart_interval) {
        n_restarts = ceil_div(mx_size * my_size, restart_interval) - 1;
        leftover = mx_size * my_size - n_restarts * restart_interval;
        /* final interval may be incomplete */

        for (i = 0; i < n_restarts; i++) {
            for (j = 0; j < restart_interval; j++)
                process_MCU(fi);
            /* proc till all EOB met */
            aux = get_next_MK(fi);
            if (!RST_MK(aux)) {
#if(DEBUG_MODE>0)
                io_printf(IO_BUF, "[ERROR] Lost Sync after interval!\n");
#endif
                aborted_stream(ON_ELSE);
            }
#if(DEBUG_MODE>0)
            else io_printf(IO_BUF, "[INFO] Found Restart Marker at-%d\n", nCharRead);
#endif

            reset_prediction();
            clear_bits();
        }		/* intra-interval loop */
    }
    else
        leftover = mx_size * my_size;

      /* process till end of row without restarts */
    for (i = 0; i < leftover; i++)
        process_MCU(fi);

    in_frame = 0;
}

void get_EOI()
{
#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Found the EOI marker at-%d!\n", nCharRead);
#endif
    if (in_frame) aborted_stream(ON_ELSE);

#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Total skipped bytes %d, total stuffers %d\n", passed, stuffers);
#endif
    aborted_stream(ON_FINISH);
}










/*--------------------------- HELPER FUNCTION -----------------------------*/
/*-------------------------------------------------------------------------*/

void app_init ()
{
    coreID = spin1_get_core_id ();
    if(coreID == SDP_RECV_CORE_ID) {
        spin1_callback_on (SDP_PACKET_RX, hSDP, -1);
    }
    sdramImgBufInitialized = false;
}

/* Round up to the next highest power of 2 
 * http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
uint roundUp(uint v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/* resizeImgBuf() is called when host-PC trigger an SDP with SDP_CMD_INIT_SIZE
 *
 */
void resizeImgBuf(uint szFile, uint null)
{
    bool createNew;
    if(sdramImgBufInitialized) {
        if(szFile > sdramImgBufSize) {
           sark_xfree(sv->sdram_heap, sdramImgBuf, ALLOC_LOCK);
           createNew = true;
        } else {
           // nothing to change, just
           // reset the image buffer pointer to the starting position
           sdramImgBufPtr = sdramImgBuf;
           createNew = false;
#if(DEBUG_MODE > 0)
           io_printf(IO_STD, "No need to create a new sdram buffer\n");
           io_printf(IO_STD, "sdramImgBufPtr is reset to 0x%x\n", sdramImgBufPtr);
#endif
        }
    } else {
        createNew = true;
    }

    if(createNew) {
        /* From sark.h regarding sark_xalloc:
           Returns NULL on failure (not enough memory, bad tag or tag in use).

           The flag parameter contains two flags in the bottom two bits. If bit 0
           is set (ALLOC_LOCK), the heap manipulation is done behind a lock with
           interrupts disabled. If bit 1 is set (ALLOC_ID), the block is tagged
           with the AppID provided in bits 15:8 of the flag, otherwise the AppID
           of the current application is used.

           The 8-bit "tag" is combined with the AppID and stored in the "free"
           field while the block is allocated. If the "tag" is non-zero, the
           block is only allocated if the tag is not in use.  The tag (and AppID)
           is stored in the "alloc_tag" table while the block is allocated.

           Hence, flag is set only to ALLOC_LOC: behind lock
                  tag is set to 0, to make sure it is always allocated (using AppID) 
        */
        sdramImgBufSize = roundUp(szFile);

#if(DEBUG_MODE > 0)
        io_printf(IO_STD, "Allocating %d-bytes of sdram buffer\n", sdramImgBufSize);
#endif

        sdramImgBuf = sark_xalloc (sv->sdram_heap, szFile, 0, ALLOC_LOCK);
        if(sdramImgBuf != NULL) {
            sdramImgBufInitialized = true;
            sdramImgBufPtr = sdramImgBuf;
#if(DEBUG_MODE > 0)
            io_printf(IO_BUF, "Sdram buffer is allocated at 0x%x with ptr 0x%x\n", sdramImgBuf, sdramImgBufPtr);
#endif
        } else {
            io_printf(IO_BUF, "[ERR] Fail to create sdramImgBuf!\n");
            // dangerous: terminate then!
            rt_error (RTE_MALLOC);
            sdramImgBufInitialized = false;
        }
    }

    // additional function for fgetc()
    streamPtr = sdramImgBuf;
}

/* If fails to decode the JPEG file in sdram buffer, abort the operation gracefully */
void aborted_stream(cond_t condition)
{
    io_printf(IO_BUF, "[ERROR] Abnormal end of decompression process!\n");
#if(DEBUG_MODE>0)
    io_printf(IO_BUF, "[INFO] Total skipped bytes %d, total stuffers %d\n", passed, stuffers);
#endif
    if(condition==ON_ELSE) {
        free_structures(); // 24 Juni: belum selesai
    } else if(condition==ON_EXIT) {
        // TODO: if called by exit(1)
    } else if(condition==ON_FINISH) {
        // TODO: when EOI is encounterd
        /*
        fclose(fi);
        RGB_save(fo);
        fclose(fo);
        free_structures();
  #ifdef SPY
        output_stats(fnam);
  #endif
        fprintf(stderr, "\nDone.\n");
        exit(0);
        */
    }
}

void free_structures()
{
    // TODO: 24 Juni, jam 00:04 baru sampai sini
    /*
    int i;

    for (i=0; i<4; i++) if (QTvalid[i]) free(QTable[i]);

    free(ColorBuffer);
    free(FrameBuffer);

    for (i=0; MCU_valid[i] != -1; i++) free(MCU_buff[i]);
    */
}
