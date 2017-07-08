/*  detect.c
 *
 *  Signal detection functions of xwefax application
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

#include "detect.h"

/*------------------------------------------------------------------------*/

/* FM_Detect_Zero_Crossing()
 *
 * Estimates the frequency of the incoming WEFAX audio signal by
 * counting DSP samples, up for +ve and down for -ve, up to a
 * maximum count of +- 1/4 of a cycle (of the highest (white)
 * frequency). This results in the count passing through zero
 * between the +ve and -ve half cycles of the signal and thus
 * a measure of the length of half a signal cycle is obtained.
 * From this the instantaneous signal frequency is calculated.
 */
  gboolean
FM_Detect_Zero_Crossing( unsigned char *signal_level )
{
  short
	signal_sample,	/* Signal sample from DSP */
	signal_max;		/* Maximum level from Audio DSP */

  double
	half_cycle,	/* Half period of the WEFAX signal */
	dsp_rate2,	/* Half the DSP rate, as a double  */
	discr_op,	/* Output of FM detector (0-255)   */
	dx = 0.0;	/* Interpolation of zero crossing point */

  /* Index of DSP samples used. It is a float as
   * for some modes, like SSTV, the pixel length
   * is not an integer number of DSP samples */
  static double idx = 0.0;

  static double
	zeros_period = 0.0,	/* Time elapsed between zeros, in DSP samples */
	signal_freq  = 0.0,	/* Measured frequency of incoming WEFAX signal */
	sig_average  = 0.0,	/* Signal samples average */
	last_average = 0.0;	/* Last signal average */

  int
	num_incrm,	/* Number of increments to period counter */
	num_zeros;	/* Number of zero crossings in a pixel */

  /* These two variables are used to limit the effects of noise
   * by imposing a minimum count of DSP samples between zeros */
  static int
	/* Minimum length of WEFAX signal 1/3 cycle in DSP samples */
	min_cycle3 = 0,
	/* Count of samples between zero crossings */
	num_samples = 0;

  /* Initialize above, make it a little smaller than theoretical */
  if( !min_cycle3 )
	min_cycle3 = rc_data.dsp_rate / rc_data.white_freq / 3;

  /* Look for a zero crossing of the WEFAX audio
   * signal over the duration of each image pixel */
  signal_max = 0;
  num_incrm  = 0;
  num_zeros  = 0;
  dsp_rate2  = (double)( rc_data.dsp_rate / 2 );
  while( idx < rc_data.pixel_len )
  {
	if( rc_data.tcvr_type == PERSEUS )
	{
	  Demodulate_SSB( &signal_sample );
	}
	else
	{
	  /* Get new sample from DSP buffer */
	  if( !Sound_Signal_Sample(&signal_sample) )
		return( FALSE );
	}

	/* Get max absolute value of signal sample */
	if( signal_max < abs(signal_sample) )
	  signal_max = abs( signal_sample );

	/* Sliding widow average of DSP signal samples */
	sig_average  = sig_average * ( SIG_AVE_WINDOW - 1.0 );
	sig_average += (double)signal_sample;
	sig_average /= SIG_AVE_WINDOW;

	/* This gives us a zero crossing of the input waveform */
	num_samples++;
	if( ((sig_average * last_average) <= 0.0) &&
		(num_samples >= min_cycle3) )
	{
	  /* Signal frequency is 1/2 DSP rate / length of half cycle */
	  /* Interpolate point of zero crossing */
	  if( (last_average - sig_average) != 0.0 )
		dx = sig_average / ( last_average - sig_average );
	  if( dx < -1.0 ) dx = -1.0;
	  if( dx > 1.0 )  dx =  1.0;

	  num_zeros++;
	  num_incrm = 0;
	  num_samples = 0;
	} /* if( (sig_average * last_average) < 0.0 ) */

	/* Save current signal average */
	last_average = sig_average;

	/* Count number of signal samples
	 * between zero crossings */
	zeros_period += 1.0;
	num_incrm++;

	/* Count DSP samples */
	idx += 1.0;
  } /* while( idx < rc_data.pixel_len ) */

  /* Calculate signal frequency from half cycle period */
  zeros_period += dx; /* Add extrapolation of zero crossing */
  if( num_zeros )
  {
	half_cycle = ( zeros_period - (double)num_incrm ) / (double)num_zeros;
	if( half_cycle != 0.0 )
	  signal_freq = dsp_rate2 / half_cycle;
  }

  /* Prepares zeros_period to properly count
   * signal samples to next zero crossing */
  zeros_period = (double)num_incrm - dx;

  /* Reset the samples index */
  idx -= rc_data.pixel_len;

  /* Scale and floor frequency to give a value 0-255 */
  discr_op = signal_freq / DISCR_SCALE - DISCR_FLOOR;

  /* Limit disriminator output in right range */
  if( discr_op > 255.0 ) discr_op = 255.0;
  if( discr_op < 0.0 )   discr_op = 0.0;
  *signal_level = (unsigned char)discr_op;

  /* Display maximum signal level scaled down */
  if( isFlagClear(DISPLAY_SIGNAL) )
  {
	Display_Signal( (unsigned char)(signal_max >> 7) );
	Display_Level_Gauge(
		(int)(signal_max / SIG_GAUGE_SCALE),
		SIG_GAUGE_LEVEL1, SIG_GAUGE_LEVEL2 );
  }

  return( TRUE );
} /* FM_Detect_Zero_Crossing() */

