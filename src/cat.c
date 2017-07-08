/*  cat.c
 *
 *  Transceiver CAT control functions for the xwefax application
 */

/*
 *  xwefax: An application to decode Radio WEFAX signals from
 *  a Radio Receiver, through the computer's sound card.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

#include "cat.h"
#include "shared.h"

static char
  /* Command codes for Yaesu FT847/FT857 CAT */
  FT847_CAT_ON[]	= { 0, 0, 0, 0, (char)0x00 }, /*  Enable CAT */
  FT847_CAT_OFF[]	= { 0, 0, 0, 0, (char)0x80 }, /* Disable CAT */
  FT847_MAIN_VFO[]	= { 0, 0, 0, 0, (char)0x03 }, /* Read main VFO freq */
  FT847_SET_VFO[]	= { 0, 0, 0, 0, (char)0x01 }, /* Set main VFO freq  */

  /* Command codes for Elecraft K3 */
  K3_K31_S[]  = "K31;",				/* Set K3 CAT mode */
  K3_K31_G[]  = "K3;",				/* Request K3 CAT mode */
  K3_VFOA_S[] = "FAnnnnnnnnnnn;",	/* Set VFO A frequency */
  K3_VFOA_G[] = "FA;",				/* Get VFO A frequency */
  K3_MODE_G[] = "MD;",				/* Get transceiver mode */
  K3_MODE_S[] = "MDn;",				/* Set transceiver mode */
  K3_FLBW_G[] = "FW;",				/* Get filter bandwidth */
  K3_FLBW_S[] = "FWnnnn;",			/* Set filter bandwidth */
  K3_IFSH_G[] = "IS;",				/* Get IF shift */
  K3_IFSH_S[] = "IS nnnn;";			/* Set IF shift */

/* Serial port File descriptor */
static int serial_fd = 0;

/* Original serial port options */
static struct termios tcvr_serial_old_options;

/* Transceiver status buffer */
static tcvr_status_t tcvr_status;

/*------------------------------------------------------------------------*/

/*  Open_Tcvr_Serial()
 *
 *  Opens Tcvr's Serial Port device, returns the file
 *  descriptor tcvr_serial_fd on success or exits on error
 */

  gboolean
