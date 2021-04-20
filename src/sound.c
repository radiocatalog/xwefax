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

#include "sound.h"
#include "shared.h"

static int
  recv_buffer_size, /* Recv DSP signal samples buffer   */
  recv_buffer_idx;  /* Index to Rx signal samples buffer*/

/* Receive samples buffer */
static short *recv_buffer = NULL;

/* ALSA pcm capture and mixer handles */
static snd_pcm_t *capture_handle  = NULL;
static snd_mixer_t *mixer_handle  = NULL;
static snd_pcm_hw_params_t *hw_params = NULL;

/* Simple mixer elements for setting
 * up capture sources and volume */
static snd_mixer_elem_t *cap_elem = NULL;

/*------------------------------------------------------------------------*/

/* Open_PCM()
 *
 * Opens a pcm device for a given handle
 */
  static gboolean
Open_PCM(
    snd_pcm_t **handle,
    snd_pcm_stream_t stream,
    char *mesg, int *error )
{
  /*** Open/setup dsp/pcm and mixer ***/
  Show_Message( _("Setting up Sound Card ... "), "black" );
  Show_Message( _("Opening PCM Device ... "), "black" );

  /* Open pcm */
  *error = snd_pcm_open(
      handle, rc_data.pcm_dev, stream, PCM_OPEN_MODE );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot open sound device "), MESG_SIZE );
    Strlcat( mesg, rc_data.pcm_dev, MESG_SIZE );
    return( FALSE );
  }

  /* Allocate memory to hardware parameters structure */
  *error = snd_pcm_hw_params_malloc( &hw_params );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot allocate hw_params struct"), MESG_SIZE );
    return( FALSE );
  }

  /* Initialize hardware parameter structure */
  *error = snd_pcm_hw_params_any( *handle, hw_params );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot initialize hw_params struct"), MESG_SIZE );
    return( FALSE );
  }

  /* Set access type */
  *error = snd_pcm_hw_params_set_access(
      *handle, hw_params, SND_PCM_ACCESS );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot set PCM access type"), MESG_SIZE );
    return( FALSE );
  }

  /* Set sample format */
  *error = snd_pcm_hw_params_set_format(
      *handle, hw_params, SND_PCM_FORMAT );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot set sample format"), MESG_SIZE );
    return( FALSE );
  }

  /* Set sample rate */
  *error = snd_pcm_hw_params_set_rate(
      *handle, hw_params, (unsigned int)rc_data.dsp_rate, EXACT_VAL );
  if( *error < 0 )
  {
    snprintf( mesg, MESG_SIZE,
        _("Cannot set sample rate to %d"), rc_data.dsp_rate );
    return( FALSE );
  }

  /* Set channel count */
  *error = snd_pcm_hw_params_set_channels(
      *handle, hw_params, (unsigned int)rc_data.num_chn );
  if( *error < 0 )
  {
    snprintf( mesg, MESG_SIZE,
        _("Cannot set channel count to %d"), rc_data.num_chn );
    return( FALSE );
  }

  /* Set number of periods */
  *error = snd_pcm_hw_params_set_periods(
      *handle, hw_params, NUM_PERIODS, EXACT_VAL );
  if( *error < 0)
  {
    snprintf( mesg, MESG_SIZE,
        _("Cannot set number periods to %d"), NUM_PERIODS );
    return( FALSE );
  }

  /* Set period size */
  *error = snd_pcm_hw_params_set_period_size(
      *handle, hw_params, PERIOD_SIZE, EXACT_VAL );
  if( *error < 0)
  {
    snprintf( mesg, MESG_SIZE,
        _("Cannot set period size to %d"), PERIOD_SIZE );
    return( FALSE );
  }

  /* Set parameters */
  *error = snd_pcm_hw_params( *handle, hw_params );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot set capture parameters"), MESG_SIZE );
    return( FALSE );
  }
  snd_pcm_hw_params_free( hw_params );
  hw_params = NULL;

  /* Prepare audio interface for use */
  *error = snd_pcm_prepare( *handle );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot prepare audio interface"), MESG_SIZE );
    return( FALSE );
  }

  Show_Message( _("PCM Device opened OK"), "green" );
  return( TRUE );
} /* Open_PCM() */

/*------------------------------------------------------------------------*/

/* Open_Mixer()
 *
 * Opens mixer interface
 */
  static gboolean