/*------------------------------------------------------------------------*/

/* FM_Detect_Bilevel()
 *
 * Estimates the WEFAX input signal's frequency by comparing
 * the output of a Goertzel detector on the black frequency
 * (1500 Hz) and one on the white frequency (2300 Hz).
 */
  gboolean
FM_Detect_Bilevel( unsigned char *signal_level )
{
  static int
	first_call = TRUE,
	det_period,	/* Integration period of Goertzel detector */
	signal_idx;	/* Signal samples buffer index */

  int
	idx,
	black_level, /* Level of the Black signal */
	white_level; /* Level of the White signal */

  /* Circular signal samples buffer for Goertzel detector */
  static short *signal_buff = NULL;
  short signal_max;  /* Maximum level from Audio DSP */

  /* Variables for the Goertzel algorithm */
  static double black_cosw, black_coeff;
  static double white_cosw, white_coeff;
  static double scale;
  double black_q0, black_q1, black_q2;
  double white_q0, white_q1, white_q2;

  /* Index of DSP samples used. It is a float as
   * for some modes, like SSTV, the pixel length
   * is not an integer number of DSP samples */
  static double pixel_idx = 0.0;


  /* Initialize on first call */
  if( first_call )
  {
	double w;

	/* Omega for the white frequency */
	w = M_2PI / (double)rc_data.dsp_rate * (double)rc_data.white_freq;
	white_cosw  = cos( w );
	white_coeff = 2.0 * white_cosw;

	/* Omega for the black frequency */
	w = M_2PI / (double)rc_data.dsp_rate * (double)rc_data.black_freq;
	black_cosw  = cos( w );
	black_coeff = 2.0 * black_cosw;

	det_period = rc_data.dsp_rate /
	  (rc_data.white_freq - rc_data.black_freq);

	/* To keep values of detected signal in reasonable limits */
	scale = (double)det_period * BILEVEL_SCALE_FACTOR;

	/* Allocate samples buffer and clear */
	size_t len = sizeof(short) * (size_t)det_period;
	if( !mem_realloc((void **)&signal_buff, len) )
	  return( FALSE );
	bzero( signal_buff, len );
	signal_idx = 0;

	first_call = FALSE;
  } /* if( first_call... ) */

  /* Save samples for detector */
  signal_max = 0;
  while( pixel_idx < rc_data.pixel_len )
  {
	if( rc_data.tcvr_type == PERSEUS )
	{
	  Demodulate_SSB( &signal_buff[signal_idx] );
	}
	else
	{
	  /* Get signal sample from buffer, abort on error */
	  if( !Sound_Signal_Sample(&signal_buff[signal_idx]) )
		return( FALSE );
	}

	/* Get max absolute value of signal sample */
	if( signal_max < abs(signal_buff[signal_idx]) )
	  signal_max = abs( signal_buff[signal_idx] );

	/* Increment/reset circular buffer's index */
	signal_idx++;
	if( signal_idx >= det_period ) signal_idx = 0;

	/* Count DSP samples */
	pixel_idx += 1.0;

  } /* while( idx < rc_data.pixel_len ) */

  /* Reset the samples index */
  pixel_idx -= rc_data.pixel_len;

  /* Calculate signal level of black and white
   * tone frequencies using Goertzel algorithm */
  black_q1 = black_q2 = 0.0;
  white_q1 = white_q2 = 0.0;
  for( idx = 0; idx < det_period; idx++ )
  {
	black_q0 =
	  black_coeff * black_q1 - black_q2 + (double)signal_buff[signal_idx];
	black_q2 = black_q1;
	black_q1 = black_q0;

	white_q0 =
	  white_coeff * white_q1 - white_q2 + (double)signal_buff[signal_idx];
	white_q2 = white_q1;
	white_q1 = white_q0;

	/* Increment/reset circular buffers' index */
	signal_idx++;
	if( signal_idx >= det_period ) signal_idx = 0;

  } /* for( idx = 0; idx < det_period; idx++ ) */

  /* Magnitude of black tone scaled by dot size and tone freq */
  black_q1 /= scale;
  black_q2 /= scale;
  black_level = (int)
	((black_q1 * black_q1 + black_q2 * black_q2 -
	  black_q1 * black_q2 * black_coeff));

  /* Magnitude of white tone scaled by dot size and tone freq */
  white_q1 /= scale;
  white_q2 /= scale;
  white_level = (int)
	( (white_q1 * white_q1 + white_q2 * white_q2 -
	   white_q1 * white_q2 * white_coeff) );

  /* Calculate signal level according to ratio between
   * black and white Goertzel tone detector outputs */
  if( black_level > 8 * white_level )
	*signal_level = 0;
  else if( (black_level <= 8 * white_level) && (black_level > 4 * white_level) )
	*signal_level = 64;
  else if( (black_level <= 4 * white_level) && (white_level < 4 * black_level) )
	*signal_level = 128;
  else if( (white_level >= 4 * black_level) && (white_level < 8 * black_level) )
	*signal_level = 196;
  else *signal_level = 255;

  /* Display maximum signal level scaled down */
  if( isFlagClear(DISPLAY_SIGNAL) )
  {
	Display_Signal( (unsigned char)(signal_max >> 7) );
	Display_Level_Gauge(
		(int)(signal_max / SIG_GAUGE_SCALE),
		SIG_GAUGE_LEVEL1, SIG_GAUGE_LEVEL2 );
  }

  return( TRUE );
} /* FM_Detect_Bilevel() */

