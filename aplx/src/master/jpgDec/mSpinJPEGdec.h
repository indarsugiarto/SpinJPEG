#ifndef MSPINJPEGDEC_H
#define MSPINJPEGDEC_H

#include <stdbool.h> 		//introduced in C99 with true equal 1, and false equal 0
#include "spin1_api.h"
#include "../../SpinJPEG.h"

/********************** CONST/MACRO definitions ***********************/


/*----------- DMA functionality -----------*/
#define DMA_JPG_IMG_BUF_WRITE	1
#define DMA_RAW_IMG_BUF_WRITE   2

/*----------- Pre-defined user events -----------*/
#define UE_START_DECODER    1

/*----------- Debugging functionality -----------*/
#define DEBUG_MODE	1







/********************** VARIABLES definitions ***********************/

/*--- global variables here ---*/

cd_t   comp[3]; /* descriptors for 3 components */

PBlock *MCU_buff[10];  /* decoded component buffer */
                  /* between IDCT and color convert */
int    MCU_valid[10];  /* components of above MCU blocks */

PBlock *QTable[4];     /* three quantization tables */
int    QTvalid[4];     /* at most, but seen as four ... */

/*--- picture attributes ---*/
 int x_size, y_size;	/* Video frame size     */
int rx_size, ry_size;	/* down-rounded Video frame size */
                /* in pixel units, multiple of MCU */
int MCU_sx, MCU_sy;  	/* MCU size in pixels   */
int mx_size, my_size;	/* picture size in units of MCUs */
int n_comp;		/* number of components 1,3 */

/*--- processing cursor variables ---*/
int	in_frame, curcomp;   /* frame started ? current component ? */
int	MCU_row, MCU_column; /* current position in MCU unit */

/*--- RGB buffer storage ---*/
unsigned char *ColorBuffer;   /* MCU after color conversion */
unsigned char *FrameBuffer;   /* complete final RGB image */
PBlock        *PBuff;           /* scratch pixel buffer */
FBlock        *FBuff;           /* scratch frequency buffer */

/*--- process statistics ---*/
int stuffers;	/* number of stuff bytes in file */
int passed;	/* number of bytes skipped looking for markers */

/*--- Generic, SpiNN-related variables ---*/
uint coreID;

/*--- JPG file storage ---*/
bool sdramImgBufInitialized;
uchar *sdramImgBuf;
uchar *sdramImgBufPtr;
uint sdramImgBufSize;

/*--- Debugging variables ---*/
uint nReceivedChunk;
uint szImgFile;
ushort wImg;	// this info needs to be sent from host when the image data is raw
ushort hImg;	// this will be passed on to the encoder
static uint dmaAllocErrCntr = 0;









/********************** FUNCTION prototypes ***********************/
/*--- Main/helper functions ----*/
typedef enum cond_e
{
  ON_EXIT,             //!< triggered by exit(1)
  ON_ELSE,              //!< not by exit(1)
  ON_FINISH
} cond_t;

void decode(uint arg0, uint arg1);          // decoder main loop
void app_init ();
void resizeImgBuf(uint szFile, uint portSrc); // portSrc can be used to distinguish, if it is for JPEG data or Raw data
void aborted_stream(cond_t condition);
void free_structures();

/*--- Event Handlers ---*/
void hSDP (uint mBox, uint port);
void hDMA (uint tid, uint tag);
void hUEvent(uint eventID, uint arg);


/*-----------------------------------------*/
/* prototypes from parse.c		   */
/*-----------------------------------------*/
uint get_next_MK(uchar *fi);
uint get_size(uchar *fi);
int init_MCU(void);
int process_MCU(uchar *fi);
int	load_quant_tables(uchar *fi);
void skip_segment(uchar *fi);
void clear_bits();
uchar get_one_bit(uchar *fi);
unsigned long get_bits(uchar *fi, int number);


/*-----------------------------------------*/
/* prototypes from utils.c		   */
/*-----------------------------------------*/
#define EOF -1
/* imitating std-C fgetc() */
static int nCharRead = 0;    //!< also related to szImgFile
uchar *streamPtr;            //!< also related to sdramImgBuf
int fgetc(uchar *stream);
/*-------------------------*/
int ceil_div(int N, int D);
int floor_div(int N, int D);
void reset_prediction();
int reformat(unsigned long S, int good);
void color_conversion(void);

/*-------------------------------------------*/
/* prototypes from table_vld.c or tree_vld.c */
/*-------------------------------------------*/

int	load_huff_tables(uchar *fi);
uchar get_symbol(uchar *fi, int select);

/*-----------------------------------------*/
/* prototypes from huffman.c 		   */
/*-----------------------------------------*/
/* unpack, predict, dequantize, reorder on store */
void unpack_block(uchar *fi, FBlock *T, int comp);

/*-------------------------------------------*/
/* prototypes from idct.c                    */
/*-------------------------------------------*/

void	IDCT(const FBlock *S, PBlock *T);


#endif
