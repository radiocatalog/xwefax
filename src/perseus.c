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

#include "perseus.h"
#include "shared.h"

/* Perseus device description */
static perseus_descr *descr = NULL;

/* I/Q Samples buffers for the LP Filters */
static double *demod_buf_i = NULL, *demod_buf_q = NULL;

/*----------------------------------------------------------------------*/

/* Perseus_Settings()
 *
 * Sets the Preamp on/off, Attenuators on/off and ADC Dither on/off
 */
  static gboolean
Perseus_Settings( uint8_t setting )
{
  char txt[MESG_SIZE];
  static uint8_t settings = PERSEUS_INITIAL_SETTINGS;
  int ret;


  if( !descr ) return( TRUE );

  /* Set or clear a setting flag bit */
  if( setting & PERSEUS_CLEAR_SETTING )
    settings &= setting;
  else
    settings |= setting;

  /* Set attenuators */
  ret = perseus_set_attenuator_n(
      descr, settings & PERSEUS_ATTEN_SETTINGS );
  if( ret < 0 )
  {
    snprintf( txt, sizeof(txt),
        _("Perseus: perseus_set_attenuator() error:\n%s"),
        perseus_errorstr() );
    Error_Dialog( txt, OK );
    return( FALSE );
  }

    return( TRUE );
} /* Perseus_Settings() */

/*----------------------------------------------------------------------*/

/* Perseus_Attenuators()
 *
 * Calculates S-meter indication from ADAGC scale factor
 * and also controls input attenuators in auto mode
 */
  static void
Perseus_Attenuators( double adagc_scale )
{
  /* ADAGC scale factor in relative dBm */
  double log_adagc_scale;
  static gboolean att_10db = FALSE, att_20db = FALSE;

  /* Set Perseus attenuators if in auto mode */
  log_adagc_scale = 20.0 * log10( adagc_scale );
    if( log_adagc_scale > PERSEUS_ATT_UP )
    {
      if( !att_10db && !att_20db )
      {
        Perseus_Settings( PERSEUS_ATTEN_10DB );
        att_10db = TRUE;
      }
      else if( !att_20db && att_10db )
      {
        Perseus_Settings(  PERSEUS_ATTEN_20DB );
        Perseus_Settings( (uint8_t)~PERSEUS_ATTEN_10DB );
        att_20db = TRUE;
        att_10db = FALSE;
      }
      else if( att_20db && !att_10db )
      {
        Perseus_Settings( PERSEUS_ATTEN_10DB );
        att_10db = TRUE;
      }
    } /* if( log_adagc_scale > PERSEUS_ATT_UP ) */
    else if( log_adagc_scale < PERSEUS_ATT_DOWN )
    {
      if( att_10db )
      {
        Perseus_Settings( (uint8_t)~PERSEUS_ATTEN_10DB );
        att_10db = FALSE;
      }
      else if( att_20db && !att_10db )
      {
        Perseus_Settings(  PERSEUS_ATTEN_10DB );
        Perseus_Settings( (uint8_t)~PERSEUS_ATTEN_20DB );
        att_10db = TRUE;
        att_20db = FALSE;
      }
    } /* if( log_adagc_scale > PERSEUS_ATT_DOWN ) */

} /* Perseus_Attenuators() */

/*----------------------------------------------------------------------*/

/* Demodulate_SSB()
 *
 * Demodulates SSB Signals
 */
  gboolean
