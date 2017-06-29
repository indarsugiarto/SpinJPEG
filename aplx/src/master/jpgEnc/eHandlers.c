#include "mSpinJPEGenc.h"

/************************************************************************************/
/*-------------------------------- FR EVENT ----------------------------------------*/
void hFR (uint key, uint payload)
{
	switch (key) {
	case FRPL_NEW_RAW_IMG_INFO_KEY:
		wImg = payload >> 16;
		hImg = payload & 0xFFFF;
		break;
    case FRPL_NEW_RAW_IMG_ADDR_KEY:
		sdramImgBuf = (uchar *)payload;
		io_printf(IO_BUF, "Got img %dx%d buffered at 0x%x\n", wImg, hImg, payload);
		spin1_schedule_callback(encode, 0, 0, 1);
		break;
	}
}


void hDMA(uint tid, uint tag)
{
	if(tag==DMA_FETCH_IMG_FOR_SDP_TAG)
		dmaImgFromSDRAMdone = true;
}