Open_Mixer( char *mesg, int *error )
{
  snd_mixer_elem_t *elem;
  snd_mixer_selem_id_t *sid;

  /* Abort if mixer already setup */
  if( isFlagSet(MIXER_SETUP) ) return( TRUE );

  /* Open mixer handle */
  Show_Message( _("Opening Mixer Device ..."), "black" );
  *error = snd_mixer_open( &mixer_handle, 0 );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot open mixer handle"), MESG_SIZE );
    return( FALSE );
  }

  /* Attach mixer */
  *error = snd_mixer_attach( mixer_handle, rc_data.snd_card );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot attach mixer to "), MESG_SIZE );
    Strlcat( mesg, rc_data.snd_card, MESG_SIZE );
    return( FALSE );
  }

  /* Register mixer */
  *error = snd_mixer_selem_register( mixer_handle, NULL, NULL );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot register mixer"), MESG_SIZE );
    return *error;
  }

  /* Load mixer */
  *error = snd_mixer_load( mixer_handle );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot load mixer"), MESG_SIZE );
    return( FALSE );
  }

  /* Allocate selem_id structure */
  *error = snd_mixer_selem_id_malloc( &sid );
  if( *error < 0 )
  {
    Strlcat( mesg, _("Cannot allocate selem_id struct"), MESG_SIZE );
    return( FALSE );
  }

  /* Find capture source selem */
  snd_mixer_selem_id_set_index( sid, 0 );
  snd_mixer_selem_id_set_name( sid, rc_data.cap_src );
  elem = snd_mixer_find_selem( mixer_handle, sid );
  if( elem == NULL )
  {
    Strlcat( mesg, _("Cannot find capture source element "), MESG_SIZE );
    Strlcat( mesg, rc_data.cap_src, MESG_SIZE );
    snd_mixer_selem_id_free(sid);
    *error = 0;
    return( FALSE );
  }

  /* Set capture switch for capture source */
  if( snd_mixer_selem_has_capture_switch(elem) )
  {
    *error = snd_mixer_selem_set_capture_switch(
        elem, rc_data.channel, 1 );
    if( *error < 0 )
    {
      Strlcat( mesg, _("Cannot set capture device "), MESG_SIZE );
      Strlcat( mesg, rc_data.cap_src, MESG_SIZE );
      snd_mixer_selem_id_free(sid);
      return( FALSE );
    }
  }
  else
  {
    snprintf( mesg, MESG_SIZE,
        _("Device %s does not have Capture capability"),
        rc_data.cap_src );
    snd_mixer_selem_id_free(sid);
    *error = 0;
    return( FALSE );
  }

  /* Find capture volume selem if not -- */
  if( strcmp(rc_data.cap_vol, "--") != 0 )
  {
    snd_mixer_selem_id_set_index( sid, 0 );
    snd_mixer_selem_id_set_name( sid, rc_data.cap_vol );
    cap_elem = snd_mixer_find_selem( mixer_handle, sid );
    if( !cap_elem )
    {
      snprintf( mesg, MESG_SIZE,
          _("Cannot find volume element %s\n"),
          rc_data.cap_vol );
      Error_Dialog( mesg, OK );
    }
  }

  Show_Message( _("Mixture Device opened OK"), "green" );
  SetFlag( MIXER_SETUP );
  snd_mixer_selem_id_free( sid );

  return( TRUE );
} /* Open_Mixer() */

/*------------------------------------------------------------------------*/

/* Set_Capture_Level()
 *
 * Sets Capture Control level
 */
  static gboolean
Set_Capture_Level( int level, char *mesg, int *error )
{
  long cmin, cmax;

  /* Abort with no error if Mixer not setup */
  if( mixer_handle == NULL ) return( TRUE );

  /* Set capture volume */
  Show_Message( _("Setting up Capture Level ..."), "black" );
  if( cap_elem != NULL )
  {
    if( snd_mixer_selem_has_capture_volume(cap_elem) )
    {
      /* Change from % volume to sound card value */
      long lev;
      snd_mixer_selem_get_capture_volume_range(
          cap_elem, &cmin, &cmax );
      lev = cmin + ((cmax - cmin) * level) / 100;

      /* Set capture volume */
      *error = snd_mixer_selem_set_capture_volume(
          cap_elem, rc_data.channel, lev );
      if( *error < 0 )
      {
        snprintf( mesg, MESG_SIZE,
            _("Cannot set capture volume to %d\n"\
              "Error: %s"),
            level, snd_strerror(*error) );
        Show_Message( mesg, "red" );
        Error_Dialog( mesg, OK );
        return( FALSE );
      }
    }
  } /* if( cap_elem != NULL ) */

  Show_Message( _("Capture Level set up OK"), "green" );

  return( TRUE );
} /* Set_Capture_Level() */