/*------------------------------------------------------------------------*/

/*  Phasing_Detect()
 *
 *  Detects phasing pulses using Goertzel's algorithm.
 */

  gboolean
Phasing_Detect( void )
{
  static int
	pixels_per_line = 0, /* WEFAX RPM or lines per minute */
	pixel_idx		= 0, /* Index to pixels in Wefax line */
	phasing_cnt     = 0, /* Count of phasing pulse lines examined */
	phasing_error   = 0, /* The distance of phasing pulse from line middle  */
	phasing_ref;		 /* Reference for initial position of phasing pulse */

  /* Average of phasing pulse over sliding window */
  static double	phasing_pulse_ave = 0.0;

  int
	error_limit,		/* Max value of phasing pulse position error */
	phasing_pulse_max,	/* Maximum level of above over the length of a line  */
	pulse_max_idx = 0;	/* Fragment count where maximum pulse level occurs */

  /* Output from FM detector */
  unsigned char discr_op;


  /* Initialize on change of parameters */
  if( pixels_per_line != rc_data.pixels_per_line )
  {
	/* We need to look for a phasing pulse maximum
	 * over the length (in pixesls) of 1 WEFAX line */
	pixels_per_line = rc_data.pixels_per_line;

	/* This puts the pahsing pulse in the middle
	 * of the image line during the syncing process */
	phasing_ref = pixels_per_line / 2 + PHASING_PULSE_LEN;

	pixel_idx = 0;
  } /* if( pixels_per_line != rc_data.pixels_per_line ) */

  /* Stop on user request */
  if( isFlagSet(RECEIVE_STOP) )
  {
	pixel_idx	  = 0;
	phasing_cnt   = 0;
	phasing_error = 0;
	wefax_action  = ACTION_STOP;
	return( TRUE );
  }

  /* Skip looking for phasing pulses */
  if( isFlagSet(SKIP_ACTION) )
  {
	/* Re-initialize line buffer indices */
	gbl_linebuff_input  = 0;
	gbl_linebuff_output =
	  rc_data.line_buffer_size - rc_data.pixels_per_line2;

	Show_Message( _("Skipping Phasing Pulse Sync"), "orange" );
	Show_Message( _("Starting WEFAX Image Decoder ..."), "black" );
	Show_Message( _("Listening for Stop Tone ..."), "black" );
	Set_Indicators( ICON_SYNC_SKIP );

	ClearFlag( SKIP_ACTION );
	pixel_idx	  = 0;
	phasing_cnt   = 0;
	phasing_error = 0;
	wefax_action  = ACTION_DECODE;
	return( TRUE );
  }

  /* Fill line buffer with pixel values */
  /* Get signal from FM detector, abort on error */
  if( !FM_Detector(&discr_op) ) return( FALSE );

  /* Display the phasing pulse level */
  if( isFlagSet(DISPLAY_SIGNAL) ) Display_Signal( discr_op );

  /* Fill line buffer with pixel values */
  gbl_line_buffer[ gbl_linebuff_input ] = discr_op;

  /* Advance line buffer input index */
  gbl_linebuff_input++;
  if( gbl_linebuff_input >= pixels_per_line )
	gbl_linebuff_input = 0;

  pixel_idx++;
  if( pixel_idx < pixels_per_line ) return( TRUE );

  /* Look for a phasing pulse maximum over a line.
   * We begin the index from -phasing_error to dump
   * the number of pixels needed to center the pulse */
  phasing_pulse_max = -256;
  phasing_pulse_ave = 0.0;

  /* Look for phasing pulse maximum */
  for( pixel_idx = 0; pixel_idx < pixels_per_line; pixel_idx++ )
  {
	/* Average line buffer pixel values */
	phasing_pulse_ave *= PHASING_PUSLE_WIN - 1.0;
	phasing_pulse_ave += (double)gbl_line_buffer[ pixel_idx ];
	phasing_pulse_ave /= PHASING_PUSLE_WIN;

	/* Record the input buffer index where max occurs */
	if( phasing_pulse_max < (int)phasing_pulse_ave )
	{
	  phasing_pulse_max = (int)phasing_pulse_ave;
	  pulse_max_idx     = pixel_idx;
	}
  } /* for( pixel_idx = 0; pixel_idx < pixels_per_line; pixel_idx++ */

  /* Count phasing pulse lines */
  phasing_cnt++;

  /* Limit of pulse position error is
   * reduced progressively with line count */
  error_limit   = pixels_per_line / 2 / phasing_cnt;
  phasing_error = pulse_max_idx - phasing_ref;

  /* Limit value of phasing error to avoid big jumps */
  if( phasing_error > error_limit )
	phasing_error = error_limit;
  else if( phasing_error < -error_limit )
	phasing_error = -error_limit;

  /* Adjust the line buffer index by the phasing error */
  gbl_linebuff_input -= phasing_error;
  if( gbl_linebuff_input >= pixels_per_line )
	gbl_linebuff_input -= pixels_per_line;
  else if( gbl_linebuff_input < 0 )
	gbl_linebuff_input += pixels_per_line;

  /* Look for phasing pulses over most of phasing lines */
  if( phasing_cnt > rc_data.phasing_lines )
  {
	/* Point the line buffer output index to middle
	 * of the lines buffer, this puts the phasing
	 * pulse at the beginning of image lines */
	gbl_linebuff_output =
	  rc_data.line_buffer_size - rc_data.pixels_per_line2;

	Show_Message( _("Phasing Pulse Synching ended"), "green" );
	Show_Message( _("Starting WEFAX Image Decoder ..."), "black" );
	Show_Message( _("Listening for Stop Tone ..."), "black" );
	Set_Indicators( ICON_SYNC_APPLY );
	pixel_idx	  = 0;
	phasing_cnt   = 0;
	phasing_error = 0;
	wefax_action  = ACTION_DECODE;
  }

 pixel_idx = 0;
 return( TRUE );
} /* Phasing_Detect() */