Open_Tcvr_Serial( void )
{
  struct termios new_options; /* New serial port options */
  struct flock lockinfo;      /* File lock information   */
  int error = 0;
  char test[5];

  /* Abort if serial already open */
  if( isFlagSet(CAT_SETUP) ) return( TRUE );

  /* Open Serial device, exit on error */
  Show_Message( _("Setting up TCVR CAT ..."), "black" );
  serial_fd = open( rc_data.cat_serial, O_RDWR | O_NOCTTY );
  if( serial_fd < 0 )
  {
	char mesg[MESG_SIZE];
	perror( rc_data.cat_serial );
	snprintf( mesg, sizeof(mesg),
		_("Failed to open %s\n"\
		  "Transceiver CAT not setup"),
		rc_data.cat_serial );
	Show_Message( mesg, "red" );
	Error_Dialog( mesg, OK );
	ClearFlag( ENABLE_CAT );
	serial_fd = 0;
	return( FALSE );
  }

  /* Attempt to lock entire Serial port device file */
  lockinfo.l_type   = F_WRLCK;
  lockinfo.l_whence = SEEK_SET;
  lockinfo.l_start  = 0;
  lockinfo.l_len    = 0;

  /* If Serial device is already locked, abort */
  error = fcntl( serial_fd, F_SETLK, &lockinfo );
  if( error == -1 )
  {
	char mesg[MESG_SIZE];
	error = fcntl( serial_fd, F_GETLK, &lockinfo );
	if( error != -1 )
	{
	  snprintf( mesg, sizeof(mesg),
		  _("Failed to lock %s\n"\
			"Device locked by pid %d\n"\
			"Transceiver CAT not setup"),
		  rc_data.cat_serial, lockinfo.l_pid );
	}
	else
	{
	  snprintf( mesg, sizeof(mesg),
		  _("Failed to lock %s\n"\
			"Transceiver CAT not setup"),
		  rc_data.cat_serial );
	}

	Show_Message( mesg, "red" );
	Error_Dialog( mesg, OK );
	Cancel_CAT();
	return( FALSE );
  }

  /* Save the current serial port options */
  error |= tcgetattr( serial_fd, &tcvr_serial_old_options );

  /* Read the current serial port options */
  error |= tcgetattr( serial_fd, &new_options );

  /* Set the i/o baud rates */
  switch( rc_data.tcvr_type )
  {
	case FT847:
	  error |= cfsetspeed( &new_options, B57600 );
	  break;

	case FT857: case K2: case K3:
	  error |= cfsetspeed( &new_options, B38400 );
	  break;
  }

  /* Set options for 'raw' I/O mode */
  cfmakeraw( &new_options );

  /* Setup read() timeout to .5 sec */
  new_options.c_cc[ VMIN ]  = 0;
  new_options.c_cc[ VTIME ] = 5;

  /* Setup the new options for the port */
  error |= tcsetattr( serial_fd, TCSAFLUSH, &new_options );
  if( error )
  {
	Show_Message( "Failed to Set Serial Port options", "red" );
	Error_Dialog( "Failed to Set Serial Port options", OK );
	Cancel_CAT();
	return( FALSE );
  }

  /* Enable CAT by testing comms with transceiver */
  SetFlag( TCVR_SERIAL_TEST );
  switch( rc_data.tcvr_type )
  {
	case FT847:
	  if( !Write_Tcvr_Serial(FT847_CAT_ON, sizeof(FT847_CAT_ON)) ||
		  !Write_Tcvr_Serial(FT847_MAIN_VFO, sizeof(FT847_MAIN_VFO)) ||
		  !Read_Tcvr_Serial(test, 5) )
	  {
		fprintf( stderr, "xwefax: failed to enable FT847 CAT\n" );
		Write_Tcvr_Serial( FT847_CAT_OFF, sizeof(FT847_CAT_OFF) );
		Show_Message( "Failed to enable FT847 CAT", "red" );
		Error_Dialog( "Failed to enable FT847 CAT", OK );
		Cancel_CAT();
		return( FALSE );
	  }
	  break;

	case FT857:
	  if( !Write_Tcvr_Serial(FT847_MAIN_VFO, sizeof(FT847_MAIN_VFO)) ||
		  !Read_Tcvr_Serial(test, 5) )
	  {
		fprintf( stderr, "xwefax: failed to enable FT857 CAT\n" );
		Show_Message( "Failed to enable FT857 CAT", "red" );
		Error_Dialog( "Failed to enable FT857 CAT", OK );
		Cancel_CAT();
		return( FALSE );
	  }
	  break;

	case K2:
	  /* Get K2 VFO A as a test of serial port */
	  if( !Write_Tcvr_Serial(K3_VFOA_G, sizeof(K3_VFOA_G)-1) ||
		  !Read_Tcvr_Serial(K3_VFOA_S, sizeof(K3_VFOA_S)-1) )
	  {
		fprintf( stderr, "xwefax: failed to enable K2 CAT\n" );
		Show_Message( "Failed to enable K2 CAT", "red" );
		Error_Dialog( "Failed to enable K2 CAT", OK );
		Cancel_CAT();
		return( FALSE );
	  }
	  break;

	case K3:
	  /* Set K3 to mode K31 as a test of serial port */
	  Strlcpy( K3_K31_S, "K31;", sizeof(K3_K31_S) );
	  if( !Write_Tcvr_Serial(K3_K31_S, sizeof(K3_K31_S)-1) )
	  {
		fprintf( stderr, "xwefax: failed to enable K3 CAT\n" );
		Show_Message( "Failed to enable K3 CAT", "red" );
		Error_Dialog( "Failed to enable K3 CAT", OK );
		Cancel_CAT();
		return( FALSE );
	  }
	  if( !Write_Tcvr_Serial(K3_K31_G, sizeof(K3_K31_G)-1) ||
		  !Read_Tcvr_Serial(K3_K31_S, sizeof(K3_K31_S)-1) ||
		  (strcmp(K3_K31_S, "K31;") != 0) )
	  {
		fprintf( stderr, "xwefax: failed to enable K3 CAT\n" );
		Show_Message( "Failed to enable K3 CAT", "red" );
		Error_Dialog( "Failed to enable K3 CAT", OK );
		Cancel_CAT();
		return( FALSE );
	  }
	  break;

  } /* switch( rc_data.tcvr_type ) */

  /* Save tcvr status */
  Tcvr_Status( GET_TCVR_STATUS, &tcvr_status );

  ClearFlag( TCVR_SERIAL_TEST );
  SetFlag( CAT_SETUP );
  Show_Message( _("TCVR CAT set up OK"), "green" );

  return( TRUE );
} /* Open_Tcvr_Serial() */

