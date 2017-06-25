/* This is the common header file used by both the worker and the master.
   Inspired by jpeg.h header for all jpeg code from Pierre Guerrier, march 1998     */

#ifndef SPINJPEG_H
#define SPINJPEG_H

/*----------------------------------*/
/* JPEG format parsing markers here */
/*----------------------------------*/

#define SOI_MK	0xFFD8		/* start of image	*/
#define APP_MK	0xFFE0		/* custom, up to FFEF */
#define COM_MK	0xFFFE		/* commment segment	*/
#define SOF_MK	0xFFC0		/* start of frame	*/
#define SOS_MK	0xFFDA		/* start of scan	*/
#define DHT_MK	0xFFC4		/* Huffman table	*/
#define DQT_MK	0xFFDB		/* Quant. table		*/
#define DRI_MK	0xFFDD		/* restart interval	*/
#define EOI_MK	0xFFD9		/* end of image		*/
#define MK_MSK	0xFFF0

#define RST_MK(x)	( (0xFFF8&(x)) == 0xFFD0 )
                        /* is x a restart interval ? */

/*-------------------------------------------------------- */
/* all kinds of macros here				*/
/*-------------------------------------------------------- */

#define first_quad(c)   ((c) >> 4)        /* first 4 bits in file order */
#define second_quad(c)  ((c) & 15)

#define HUFF_ID(hclass, id)       (2 * (hclass) + (id))

#define DC_CLASS        0
#define AC_CLASS        1

/*-------------------------------------------------------*/
/* JPEG data types here					*/
/*-------------------------------------------------------*/

typedef union {		/* block of pixel-space values */
  unsigned char	block[8][8];
  unsigned char	linear[64];
} PBlock;

typedef union {		/* block of frequency-space values */
  int block[8][8];
  int linear[64];
} FBlock;


/* component descriptor structure */

typedef struct {
  unsigned char	CID;	/* component ID */
  unsigned char	IDX;	/* index of first block in MCU */

  unsigned char	HS;	/* sampling factors */
  unsigned char	VS;
  unsigned char	HDIV;	/* sample width ratios */
  unsigned char	VDIV;

  char		QT;	/* QTable index, 2bits 	*/
  char		DC_HT;	/* DC table index, 1bit */
  char		AC_HT;	/* AC table index, 1bit */
  int		PRED;	/* DC predictor value */
} cd_t;


#endif