/*------------------------------------------------------------------------*/

/* Open_Capture()
 *
 * Opens sound card for Capture
 */
  gboolean
Open_Capture( char *mesg, int *error )
{
  /* Return if Capture is setup */
  if( isFlagSet(CAPTURE_SETUP) ) return( TRUE );

  /* Open & setup pcm for Capture */
  Show_Message( _("Opening Capture Device ..."), "black" );
  if( !Open_PCM(
        &capture_handle,
        SND_PCM_STREAM_CAPTURE,
        mesg, error) )
    return( FALSE );

  /* Size of receive samples buffer in 'shorts' */
  recv_buffer_size = PERIOD_SIZE * rc_data.num_chn;

  /* Index to recv samples buffer (set to end) */
  recv_buffer_idx = recv_buffer_size;

  /* Allocate memory to receive samples buffer */
  if( recv_buffer == NULL )
  {
    size_t alloc = (size_t)recv_buffer_size * sizeof(short);
    if( !mem_alloc((void **)&recv_buffer, alloc) )
    {
      Strlcat( mesg, _("Memory allocation failed - Quit"), MESG_SIZE );
      return( FALSE );
    }
    memset( recv_buffer, 0, alloc );
  }

  /* Open mixer & set playback voulume, abort on failure.
   * Failure to set volume level is not considered fatal */
  if( !Open_Mixer(mesg, error) ) return( FALSE );
  Set_Capture_Level( rc_data.cap_lev, mesg, error );
  Show_Message( _("Capture Device opened OK"), "green" );
  SetFlag( CAPTURE_SETUP );

  return( TRUE );
} /* Open_Capture() */

/*------------------------------------------------------------------------*/

/* Close_Mixer()
 *
 * Closes down sound card mixer control
 */
  static void
Close_Mixer( void )
{
  if( isFlagClear(XWEFAX_QUIT) )
    Show_Message( _("Closing down Sound Mixer"), "black" );

  if( mixer_handle != NULL ) snd_mixer_close( mixer_handle );
  mixer_handle = NULL;
  ClearFlag(MIXER_SETUP);
} /* Close_Mixer() */

/*------------------------------------------------------------------------*/

/* Close_Capture()
 *
 * Closes down capture from sound card
 */
  void
Close_Capture( void )
{
  if( isFlagClear(XWEFAX_QUIT) )
    Show_Message( _("Closing down Sound Capture"), "black" );

  if( capture_handle != NULL )
    snd_pcm_close( capture_handle );
  capture_handle = NULL;

  if( hw_params != NULL ) snd_pcm_hw_params_free( hw_params );
  hw_params = NULL;

  Close_Mixer();

  ClearFlag(CAPTURE_SETUP);
} /* Close_Capture() */

/*------------------------------------------------------------------------*/

/* Xrun_Recovery()
 *
 * Recover from underrrun (broken pipe) and suspend
 */
  static gboolean
Xrun_Recovery( snd_pcm_t *handle, int error )
{
  char mesg[MESG_SIZE];
  if( error == -EPIPE )
  {
    error = snd_pcm_prepare( handle );
    if( error < 0 )
    {
      snprintf( mesg, sizeof(mesg),
          _("Cannot recover from underrun\n"
            "Prepare failed\n"\
            "Error: %s\n"), snd_strerror(error) );
      Show_Message( mesg, "red" );
      Error_Dialog( mesg, QUIT );
      return( FALSE );
    }
  }
  else if( error == -ESTRPIPE )
  {
    while( (error = snd_pcm_resume(handle)) == -EAGAIN ) sleep(1);
    if( error < 0 )
    {
      error = snd_pcm_prepare( handle );
      if( error < 0 )
      {
        snprintf( mesg, sizeof(mesg),
            _("Cannot recover from suspend\n"
              "Prepare failed\n"\
              "Error: %s\n"), snd_strerror(error) );
        Show_Message( mesg, "red" );
        Error_Dialog( mesg, QUIT );
        return( FALSE );
      }
    }
  }

  return( TRUE );
} /* Xrun_Recovery() */

/*------------------------------------------------------------------------*/

/*  Signal_Sample()
 *
 *  Gets the next DSP sample of the signal input.
 */

  gboolean
