/*----------------------------------------*/
/* File: spy.c, instrumentation functions */
/*	to monitor activity of JPEG code  */
/* Author: Pierre Guerrier, march 1998	  */
/*----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "jpeg.h"


static int	 loc_code_bits	= 0;	/* catch peak length */
static int	 max_code[2]	= { 1 , 1 };	/* Luma then Chroma */

static long	 code_bits	= 0;	/* huffman codes */
static long	 code_words	= 0;

static long	 coef_bits	= 0;	/* non zero dct coefs */
static long	 coef_words[2]	= { 0 , 0 };

static long	 Luma_bits	= 0;
static long	 Chroma_bits	= 0;


/* number of bits */
/* type: digit or code */

void
trace_bits(int number, int type)
{
  if (MCU_valid[curcomp] > 0)
    Chroma_bits += number;
  else
    Luma_bits += number;

  if (type == 0) {		/* code word */
    code_bits += number;
    loc_code_bits += number;
    if (!number) {		/* done with one code word */
      if (loc_code_bits>max_code[MCU_valid[curcomp]>0])
	max_code[MCU_valid[curcomp]>0] = loc_code_bits;
      loc_code_bits = 0;
      code_words ++;
    }
  }
  else {
    coef_bits += number;
    coef_words[MCU_valid[curcomp]>0]++;
  }
}


/* we know number of Luma and chroma blocks from picture size and format */
/* we know total number of coefficients because of fixed block size */
/* we know number of bits in L and C because of counters */
/* hence the averages */

/* we know number of code words because of counters (mind EOB and ZRL) */


void
output_stats(char *filename)
{
  FILE	*spyfile;
  long	tot_coefs, Lblocks, Cblocks, Tcoef_words;
  int n_tot, n_luma, i;

  n_luma = 0;
  for (i=0; MCU_valid[i] != -1; i++)
    if (MCU_valid[i] == 0)
      n_luma = i+1;
  n_tot = i;
  tot_coefs = 64*mx_size*my_size*n_tot;
  Tcoef_words = coef_words[0] + coef_words[1];
 
  spyfile = fopen(filename, "w");
  fprintf(spyfile, "Traces for %s\n\n\n", filename);

  /* totalizers */
  fprintf(spyfile, "Total code bits\t\t%ld\n", code_bits);
  fprintf(spyfile, "Total code words\t%ld\n", code_words);
  fprintf(spyfile, "Total coeff bits\t%ld\n", coef_bits);
  fprintf(spyfile, "Total NZ coeff words\tLuma %ld\tChroma %ld\n",
	  coef_words[0], coef_words[1]);
  fprintf(spyfile, "\nTotal coeff %ld in %ld blocks (%ld Luma, %ld Chroma)\n",
	  tot_coefs, tot_coefs/64, Lblocks = mx_size*my_size*n_luma,
	  Cblocks = mx_size*my_size*(n_tot-n_luma) );

  /* block averages */
  fprintf(spyfile, "\n\nAverage block bits\tLuma %f\tChroma %f\n",
	  (double)Luma_bits/Lblocks, (double)Chroma_bits/Cblocks);
  fprintf(spyfile, "Average NZ coefs/block\tLuma %f\tChroma %f\n",
	  (double)64*coef_words[0]/(Lblocks * 64),
	  (double)64*coef_words[1]/(Cblocks * 64)	 );

  /* code averages */
  fprintf(spyfile, "Average total bits per NZ coef\t%f\n",
	  (double)(code_bits+coef_bits)/Tcoef_words);
  fprintf(spyfile, "Peak code length\tLuma %d\tChroma %d\n",
	  max_code[0], max_code[1]);
  fprintf(spyfile, "\nAverage code length\t%f\n",
	  (double)code_bits/code_words);
  fprintf(spyfile, "Percent lone words\t%f\n",
	  (double)100*(code_words-Tcoef_words)/code_words);
  fprintf(spyfile, "Average digits (when some)\t%f\n",
	  (double)coef_bits/Tcoef_words);

  fclose(spyfile);
}