/*-------------------------------------------------------------------------*/

/*  Close_Tcvr_Serial()
 *
 *  Restore old options and close the Serial port
 */

  void
Close_Tcvr_Serial( void )
{
  if( isFlagSet(CAT_SETUP) )
  {
	if( isFlagClear(XWEFAX_QUIT) )
	  Show_Message( _("Closing down TCVR CAT"), "black" );

	/* Restore tcvr status */
	Tcvr_Status( SET_TCVR_STATUS, &tcvr_status );

	/* Disable CAT for FT847 */
	if( rc_data.tcvr_type == FT847 )
	  Write_Tcvr_Serial( FT847_CAT_OFF, sizeof(FT847_CAT_OFF) );

	/* Restore serial port and close */
	if( serial_fd > 0 )
	{
	  tcsetattr( serial_fd, TCSANOW, &tcvr_serial_old_options );
	  close( serial_fd );
	  serial_fd = 0;
	}

  } /* if( isFlagSet(CAT_SETUP) ) */

  ClearFlag( CAT_SETUP );
} /* End of Close_Tcvr_Serial() */

/*-------------------------------------------------------------------------*/

/* Cancel_CAT()
 *
 * Cancels Transceiver CAT (on error)
 */

  void
Cancel_CAT( void )
{
  rc_data.tcvr_type = NONE;
  ClearFlag( CAT_SETUP );
  ClearFlag( ENABLE_CAT );
  ClearFlag( TCVR_SERIAL_TEST );
  close( serial_fd );
  serial_fd = 0;
} /* Cancel_CAT() */

/*-------------------------------------------------------------------------*/

/* Tcvr_Status()
 *
 * Gets or Sets Transceiver status (only for the K2/K3 currently)
 */
  void
Tcvr_Status( int action, tcvr_status_t *status )
{
  /* For K2 or K3 */
  if( (rc_data.tcvr_type == K2) || (rc_data.tcvr_type == K3) )
  {
	/* Get transceiver status */
	if( action == GET_TCVR_STATUS )
	{
	  if( Write_Tcvr_Serial(K3_VFOA_G, sizeof(K3_VFOA_G)-1) &&
		  Read_Tcvr_Serial(K3_VFOA_S,  sizeof(K3_VFOA_S)-1) &&
		  Write_Tcvr_Serial(K3_MODE_G, sizeof(K3_MODE_G)-1) &&
		  Read_Tcvr_Serial(K3_MODE_S,  sizeof(K3_MODE_S)-1) &&
		  Write_Tcvr_Serial(K3_FLBW_G, sizeof(K3_FLBW_G)-1) &&
		  Read_Tcvr_Serial(K3_FLBW_S,  sizeof(K3_FLBW_S)-1) &&
		  Write_Tcvr_Serial(K3_IFSH_G, sizeof(K3_IFSH_G)-1) &&
		  Read_Tcvr_Serial(K3_IFSH_S,  sizeof(K3_IFSH_S)-1) )
	  {
		status->rig_mode  = atoi( K3_MODE_S+2 );
		status->vfo_freq  = atoi( K3_VFOA_S+2 );
		status->filter_bw = atoi( K3_FLBW_S+2 );
		status->if_shift  = atoi( K3_IFSH_S+2 );
		return;
	  }
	} /* if( action == GET_TCVR_STATUS ) */

	/* Set transceiver status */
	if( action == SET_TCVR_STATUS )
	{
	  snprintf( K3_MODE_S+2,  3,    "%d;", status->rig_mode );
	  snprintf( K3_VFOA_S+2, 13, "%011d;", status->vfo_freq );
	  snprintf( K3_FLBW_S+2, 6,   "%04d;", status->filter_bw );
	  snprintf( K3_IFSH_S+2, 7,  " %04d;", status->if_shift );

	  if( (Write_Tcvr_Serial(K3_VFOA_S, sizeof(K3_VFOA_S)-1) | (int)sleep(1) ) &&
		   Write_Tcvr_Serial(K3_MODE_S, sizeof(K3_MODE_S)-1) &&
		   Write_Tcvr_Serial(K3_VFOA_S, sizeof(K3_VFOA_S)-1) &&
		   Write_Tcvr_Serial(K3_FLBW_S, sizeof(K3_FLBW_S)-1) &&
		   Write_Tcvr_Serial(K3_IFSH_S, sizeof(K3_IFSH_S)-1) )
		return;

	} /* if( action == SET_TCVR_STATUS ) */
  } /* if( (rc_data.tcvr_type == K2) || (rc_data.tcvr_type == K2) ) */

  return;
} /* Tcvr_Status() */