Demodulate_SSB( short *signal_sample )
{
  /* Index to i and q buffers */
  static int iqd_buf_idx = 0;

  /* sinf/cosf tables, their length and index */
  static double *sinf = NULL, *cosf = NULL;
  static int trig_len, itr = 0;

  double
    cutoff = 0.0,    /* Cutoff frequency of low pass filters */
    dphi   = 0.0,    /* Change in Phase angle of sampled IF signal */
    signal_ratio,    /* Ratio of detected signal to ADAGC reference */
    base_band = 0.0; /* SSB Radio Signal's base band */

  static double adagc_scale = 1.0;

  /* Demodulator filter data structs for samples buffers */
  static filter_data_t demod_filter_data_i, demod_filter_data_q;


  /* Initialize on first call */
  static gboolean init = TRUE;
  if( init )
  {
    double phi = 0.0;
    int idx;

    /* Length of sinf/cosf trig tables */
    trig_len = PERSEUS_SAMPLE_RATE / PERSEUS_WEAVER_FREQ;

    /* Allocate trigonometric tables */
    size_t req = (size_t)trig_len * sizeof(double);
    mem_alloc( (void **)&sinf, req );
    mem_alloc( (void **)&cosf, req );

    /* Calculate trigonometric tables. "Negative" Weaver
     * frequency is used for lower sideband demodulation */
    if( strcmp(rc_data.station_sideband, "USB") == 0 )
      dphi = M_2PI / (double)trig_len;
    else if( strcmp(rc_data.station_sideband, "LSB") == 0 )
      dphi = -M_2PI / (double)trig_len;

    for( idx = 0; idx < trig_len; idx++ )
    {
      sinf[idx] = sin( phi );
      cosf[idx] = cos( phi );
      phi += dphi;
    }

    /* LP Filter cutoff must be specified taking into
     * account the transition band as a fraction of Fc */
    cutoff  = PERSEUS_DEMOD_BANDW;
    cutoff /= (double)PERSEUS_SAMPLE_RATE * 2.0;

    /* Initialize Demodulator I filter */
    demod_filter_data_i.cutoff   = cutoff;
    demod_filter_data_i.ripple   = SSB_FILTER_RIPPLE;
    demod_filter_data_i.npoles   = SSB_FILTER_POLES;
    demod_filter_data_i.type     = FILTER_LOWPASS;
    demod_filter_data_i.ring_idx = 0;
    demod_filter_data_i.samples_buf = demod_buf_i;
    demod_filter_data_i.samples_buf_len = PERSEUS_BUFFER_LEN;
    Init_Chebyshev_Filter( &demod_filter_data_i );

    /* Initialize Demodulator Q filter */
    demod_filter_data_q.cutoff   = cutoff;
    demod_filter_data_q.ripple   = SSB_FILTER_RIPPLE;
    demod_filter_data_q.npoles   = SSB_FILTER_POLES;
    demod_filter_data_q.type     = FILTER_LOWPASS;
    demod_filter_data_q.ring_idx = 0;
    demod_filter_data_q.samples_buf = demod_buf_q;
    demod_filter_data_q.samples_buf_len = PERSEUS_BUFFER_LEN;
    Init_Chebyshev_Filter( &demod_filter_data_q );

    init = FALSE;
  } /* if( init ) */

  /* Wait for new IQ data */
  if( iqd_buf_idx >= PERSEUS_BUFFER_LEN )
  {
    /* Wait on DSP data to be ready for processing */
    sem_wait( &pback_semaphore );

    /* Demodulate filtered I/Q buffers */
    DSP_Filter( &demod_filter_data_i );
    DSP_Filter( &demod_filter_data_q );

    iqd_buf_idx = 0;
  }

  /* Apply Weaver SSB demodulator method to get base band */
  base_band =
    demod_buf_i[iqd_buf_idx] * sinf[itr] +
    demod_buf_q[iqd_buf_idx] * cosf[itr];
  itr++;
  if( itr >= trig_len ) itr = 0;
  iqd_buf_idx++;

  /* Apply audio derived AGC */
  /* Ratio of demodulated signal level to reference
   * level. This is about 2/3 of max level the sound
   * system can handle (+/- 32384 for 16-bit audio) */
  signal_ratio = fabs( base_band ) / ADAGC_REF_LEVEL;

  /* This is the AGC "attack" function */
  if( signal_ratio > adagc_scale )
    adagc_scale = signal_ratio;
  else /* This the AGC "decay" function */
    adagc_scale *= ADAGC_DECAY;

  /* Scale demodulated signal as needed */
  base_band /= adagc_scale;

  /* Return demod output as short int */
  *signal_sample = (short)base_band;

  /* Decimate sample values for the DFT */
  DFT_Input_Data( *signal_sample );

  /* Control attenuators as needed */
  Perseus_Attenuators( adagc_scale );

  return( TRUE );
} /* Demodulate_SSB() */

/*----------------------------------------------------------------------*/

/* Perseus_Data_Cb()
 *
 * Callback function for perseus_start_async_input
 */
  static int
