/*
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

#include "filters.h"
#include "shared.h"

/*----------------------------------------------------------------------*/

/* Init_Chebyshev_Filter()
 *
 * Calculates Chebyshev recursive filter coefficients.
 * The filter_data_t struct is defined in common.h
 */
  void
Init_Chebyshev_Filter( filter_data_t *filter_data )
{
  double *ta = NULL, *tb = NULL;
  double a0, a1, a2, b1, b2, sa, sb, gain;
  int i, p;
  double rp, ip, es, vx, kx, t, w, m;
  double d, xn0, xn1, xn2, yn1, yn2, k, tmp;


  if( !filter_data->npoles ) return;

  /* Allocate data arrays */
  size_t mreq = (size_t)(filter_data->npoles + 3) * sizeof(double);
  mem_alloc( (void **)&ta, mreq );
  mem_alloc( (void **)&tb, mreq );

  /* Allocate coefficient arrays */
  free_ptr( (void **)&(filter_data->a) );
  free_ptr( (void **)&(filter_data->b) );
  mem_alloc( (void **)&(filter_data->a), mreq );
  mem_alloc( (void **)&(filter_data->b), mreq );

  /* Allocate saved input and output arrays */
  free_ptr( (void **)&(filter_data->x) );
  free_ptr( (void **)&(filter_data->y) );
  mreq = (size_t)(filter_data->npoles + 1) * sizeof(double);
  mem_alloc( (void **)&(filter_data->x), mreq );
  mem_alloc( (void **)&(filter_data->y), mreq );

  /* Clear x and y arrays */
  for( i = 0; i <= filter_data->npoles; i++ )
  {
	filter_data->x[i] = 0.0;
	filter_data->y[i] = 0.0;
  }

  /* Initialize coefficient arrays */
  for( i = 0; i <= filter_data->npoles + 2; i++ )
  {
	filter_data->a[i] = 0.0;
	filter_data->b[i] = 0.0;
  }
  filter_data->a[2] = 1.0;
  filter_data->b[2] = 1.0;

  /* S-domain to Z-domain conversion */
  t = 2.0 * tan( 0.5 );

  /* Cutoff frequency */
  if( filter_data->cutoff >= 0.49 )
	filter_data->cutoff = 0.49;
  w = M_2PI * filter_data->cutoff;

  /* Low Pass to Low Pass or Low Pass to High Pass transform */
  if( filter_data->type == FILTER_HIGHPASS )
	k = -cos( (w + 1.0) / 2.0 ) / cos( (w - 1.0) / 2.0 );
  else if( filter_data->type == FILTER_LOWPASS )
	k = sin( (1.0 - w) / 2.0 ) / sin( (1.0 + w) / 2.0 );
  else k = 1.0; // For compiler warnings */

  /* Find coefficients for 2-pole filter for each pole pair */
  for( p = 1; p <= filter_data->npoles / 2; p++ )
  {
	/* Calculate the pole location on the unit circle */
	tmp = M_PI / (double)filter_data->npoles / 2.0 +
	  (double)(p - 1) * M_PI / (double)filter_data->npoles;
	rp  = -cos( tmp );
	ip  =  sin( tmp );

	/* Wrap from a circle to an ellipse */
	if( filter_data->ripple > 0.0 )
	{
	  tmp = 100.0 / ( 100.0 - filter_data->ripple );
	  es  = sqrt( tmp * tmp - 1.0 );
	  tmp = 1.0 / (double)filter_data->npoles;
	  vx  = tmp * asinh( 1.0 / es );
	  kx  = tmp * acosh( 1.0 / es );
	  kx  = cosh( kx );
	  rp *= sinh( vx ) / kx;
	  ip *= cosh( vx ) / kx;
	}

	/* S-domain to Z-domain conversion */
	m = rp * rp + ip * ip;
	d = 4.0 - 4.0 * rp * t + m * t * t;
	xn0 = t * t / d;
	xn1 = 2.0 * t * t / d;
	xn2 = t * t / d;
	yn1 = ( 8.0 - 2.0 * m * t * t ) / d;
	yn2 = ( -4.0 -4.0 * rp * t - m * t * t ) / d;

	/* Low Pass to Low Pass or Low Pass to High Pass transform */
	d  = 1.0 + yn1 * k -yn2 * k * k;
	a0 = ( xn0 - xn1 * k + xn2 * k * k ) / d;
	a1 = ( -2.0 * xn0 * k + xn1 + xn1 * k * k - 2.0 * xn2 * k ) / d;
	a2 = ( xn0 * k * k - xn1 * k + xn2 ) / d;
	b1 = ( 2.0 * k + yn1 + yn1 * k * k - 2.0 * yn2 * k ) / d;
	b2 = ( -k * k - yn1 * k + yn2 ) / d;

	if( filter_data->type == FILTER_HIGHPASS )
	{
	  a1 = -a1;
	  b1 = -b1;
	}

	/* Add coefficients to the cascade */
	for( i = 0; i <= filter_data->npoles + 2; i++ )
	{
	  ta[i] = filter_data->a[i];
	  tb[i] = filter_data->b[i];
	}

	for( i = 2; i <= filter_data->npoles + 2; i++ )
	{
	  filter_data->a[i] = a0 * ta[i] + a1 * ta[i-1] + a2 * ta[i-2];
	  filter_data->b[i] =      tb[i] - b1 * tb[i-1] - b2 * tb[i-2];
	}

  } /* for( p = 1; p <= np / 2; p++ ) */

  /* Finish combining coefficients */
  filter_data->b[2] = 0.0;
  for( i = 0; i <= filter_data->npoles; i++ )
  {
	filter_data->a[i] =  filter_data->a[i+2];
	filter_data->b[i] = -filter_data->b[i+2];
  }

  /* Normalize the gain */
  double sign = 1.0;
  sa = 0.0; sb = 0.0;
  for( i = 0; i <= filter_data->npoles; i++ )
  {
	if( filter_data->type == FILTER_LOWPASS )
	{
	  sa += filter_data->a[i];
	  sb += filter_data->b[i];
	}
	else if( filter_data->type == FILTER_HIGHPASS )
	{
	  sa += filter_data->a[i] * sign;
	  sb += filter_data->b[i] * sign;
	  sign = -sign;
	}
  } /* for( i = 0; <= filter_data->npoles; i++ ) */

  /* Normalize the gain */
  gain = sa / ( 1.0 - sb );
  for( i = 0; i <= filter_data->npoles; i++ )
  {
	filter_data->a[i] /= gain;
 }

  free_ptr( (void **)&ta );
  free_ptr( (void **)&tb );

  return;
} /* Init_Chebyshev_Filter() */