/*------------------------------------------------------------------------*/

/* Tone_Detect()
 *
 * Detects audio tones used for start and stop signaling.
 * The tone period is in pixels and input in pixel levels.
 */

  void
Tone_Detect(
	double tone_period,
	unsigned char input,
	int *tone_level )
{
  static int
	level_ave = 0,	 /* Sliding window average of tone level */
	input_cnt,		 /* Count of pixel level inputs */
	detector_period; /* Integration period of Goertzel detector */

  /* Variables for the Goertzel algorithm */
  static double coeff, period;
  static double q0 = 0.0, q1 = 0.0, q2 = 0.0;

  int level; /* Detected Tone level */


  /* Stop on user request */
  if( isFlagSet(RECEIVE_STOP) )
  {
	input_cnt = 0;
	period    = 0.0;
	level_ave = 0;
	wefax_action = ACTION_STOP;
	return;
  } /* if( isFlagSet(RECEIVE_STOP) ) */

  /* Skip looking for start/stop tones */
  if( isFlagSet(SKIP_ACTION) )
  {
	/* Re-initialize line buffer indices */
	gbl_linebuff_input  = 0;
	gbl_linebuff_output =
	  rc_data.line_buffer_size - rc_data.pixels_per_line2;

	input_cnt = 0;
	period    = 0.0;
	level_ave = 0;
	return;
  }

  /* Initialize on new parameters */
  if( period != tone_period )
  {
	period = tone_period;
	detector_period = (int)
	  ( period * TONE_PERIOD_MULT / rc_data.lines_per_min + 0.5 );

	double w = M_2PI / period;
	coeff = 2.0 * cos( w );

	/* Reset variables */
	input_cnt = 0;
	q0 = q1 = q2 = 0.0;
	level_ave = 0;
  } /* if( period != tone_period ) */

  /* Calculate Start/Stop level using Goertzel algorithm */
  q0 = coeff * q1 - q2 + (double)input - 127.0;
  q2 = q1;
  q1 = q0;

  /* Compute tone level and reset after detector_period inputs */
  if( input_cnt++ >= detector_period )
  {
	/* Reduce the magnitude to reasonable levels */
	q1 /= period;
	q2 /= period;
	level = (int)( q1 * q1 + q2 * q2 - q1 * q2 * coeff );

	/* Compute sliding average of tone level and return */
	level_ave *= TONE_LEVEL_AVE_WIN - 1;
	level_ave += level;
	level_ave /= TONE_LEVEL_AVE_WIN;

	/* Reset variables */
	q0 = q1 = q2 = 0.0;
	input_cnt = 0;
  } /* if( input_cnt++ >= detector_period ) */

  *tone_level = level_ave;

  return;
} /* Tone_Detect() */

