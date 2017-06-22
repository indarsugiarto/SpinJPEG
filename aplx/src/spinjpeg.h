#ifndef SPINJPEG_H
#define SPINJPEG_H

#include <stdbool.h> 		//introduced in C99 with true equal 1, and false equal 0
#include "spin1_api.h"

/********************** CONST/MACRO definitions ***********************/


/*----------- SDP functionality -----------*/
#define SDP_PORT_JPEG_DATA	1
#define SDP_PORT_JPEG_CMD	2
#define SDP_CMD_INIT_SIZE	1

#define SDP_RECV_CORE_ID	2

/*----------- DMA functionality -----------*/
#define DMA_IMG_BUF_WRITE	1


/********************** VARIABLES definitions ***********************/
/*--- Generic variables ---*/
uint coreID;

static bool sdramImgBufInitialized;
static void *sdramImgBuf;
static void *sdramImgBufPtr;
static uint sdramImgBufSize;

/********************** FUNCTION prototypes ***********************/
/*--- Main/helper functions ----*/
extern void app_init ();
extern void resizeImgBuf(uint szFile, uint null);

/*--- Event Handlers ---*/
extern void hSDP (uint mBox, uint port);
#endif