/*-------------------------------------------------------------------------*/

/*  Read_Tcvr_Serial()
 *
 *  Reads Data from the Transceiver
 */

  gboolean
Read_Tcvr_Serial( char *data, size_t len )
{
  int nbytes = 0, cnt = 0;

  /* Check for data to be available */
  if( isFlagClear(TCVR_SERIAL_TEST) &&
	  isFlagClear(CAT_SETUP) )
	return( FALSE );
  while( ++cnt < MAX_SERIAL_RETRIES )
  {
	ioctl( serial_fd, FIONREAD, &nbytes);
	if( nbytes == (int)len ) break;
	fprintf( stderr,
		"xwefax: read(): incorrect number of bytes available:"
		" %d/%d\n", nbytes, (int)len );
	usleep(100000);
  }

  /* Error condition */
  if( cnt >= MAX_SERIAL_RETRIES )
  {
	tcflush( serial_fd, TCIOFLUSH );
	fprintf( stderr,
		"xwefax: read(): incorrect number of bytes available:"
		" %d/%d after %d tries\n", nbytes, (int)len, cnt );
	Show_Message( "Serial port error-CAT cancelled", "red" );
	Error_Dialog( "Serial port error-CAT cancelled", OK );
	data[0] = '\0';
	Cancel_CAT();
	return( FALSE );
  }

  /* Clear data buffer and read from serial */
  size_t ret = (size_t)read( serial_fd, data, len );

  /* Error condition */
  if( ret != len )
  {
	tcflush( serial_fd, TCIOFLUSH );
	fprintf( stderr,
		"xwefax: read() return value wrong: %d/%d\n",
		(int)ret, (int)len );
	Show_Message( "Serial port error-CAT cancelled", "red" );
	Error_Dialog( "Serial Port error-CAT cancelled", OK );
	data[0] = '\0';
	Cancel_CAT();
	return( FALSE );
  }

  return( TRUE );
} /* End of Read_Tcvr_Serial() */

/*-------------------------------------------------------------------------*/

/*  Write_Tcvr_Serial()
 *
 *  Writes a command to the Tranceiver
 */

  gboolean
Write_Tcvr_Serial( char *data, size_t len )
{
  int cnt = 0;
  size_t ret = 0;

  /* Flush serial and write command, retrying if needed */
  if( isFlagClear(TCVR_SERIAL_TEST) &&
	  isFlagClear(CAT_SETUP) )
	return( FALSE );
  tcflush( serial_fd, TCIOFLUSH );
  while( cnt++ < MAX_SERIAL_RETRIES )
  {
	ret = (size_t)write( serial_fd, data, len );
	if( ret == len ) break;
	fprintf( stderr,
		"xwefax: write(): incorrect number of bytes written:"
		" %d/%d\n", (int)ret, (int)len );
	usleep(100000);
  }

  /* Error condition */
   if( cnt >= MAX_SERIAL_RETRIES )
  {
	tcflush( serial_fd, TCIOFLUSH );
	fprintf( stderr,
		"xwefax: write(): incorrect number of bytes written:"
		" %d/%d after %d tries\n", (int)ret, (int)len, cnt );
	Show_Message( "Serial port error-CAT cancelled", "red" );
	Error_Dialog( "Serial port error-CAT cancelled", OK );
	Cancel_CAT();
	return( FALSE );
  }

  /* Give time to CAT to do its thing */
  usleep(50000);
  return( TRUE );

} /* End of Write_Tcvr_Serial() */