Perseus_Data_Cb( void *buf, int buf_size, void *extra )
{
  /* The buffer received contains 24-bit IQ samples
   * (6 bytes per sample). We convert the samples to
   * 32 bit (msb aligned) integer IQ samples and
   * then store them in the Perseus buffer as doubles */
  uint8_t *samplebuf = (uint8_t *)buf;
  int nSamples = buf_size / 6;
  iq_sample s;

  /* Local buffers for i and q */
  static int32_t *i_buf = NULL, *q_buf = NULL;
  static int iq_idx = 0;

  /* Demodulator buffer length and count of samples cached */
  static int buffer_len = 0, count = 0;
  int buf_idx, idx;


  /* Initialize on first call */
  if( buffer_len == 0 )
  {
    buffer_len = PERSEUS_BUFFER_LEN;
    size_t siz = (size_t)buffer_len * sizeof(int32_t);
    mem_alloc( (void *)&i_buf, siz );
    mem_alloc( (void *)&q_buf, siz );
  }

  /* Clear top bytes */
  s.iq_data.i1 = s.iq_data.q1 = 0;
  for( idx = 0; idx < nSamples; idx++ )
  {
    /* Repack data to 32 bit (msb aligned) integers */
    s.iq_data.i2 = *samplebuf++;
    s.iq_data.i3 = *samplebuf++;
    s.iq_data.i4 = *samplebuf++;
    s.iq_data.q2 = *samplebuf++;
    s.iq_data.q3 = *samplebuf++;
    s.iq_data.q4 = *samplebuf++;

    /* Copy I/Q sample data to local buffers */
    i_buf[iq_idx] = s.iq.i;
    q_buf[iq_idx] = s.iq.q;
    iq_idx++;
    if( iq_idx >= buffer_len ) iq_idx = 0;

    /* Count number of samples buffered */
    count++;

    /* Copy local buffers to Perseus buffers in double form */
    if( count >= buffer_len )
    {
      for( buf_idx = 0; buf_idx < buffer_len; buf_idx++ )
      {
        demod_buf_i[buf_idx] = (double)i_buf[iq_idx];
        demod_buf_q[buf_idx] = (double)q_buf[iq_idx];
        iq_idx++;
        if( iq_idx >= buffer_len ) iq_idx = 0;
      }
      count = 0;

      /* Post to semaphore that DSP data is ready */
      int sval;
      sem_getvalue( &pback_semaphore, &sval );
      if( !sval ) sem_post( &pback_semaphore );
    }

  } /* for( idx = 0; idx < nSamples; idx++ ) */

  return( 0 );
} /* Perseus_Data_Cb() */

/*----------------------------------------------------------------------*/

/* Perseus_Read_Async()
 *
 * Pthread function to start async reading of Perseus I/Q samples
 */
  static void *
Perseus_Read_Async( void *arg )
{
  int ret = perseus_start_async_input(
      descr, PERSEUS_ASYNC_BUF_SIZE, Perseus_Data_Cb, NULL );

  if( ret < 0 )
    fprintf( stderr,
        _("perseus_start_async_input() returned %s\n"),
        perseus_errorstr() );

  return( NULL );
} /* Perseus_Read_Async() */

/*----------------------------------------------------------------------*/

/* Perseus_Set_Center_Frequency()
 *
 * Sets the Perseus FPGA DDC Center Frequency
 */
  void
Perseus_Set_Center_Frequency( int center_freq )
{
  if( isFlagSet(PERSEUS_INIT) )
  {
    /* Offset designated frequency to carrier frequency */
    double freq = (double)( center_freq + WEFAX_CARRIER_OFFSET );

    /* Correct for XO frequency error */
    freq *= 1.0 - rc_data.perseus_freq_correction;

    /* Set center frequency, enable preselector filters */
    int ret = perseus_set_ddc_center_freq( descr, freq, TRUE );

    /* Display status of frequency change */
    if( ret < 0 )
    {
      Error_Dialog( _("Failed to set DDC Center Frequency"), OK );
      return;
    }

  } /* if( isFlagSet(PERSEUS_INIT) ) */

} /* Perseus_Set_Center_Frequency() */

/*----------------------------------------------------------------------*/

/* Perseus_Close_Device()
 *
 * Closes thr Perseus device, if open
 */
  void
Perseus_Close_Device( void )
{
  if( isFlagSet(PERSEUS_INIT) )
  {
    ClearFlag( PERSEUS_INIT );
    perseus_stop_async_input( descr );
    perseus_close( descr );
    perseus_exit();
    descr = NULL;
  }

} /* Perseus_Close_Device() */

/*----------------------------------------------------------------------*/

  static void
Perseus_Init_Error( void )
{
  perseus_close( descr );
  descr = NULL;
} /* Perseus_Init_Error() */

/*----------------------------------------------------------------------*/

/* Perseus_Initialize()
 *
 * Initializes the Perseus Perseus SDR Receiver
 */
  gboolean
