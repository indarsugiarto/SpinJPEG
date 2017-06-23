#ifndef MSPINJPEG_H
#define MSPINJPEG_H

#include <stdbool.h> 		//introduced in C99 with true equal 1, and false equal 0
#include "spin1_api.h"
#include "../SpinJPEG.h"

/********************** CONST/MACRO definitions ***********************/


/*----------- SDP functionality -----------*/
#define SDP_PORT_JPEG_DATA	1
#define SDP_PORT_JPEG_CMD	2
#define SDP_CMD_INIT_SIZE	1

#define SDP_RECV_CORE_ID	2

/*----------- DMA functionality -----------*/
#define DMA_IMG_BUF_WRITE	1

/*----------- Debugging functionality -----------*/
#define DEBUG_MODE	1

/********************** VARIABLES definitions ***********************/
/*--- Generic variables ---*/
uint coreID;

bool sdramImgBufInitialized;
uchar *sdramImgBuf;
uchar *sdramImgBufPtr;
uint sdramImgBufSize;

/*--- Debugging variables ---*/
uint nReceivedChunk;
uint szImgFile;
static uint dmaAllocErrCntr = 0;


/********************** FUNCTION prototypes ***********************/
/*--- Main/helper functions ----*/
void app_init ();
void resizeImgBuf(uint szFile, uint null);

/*--- Event Handlers ---*/
void hSDP (uint mBox, uint port);
void hDMA (uint tid, uint tag);
#endif