/*------------------------------------------------------------------------*/

/* Start_Tone_Detect()
 *
 * Listens for and detects the Start tone
 */
  gboolean
Start_Tone_Detect( void )
{
  /* Detector output */
  unsigned char discr_op;
  int tone_level = 0;
  static gboolean tone_up = FALSE;


  /* Take signal samples from FM detector for the Start Tone detector */
  if( !FM_Detector(&discr_op) ) return( FALSE );
  Tone_Detect( rc_data.start_tone_period, discr_op, &tone_level );

  /* Display detector output and level gauge */
  if( isFlagSet(DISPLAY_SIGNAL) )
  {
	Display_Signal( discr_op );
	Display_Level_Gauge(
		tone_level / START_GAUGE_SCALE,
		START_TONE_DOWN / START_GAUGE_SCALE,
		START_TONE_UP / START_GAUGE_SCALE );
  }

  /* Skip looking for start tones */
  if( isFlagSet(SKIP_ACTION) )
  {
	/* Re-initialize line buffer indices */
	gbl_linebuff_input  = 0;
	gbl_linebuff_output =
	  rc_data.line_buffer_size - rc_data.pixels_per_line2;

	ClearFlag( SKIP_ACTION );
	Show_Message( _("Skipping Start Tone Detection"), "orange" );
	Show_Message( _("Synchronizing Phasing Pulses ..."), "black" );
	Set_Indicators( ICON_START_SKIP );
	Set_Indicators( ICON_SYNC_YES );
	wefax_action = ACTION_PHASING;
	tone_up = FALSE;
	return( TRUE );
  }

  /* Record the rise of start tone level */
  if( tone_level > START_TONE_UP )
	tone_up = TRUE;

  /* Go to searching for Phasing Pulses when tone goes down */
  if( (tone_level < START_TONE_DOWN) && tone_up )
  {
	Show_Message( _("Start Tone Detected"), "green" );
	Show_Message( _("Synchronizing Phasing Pulses ..."), "black" );
	Set_Indicators( ICON_START_APPLY );
	Set_Indicators( ICON_SYNC_YES );
	tone_up = FALSE;
	wefax_action = ACTION_PHASING;
  }

  return( TRUE );
} /* Start_Tone_Detect() */

/*------------------------------------------------------------------------*/

/* Stop_Tone_Detect()
 *
 * Listens for and detects the Start tone
 */
  gboolean
Stop_Tone_Detect( short discr_op )
{
  /* Detector output */
  int tone_level = 0;
  static gboolean tone_up = FALSE;


  /* Get the stop tone level */
  Tone_Detect( rc_data.stop_tone_period, discr_op, &tone_level );

  /* Display detector output and level gauge */
  if( isFlagSet(DISPLAY_SIGNAL) )
	Display_Level_Gauge(
		tone_level / STOP_GAUGE_SCALE,
		STOP_TONE_DOWN / STOP_GAUGE_SCALE,
		STOP_TONE_UP / STOP_GAUGE_SCALE );

  /* Record the rise of start tone level */
  if( tone_level > STOP_TONE_UP )
	tone_up = TRUE;

  /* Go to searching for Phasing Pulses when tone goes down */
  if( (tone_level < STOP_TONE_DOWN) && tone_up )
  {
	Show_Message( _("Stop Tone Detected"), "green" );
	tone_up = FALSE;
	return( TRUE );
  }

  return( FALSE );
} /* Stop_Tone_Detect() */

/*------------------------------------------------------------------------*/

