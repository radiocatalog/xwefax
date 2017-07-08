/*  dft.c
 *
 *  DFT functions of xwefax application
 */

/*
 *  xwefax: An application to decode Radio WEFAX signals from
 *  a Radio Receiver, through the computer's sound card.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

#include "dft.h"

/* Scaled-up, integer, sin/cos tables */
static int
  *isin = NULL,
  *icos = NULL;

/*------------------------------------------------------------------------*/

/* Idft_Init()
 *
 * Initializes Idft()
 */
  void
Idft_Init( int dft_input_size, int dft_bin_size )
{
  int i;
  size_t mreq;
  double w, dw;

  /* Allocate dft buffers */
  mreq = sizeof(int) * (size_t)dft_input_size;
  if( !mem_alloc((void **)&dft_in_r, mreq) ) return;
  if( !mem_alloc((void **)&isin, mreq) ) return;
  if( !mem_alloc((void **)&icos, mreq) ) return;
  memset( (void *)dft_in_r, 0, mreq );

  mreq = sizeof(int) * (size_t)dft_bin_size;
  if( !mem_alloc((void **)&dft_out_r, mreq) ) return;
  if( !mem_alloc((void **)&dft_out_i, mreq) ) return;

  /* Make sin/cos tables */
  dw = M_2PI / (double)dft_input_size;
  for( i = 0; i < dft_input_size; i++ )
  {
	w = dw * (double)i;
	isin[i] = (int)(127.0 * sin(w) + 0.5);
	icos[i] = (int)(127.0 * cos(w) + 0.5);
  }

} /* Idft_Init() */

/*------------------------------------------------------------------------*/

/* Idft()
 *
 * Simple, integer-only, DFT function
 */
  void
Idft( int dft_input_size, int dft_bin_size )
{
  int i, j, w;

  /* In-phase and quadrature summation */
  int sum_i, sum_q;

  /* Calculate output bins */
  for( i = 0; i < dft_bin_size; i++ )
  {
	sum_i = sum_q = w = 0;

	/* Summate input values */
	for( j = 0; j < dft_input_size; j++ )
	{
	  sum_i += dft_in_r[j] * isin[w];
	  sum_q += dft_in_r[j] * icos[w];
	  w += i + dft_bin_size; /* Makes the freq range max/min 2:1 */
	  if( w >= dft_input_size ) w -= dft_input_size;
	}

	/* Normalized summations to bins */
	dft_out_r[i] = sum_i/dft_input_size;
	dft_out_i[i] = sum_q/dft_input_size;

  } /* for( i = 0; i < k; i++ ) */

} /* Idft() */

/*------------------------------------------------------------------------*/