Sound_Signal_Sample( short *sample_val )
{
  snd_pcm_sframes_t error;

  /* Three consecutive signal samples */
  static int s1 = 0, s2 = 0, s3 = 0;

  /* Refill recv DSP samples buffer when needed */
  if( recv_buffer_idx >= recv_buffer_size )
  {
    /* Read audio samples from DSP, abort on error */
    error = snd_pcm_readi( capture_handle, recv_buffer, PERIOD_SIZE );
    if( error != PERIOD_SIZE )
    {
      fprintf( stderr, "xwefax: Signal_Sample(): %s\n",
          snd_strerror((int)error) );

      /* Try to recover from error */
      if( !Xrun_Recovery(capture_handle, (int)error) )
        return( FALSE );
    } /* if( error  ) */

    /* Start buffer index according to stereo/mono mode */
    recv_buffer_idx = rc_data.use_chn;

  } /* End of if( recv_buffer_idx >= recv_buffer_size ) */

  /* Get next signal sample */
  s3 = (int)recv_buffer[recv_buffer_idx];

  /* There seems to be a glitch somewhere in my sound system
   * which produces a rogue DSP sample from time to time, that
   * is of opposite sign to the surrounding samples. This upsets
   * the operation of the signal detector functions. the code
   * below replaces a rogue sample with the average of samples
   * eiter side of it, to reduce glitches in signal detectors
   */
  if( (s1 * s2 < 0) && (s2 * s3 < 0) ) s2 = ( s1 + s3 ) / 2;

  /* Take the value of the middle sample */
  *sample_val = (short)s2;

  /* Shift signal samples */
  s1 = s2;
  s2 = s3;

  /* Increment according to mono/stereo mode */
  recv_buffer_idx += rc_data.num_chn;

  /* Produces simulated alternate black/white lines
  {
    static double ww = M_2PI * 2300.0 / 48000.0;
    static double wb = M_2PI * 1500.0 / 48000.0;
    static double w = 0.0;
    static int cn = 2400 * 20;
    static int idx = 0, lc = 0;

    if( lc )
    {
      idx++;
      if( idx >= cn )
      {
        idx = 0;
        lc = 0;
      }
      *sample_val = (short)( 30000.0 * sin(w) );
      w += ww;
      if( w > M_2PI ) w -= M_2PI;
    }
    else
    {
      idx++;
      if( idx >= cn )
      {
        idx = 0;
        lc = 1;
      }
      *sample_val = (short)(30000.0 * sin(w));
      w += wb;
      if( w > M_2PI ) w -= M_2PI;
    }
  } */

  /* Produces simulated phasing lines
  {
    static double ww = M_2PI * 2300.0 / 48000.0;
    static double wb = M_2PI * 1500.0 / 48000.0;
    static double w = 0.0;
    static int cn1 = 55 * 20;
    static int cn2 = 1145 * 20;
    static int idx = 0, lc = 0;

    if( lc )
    {
      idx++;
      if( idx >= cn1 )
      {
        idx = 0;
        lc = 0;
      }
      *sample_val = (short)( 30000.0 * sin(w) );
      w += ww;
      if( w > M_2PI ) w -= M_2PI;
    }
    else
    {
      idx++;
      if( idx >= cn2 )
      {
        idx = 0;
        lc = 1;
      }
      *sample_val = (short)(30000.0 * sin(w));
      w += wb;
      if( w > M_2PI ) w -= M_2PI;
    }
  } */

  /* Produces a checkerboard pattern
  {
    static double ww = M_2PI * 2300.0 / 48000.0;
    static double wb = M_2PI * 1500.0 / 48000.0;
    static double w = 0.0;
    static int idx = 0, col = 0, pix = 0;
    double npix = 2;

    if( col )
    {
      *sample_val = (short)( 30000.0 * sin(w) );
      w += ww;
      if( w > M_2PI ) w -= M_2PI;

      idx++;
      if( (double)idx >= npix * (double)rc_data.pixel_len )
      {
        idx = 0;
        col = 0;
        pix++;
      }
    }
    else
    {
      *sample_val = (short)( 30000.0 * sin(w) );
      w += wb;
      if( w > M_2PI ) w -= M_2PI;

      idx++;
      if( (double)idx >= npix * (double)rc_data.pixel_len )
      {
        idx = 0;
        col = 1;
        pix++;
      }
    }
    if( pix >= rc_data.pixels_per_line )
    {
      pix = 0;
      if( col ) col = 0;
      else col = 1;
    }
  } */

  /* Decimate sample values for the DFT */
  DFT_Input_Data( *sample_val );

  return( TRUE );
} /* End of Signal_Sample() */

/*------------------------------------------------------------------------*/