/*----------------------------------------------------------------------*/

/* DSP_Filter()
 *
 * DSP Recursive Filter, normally used as low pass
 */
  void
DSP_Filter( filter_data_t *filter_data )
{
  /* Index to samples buffer */
  int buf_idx, idx, npp1, len;
  double y, yn0;

  /* Filter samples in the buffer */
  npp1 = filter_data->npoles + 1;
  len  = filter_data->samples_buf_len;
  for( buf_idx = 0; buf_idx < len; buf_idx++ )
  {
	/* Calculate and save filtered samples */
	yn0 = filter_data->samples_buf[buf_idx] * filter_data->a[0];
	for( idx = 1; idx < npp1; idx++ )
	{
	  /* Summate contribution of past input samples */
	  y    = filter_data->a[idx];
	  y   *= filter_data->x[filter_data->ring_idx];
	  yn0 += y;

	  /* Summate contribution of past output samples */
	  y   = filter_data->b[idx];
	  y  *= filter_data->y[filter_data->ring_idx];
	  yn0 += y;

	  /* Advance ring buffers index */
	  filter_data->ring_idx++;
	  if( filter_data->ring_idx >= npp1 )
		filter_data->ring_idx = 0;

	} /* for( idx = 0; idx < npp1; idx++ ) */

	/* Save new yn0 output to y ring buffer */
	filter_data->y[filter_data->ring_idx] = yn0;

	/* Save current input sample to x ring buffer */
	filter_data->x[filter_data->ring_idx] =
	  filter_data->samples_buf[buf_idx];

	/* Return filtered samples */
	filter_data->samples_buf[buf_idx] = yn0;

  } /* for( buf_idx = 0; buf_idx < len; buf_idx++ ) */

} /* DSP_Filter() */

/*----------------------------------------------------------------------*/

