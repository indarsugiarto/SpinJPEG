#ifndef MSPINJPEGENC_H
#define MSPINJPEGENC_H

/********************** VARIABLES definitions ***********************/
/*--- Generic, SpiNN-related variables ---*/
uint coreID;



/********************** FUNCTION prototypes ***********************/
/*--- Main/helper functions ----*/
typedef enum cond_e
{
  ON_EXIT,             //!< triggered by exit(1)
  ON_ELSE,              //!< not by exit(1)
  ON_FINISH
} cond_t;

void encode(uint arg0, uint arg1);          // encoder main loop
void app_init ();

/*--- Event Handlers ---*/
void hFR (uint key, uint payload);


#endif