/*-------------------------------------------------------------------------*/

/*  Read_Rx_Freq()
 *
 *  Reads the Rx freq of the Yaesu transceiver.
 */

  gboolean
Read_Rx_Freq( int *freq )
{
  /* A char string for sending and receiving  */
  /* data strings to and from the transceiver */
  char frq[5];

  /* Abort if CAT disabled */
  if( isFlagClear(CAT_SETUP) ) return( TRUE );

  switch( rc_data.tcvr_type )
  {
	case FT847: case FT857:
	/* Read Rx frequency */
	memset( frq, 0, sizeof(frq) );
	if( !Write_Tcvr_Serial(FT847_MAIN_VFO, sizeof(FT847_MAIN_VFO)) ||
		!Read_Tcvr_Serial(frq, sizeof(frq)) )
	  return( FALSE );

	/* Decode frequency data to 10Hz */
	*freq = 0;
	*freq += (frq[0] & 0xF0) >> 4;
	*freq *= 10;
	*freq += frq[0] & 0x0F;
	*freq *= 10;
	*freq += (frq[1] & 0xF0) >> 4;
	*freq *= 10;
	*freq += frq[1] & 0x0F;
	*freq *= 10;
	*freq += (frq[2] & 0xF0) >> 4;
	*freq *= 10;
	*freq += frq[2] & 0x0F;
	*freq *= 10;
	*freq += (frq[3] & 0xF0) >> 4;
	*freq *= 10;
	*freq += frq[3] & 0x0F;
	*freq *= 10;
	break;

	case K2: case K3:
	/* Read Rx frequency */
	if( !Write_Tcvr_Serial(K3_VFOA_G, sizeof(K3_VFOA_G)-1) ||
		!Read_Tcvr_Serial(K3_VFOA_S, sizeof(K3_VFOA_S)-1) )
	  return( FALSE );
	*freq = atoi( K3_VFOA_S + 2 );
	break;

  } /* switch( rc_data.tcvr_type ) */

  return( TRUE );
} /* End of Read_Rx_Freq() */

/*------------------------------------------------------------------------*/

/*  Write_Rx_Freq()
 *
 *  Writes the Rx freq to the Yaesu FT-847 transceiver.
 */

  gboolean
Write_Rx_Freq( int freq )
{
  /* Buffer used for converting freq. to string */
  char freq_buff[9];
  int idx; /* Index for loops etc */


  /* Abort if CAT disabled */
  if( isFlagClear(CAT_SETUP) ) return( TRUE );

  switch( rc_data.tcvr_type )
  {
	case FT847: case FT857:
	  /* Set Rx frequency */
	  snprintf( freq_buff, sizeof(freq_buff), "%08d", freq/10 );
	  for( idx = 0; idx < 4; idx++ )
	  {
		FT847_SET_VFO[idx]  = (char)((freq_buff[2*idx]   - '0') << 4);
		FT847_SET_VFO[idx] |= (freq_buff[2*idx+1] - '0');
	  }
	  if( !Write_Tcvr_Serial(FT847_SET_VFO, sizeof(FT847_SET_VFO)) )
		return( FALSE );
	  break;

	case K2: case K3:
	  /* Set Rx frequency */
	  snprintf( K3_VFOA_S+2, sizeof(K3_VFOA_S)-2, "%011d;", freq );
	  if( !Write_Tcvr_Serial(K3_VFOA_S, sizeof(K3_VFOA_S)-1) )
		return( FALSE );

	  /* Set mode to USB */
	  if( strcmp( rc_data.station_sideband, "LSB") == 0 )
		snprintf( K3_MODE_S+2, sizeof(K3_MODE_S)-2, "%d;", 1 );
	  else
		snprintf( K3_MODE_S+2, sizeof(K3_MODE_S)-2, "%d;", 2 );
	  if( !Write_Tcvr_Serial(K3_MODE_S, sizeof(K3_MODE_S)-1) )
		return( FALSE );

	  /* Set filter bandwidth */
	  snprintf( K3_FLBW_S+2, sizeof(K3_FLBW_S)-2, "%04d;", 140 );
	  if( !Write_Tcvr_Serial(K3_FLBW_S, sizeof(K3_FLBW_S)-1) )
		return( FALSE );

	  /* Set IF shift */
	  snprintf( K3_IFSH_S+2, sizeof(K3_IFSH_S)-2, "% 04d;", 1900 );
	  if( !Write_Tcvr_Serial(K3_IFSH_S, sizeof(K3_IFSH_S)-1) )
		return( FALSE );
	  break;

  } /* switch( rc_data.tcvr_type ) */

  return( TRUE );
} /* End of Write_Rx_Freq() */