Perseus_Initialize( void )
{
  int ret;
  char txt[MESG_SIZE];
  eeprom_prodid prodid;
  pthread_t pthread_id;

  /* Abort if already init */
  if( isFlagSet(PERSEUS_INIT) ) return( TRUE );

  /* Set debug info dumped to stderr
   * to the maximum verbose level */
  perseus_set_debug( PERSEUS_DEBUG_LEVEL );

  /* Check how many Perseus receivers
   * are connected to the system */
  ret = perseus_init();
  if( ret == 0 )
  {
    perseus_exit();
    Error_Dialog( _("No Perseus Receivers detected"), QUIT );
    return( FALSE );
  }

  /* Open Perseus device */
  descr = perseus_open( PERSEUS_DEVICE_INDEX );
  if( descr == NULL )
  {
    perseus_exit();
    snprintf( txt, sizeof(txt),
        _("Failed to open Perseus Receiver:\n%s"),
        perseus_errorstr() );
    Error_Dialog( txt, QUIT );
    return( FALSE );
  }

  /* Download the standard firmware to the unit */
  ret = perseus_firmware_download( descr, NULL );
  if( ret < 0 )
  {
    Perseus_Init_Error();
    snprintf( txt, sizeof(txt),
        _("Perseus Firmware download error:\n%s"),
        perseus_errorstr() );
    Error_Dialog( txt, QUIT );
    return( FALSE );
  }

  /* Dump some information about the receiver (S/N and HW rev) */
  if( descr->is_preserie == TRUE )
    fprintf( stderr, "The device is a preserie unit" );
  else if( perseus_get_product_id(descr, &prodid) < 0 )
    fprintf( stderr, "Perseus get_product_id error: %s",
        perseus_errorstr() );
  else
    fprintf(stderr, "Receiver S/N: "
        "%05d-%02hX%02hX-%02hX%02hX-%02hX%02hX - HW Release:%hd.%hd\n",
        (uint16_t) prodid.sn,
        (uint16_t) prodid.signature[5],
        (uint16_t) prodid.signature[4],
        (uint16_t) prodid.signature[3],
        (uint16_t) prodid.signature[2],
        (uint16_t) prodid.signature[1],
        (uint16_t) prodid.signature[0],
        (uint16_t) prodid.hwrel,
        (uint16_t) prodid.hwver);

  /* Configure the receiver sample rate */
  ret = perseus_set_sampling_rate( descr, PERSEUS_SAMPLE_RATE );
  if( ret < 0 )
  {
    Perseus_Init_Error();
    snprintf( txt, sizeof(txt),
        _("Perseus fpga configuration error:\n%s"),
        perseus_errorstr() );
    Error_Dialog( txt, QUIT );
    return( FALSE );
  }

  /* Disable ADC Dither, enable ADC Preamp, disable attenuators */
  if( !Perseus_Settings(PERSEUS_INITIAL_SETTINGS) )
  {
    Perseus_Init_Error();
    return( FALSE );
  }

  /* Set ADC preamp and dither */
  ret = perseus_set_adc( descr, TRUE, TRUE );
  if( ret < 0 )
  {
    Perseus_Init_Error();
    snprintf( txt, sizeof(txt),
        _("Perseus perseus_set_adc() error:\n%s"),
        perseus_errorstr() );
    Error_Dialog( txt, QUIT );
    return( FALSE );
  }

  /* Set center frequency, enable preselector filters */
  ret = perseus_set_ddc_center_freq(
      descr, (double)rc_data.station_freq, TRUE );
  if( ret < 0 )
  {
    Perseus_Init_Error();
    Error_Dialog(
        _("Failed to set DDC Center Frequency"), OK );
    return( FALSE );
  }

  /* Allocate I/Q double buffers */
  size_t req = (size_t)PERSEUS_BUFFER_LEN * sizeof(double);
  if( demod_buf_i == NULL )
    mem_alloc( (void **)&demod_buf_i, req );
  if( demod_buf_q == NULL )
    mem_alloc( (void **)&demod_buf_q, req );

  /* Init semaphore */
  sem_init( &pback_semaphore, 0, 0 );

  /* Create a thread for async read from Perseus device */
  ret = pthread_create( &pthread_id, NULL, Perseus_Read_Async, NULL );
  if( ret != 0 )
  {
    Perseus_Init_Error();
    Error_Dialog(
        _("Failed to create Perseus Async Read thread"), QUIT );
    return( FALSE );
  }
  sleep( 1 );

  SetFlag( PERSEUS_INIT );

  return( TRUE );
} /* Perseus_Initialize() */

/*----------------------------------------------------------------------*/