/*-------------------------------------------------------------------------*/

/* Set_Rx_Freq_Idle_Cb()
 *
 * Used as an idle callback when Rx freq and mode are set
 */
  gboolean
Set_Rx_Freq_Idle_Cb( gpointer data )
{
  /* Set up Perseus SDR center frequency */
  if( rc_data.tcvr_type == PERSEUS )
  {
	/* Init Perseus if needed */
	if( isFlagClear(PERSEUS_INIT) )
	{
	  if( !Perseus_Initialize() )
		return( FALSE );
	}

	Perseus_Set_Center_Frequency( rc_data.station_freq );
	return( FALSE );
  }

  /* Setup CAT if enabled */
  if( isFlagSet(ENABLE_CAT) && isFlagClear(CAT_SETUP) )
	Open_Tcvr_Serial();

  /* Set Rx frequency */
  if( isFlagSet(CAT_SETUP) )
	Write_Rx_Freq( rc_data.station_freq );

  return( FALSE );
} /* Set_Rx_Freq_Idle_Cb() */

/*------------------------------------------------------------------------*/

/* Tune_Tcvr()
 *
 * Tunes the transceiver to the frequency of the strongest
 * signal near a mouse click in the waterfall window
 */

  gboolean
Tune_Tcvr( double x )
{
  int
	from, to,	/* Range to scan for a max bin value  */
	bin_max,	/* Max bin value found in this range  */
	max_idx,	/* dft idx at which the max is found */
	dft_idx,	/* Idx used to search dft bin values */
	tcvr_freq,	/* Transceiver's Rx frequency */
	audio_freq;	/* Audio frequency in waterfal window */


  /* Calculate dft index corresponding
   * to pointer x in waterfall window */
  dft_idx = (int)(x - 0.5);

  /* Look above and below click point for max bin val */
  from = dft_idx - 10;
  if( from < 0 ) from = 0;
  to = dft_idx + 10;
  if( to >= gbl_wfall_width ) to = gbl_wfall_width - 1;

  /* Find max bin value around click point */
  bin_max = 0;
  max_idx = dft_idx;
  for( dft_idx = from; dft_idx < to; dft_idx++ )
	if( bin_max < gbl_bin_ave[dft_idx] )
	{
	  bin_max = gbl_bin_ave[dft_idx];
	  max_idx = dft_idx;
	}

  /* Audio freq. corresponding to dft index */
  audio_freq  = ( DFT_UPPER_FREQ - DFT_LOWER_FREQ ) * max_idx;
  audio_freq /= gbl_wfall_width;
  audio_freq += DFT_LOWER_FREQ;

  /* Adjust receiver frequency */
  if( rc_data.tcvr_type == PERSEUS )
  {
	rc_data.station_freq += audio_freq - rc_data.white_freq;
	Perseus_Set_Center_Frequency( rc_data.station_freq );
  }
  else
  {
	/* Abort if CAT not set up */
	if( isFlagClear(CAT_SETUP) ) return( TRUE );

	/* Read current Rx frequency */
	if( !Read_Rx_Freq(&tcvr_freq) ) return( FALSE );

	/* Add correction to TCVR frequency */
	tcvr_freq += audio_freq - rc_data.white_freq;

	/* Calculate and write Rx freq. to center audio on AUDIO_FREQUENCY */
	if( !Write_Rx_Freq(tcvr_freq) ) return( FALSE );
  }

  return( TRUE );
} /* Tune_Tcvr() */

/*-------------------------------------------------------------------------*/

