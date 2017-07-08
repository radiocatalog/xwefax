/*  common.h
 *
 *  Common objects for xwefax application
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


#ifndef COMMON_H
#define COMMON_H	1

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>
#include <perseus-sdr.h>
#include <semaphore.h>

/* Generel definitions for image processing */
#define MAX_FILE_NAME      255 /* Max length for optional filenames */

/* Size of char arrays (strings) for error messages etc */
#define MESG_SIZE	128

/* DFT parameter definitions */
#define DFT_INPUT_SIZE	 2550 /* Size of dft input samples array */
#define DFT_UPPER_FREQ	 2400 /* Frequency at upper end of DFT display */
#define DFT_LOWER_FREQ	 1200 /* Frequency at lower end of DFT display */
#define DFT_FREQ_MULTP		2 /* Ratio of above frequencies for DFT display */

/* Flow control flags */
#define CAPTURE_SETUP	 0x00000001 /* Sound card capture has been set up */
#define MIXER_SETUP		 0x00000002 /* Sound card Mixer has been set-up */
#define RECEIVE_STOP	 0x00000004 /* Stop WEFAX reception and clean up */
#define SKIP_ACTION		 0x00000008 /* Skip current action */
#define INIMAGE_PHASING  0x00000010 /* Enable In-Image phasing pulse detection */
#define SAVE_IMAGE_PGM	 0x00000020 /* Save the image buffer to PGM file */
#define SAVE_IMAGE_JPG	 0x00000040 /* Save the image buffer to JPG file */
#define ENABLE_CAT		 0x00000080 /* Enable CAT for transceiver */
#define CAT_SETUP		 0x00000100 /* CAT is set up */
#define TCVR_SERIAL_TEST 0x00000200 /* Serial port is under test */
#define XWEFAX_QUIT		 0x00000400 /* Xwefax in quit sequence */
#define SAVE_STATIONS	 0x00000800 /* Xwefax in quit sequence */
#define DISPLAY_SIGNAL   0x00001000 /* Display detector output */
#define START_NEW_IMAGE  0x00002000 /* Restart WEFAX image decoder after params change */
#define SAVE_IMAGE		 0x00004000 /* Enable saving of WEFAX image */
#define PERSEUS_INIT	 0x00008000 /* Perseus receiver initialized */

/* Wefax control flags */
enum
{
  ACTION_BEGIN = 1,	/* Begin the WEFAX Image decoding process */
  ACTION_START,		/* Look for the WEFAX Start tone */
  ACTION_PHASING,	/* Sync with the WEFAX phasing pulse */
  ACTION_DECODE,	/* Decode the WEFAX image */
  ACTION_STOP		/* Stop operations */
};

/* Status Indicator Icons */
enum
{
  ICON_START_YES = 1,
  ICON_SYNC_YES,
  ICON_DECODE_YES,
  ICON_SAVE_YES,
  ICON_START_NO,
  ICON_SYNC_NO,
  ICON_DECODE_NO,
  ICON_SAVE_NO,
  ICON_START_SKIP,
  ICON_SYNC_SKIP,
  ICON_DECODE_SKIP,
  ICON_SAVE_SKIP,
  ICON_START_APPLY,
  ICON_SYNC_APPLY,
  ICON_DECODE_APPLY,
  ICON_SAVE_APPLY
};

enum
{
  ENHANCE_NONE = 0,
  ENHANCE_CONTRAST,
  ENHANCE_BILEVEL
};

#define SUCCESS		1
#define ERROR		0

/* For the Error Dialog */
#define QUIT	TRUE
#define OK		FALSE

/* Some character definitions */
#define HT		0x09 /* Horizontal Tab  */
#define CR		0x0d /* Carriage Return */
#define LF		0x0a /* Line Feed */

/* WEFAX start/stop tones */
#define IOC576_START_TONE	300
#define IOC288_START_TONE	675
#define WEFAX_STOP_TONE		450

/* Length of phasing pulse in pixels */
#define PHASING_PULSE_LEN	55

/* Length of phasing pulse sliding window */
#define PHASING_PUSLE_WIN	32.0

/* Value needed to position in-image
 * phasing pulse at beginning of line */
#define PHASING_PULSE_REF	40

/* Length of line buffer for reading files */
#define LINE_BUFF_LEN	80

/* Stations treeview up-down selection */
#define SELECT_ROW_UP		1
#define SELECT_ROW_DOWN		2

/* Should have been in math.h */
#ifndef M_2PI
  #define M_2PI		6.28318530717958647692
#endif

/* Transceiver or Receiver type */
enum
{
  NONE = 0,
  PERSEUS,
  RADIO,	/* One of the 4 Transceivers below */
  FT847,
  FT857,
  K2,
  K3
};

/* Runtime config data */
typedef struct rc_data
{
  char
	snd_card[32], /* Sound card name */
	pcm_dev[32],  /* PCM device name */
	cap_src[32],  /* Capture source  */
	cap_vol[32];  /* Capture volume control  */

  int
	channel,	/* ALSA "channel" for use in mixer setup */
	num_chn,	/* Number of audio channels (2=stereo, 1=mono) */
	use_chn,	/* Channel in use (frontleft=0, frontright=1 etc) */
	cap_lev,	/* Recording/Capture level */
	dsp_rate;	/* DSP rate (speed) samples/sec */

  double dft_stride; /* DFT stride over input data (dsp samples) */

  /* Transceiver serial device */
  char cat_serial[32];

  /* Transceiver or Receiver type 1=FT847 2=FT857 3=K2 4=K3 5=PERSEUS */
  int tcvr_type;

  /* DSP buffer size */
  guint	dsp_buf_size;

  /* xwefax working directory and stations file */
  char xwefax_dir[64];
  char stations_file[64];

  /* Set default widow height */
  int window_height;

  /* WEFAX station Sideband (USB or LSB) */
  char station_sideband[4];

  int
	black_freq,			/* The black color freq, normally 1500 Hz */
	white_freq,			/* The white color freq, normally 2300 Hz */
	image_lines,		/* Maximum number of lines to decode */
	pixels_per_line,	/* Image resolution pixels/line */
	pixels_per_line2,	/* Image resolution pixels/line / 2 */
	stop_tone,			/* WEFAX stop tone, normally 450 Hz */
	start_tone,			/* WEFAX start tone, depends on IOC value */
	ioc_value,			/* IOC value specified in xwefaxrc */
	phasing_lines,		/* Number of phasing pulse lines to use for sync */
	image_enhance,		/* Image enhancement algorithm number */
	line_buffer_size,	/* Size of buffer for the pixels of 2 lines */
	station_freq;		/* Frequency in Hz of WEFAX station */

  double
	pixel_len,			/* Duration of a pixel in DSP samples */
	lines_per_min,		/* Line transmission rate */
	sync_slant,			/* Difference in scan speed in pix/line */
	start_tone_period,	/* Period of start tone in DSP samples */
	stop_tone_period;	/* Period of stop tone in DSP samples  */

  /* Center frequency of Perseus receiver */
  double perseus_freq_correction;
  int    perseus_rate_correction;

} rc_data_t;

/* Filter data struct */
typedef struct filter_data
{
  double
	cutoff,	/* Cutoff frequency as fraction of sample rate */
	ripple;	/* Passband ripple as a percentage */

  int
	npoles,	/* Number of poles, _must_ be even */
	type;	/* Filter type, as below this struct */

  /* a and b Coefficients of the filter */
  double *a, *b;

  /* Saved input and output values */
  double *x, *y;

  /* Ring buffer index for above */
  int ring_idx;

  /* Input samples buffer and its length */
  double *samples_buf;
  int samples_buf_len;

} filter_data_t;

/* Filter type for above struct */
enum
{
  FILTER_LOWPASS = 0,
  FILTER_HIGHPASS,
  FILTER_BANDPASS
};

/* Transceiver status data */
typedef struct
{
  int
	vfo_freq,	/* VFO A frequency  */
	rig_mode,	/* Transceiver mode (usb|cw etc) */
	filter_bw,
	if_shift;
} tcvr_status_t;

/** Extensible byte buffer */
typedef struct
{
  uint8_t *stream; /* byte buffer */
  int len;         /* current length */
  int siz;         /* maximum size */
} jpec_buffer_t;

/** Structure used to hold and process an image 8x8 block */
typedef struct
{
  float dct[64];            /* DCT coefficients */
  int quant[64];            /* Quantization coefficients */
  int zz[64];               /* Zig-Zag coefficients */
  int len;                  /* Length of Zig-Zag coefficients */
} jpec_block_t;

/** Entropy coding data that hold state along blocks */
typedef struct
{
  int32_t buffer;             /* bits buffer */
  int nbits;                  /* number of bits remaining in buffer */
  int dc;                     /* DC coefficient from previous block (or 0) */
  jpec_buffer_t *buf;         /* JPEG global buffer */
} jpec_huff_state_t;

/** Type of an Huffman JPEG encoder */
typedef struct
{
  jpec_huff_state_t state;    /* state from previous block encoding */
} jpec_huff_t;

/** Skeleton for an Huffman entropy coder */
typedef struct
{
  void *opq;
  void (*del)(void *opq);
  void (*encode_block)(void *opq, jpec_block_t *block, jpec_buffer_t *buf);
} jpec_huff_skel_t;

/** JPEG encoder */
typedef struct
{
  /** Input image data */
  const uint8_t *img;                   /* image buffer */
  uint16_t w;                           /* image width */
  uint16_t h;                           /* image height */
  uint16_t w8;                          /* w rounded to upper multiple of 8 */

  /** JPEG extensible byte buffer */
  jpec_buffer_t *buf;

  /** Compression parameters */
  int qual;                             /* JPEG quality factor */
  int dqt[64];                          /* scaled quantization matrix */

  /** Current 8x8 block */
  int bmax;                             /* maximum number of blocks (N) */
  int bnum;                             /* block number in 0..N-1 */
  uint16_t bx;                          /* block start X */
  uint16_t by;                          /* block start Y */
  jpec_block_t block;                   /* block data */

  /** Huffman entropy coder */
  jpec_huff_skel_t *hskel;
} jpec_enc_t;


/*** Function Prototypes created by cproto */
/* callbacks.c */
void Error_Dialog(char *mesg, gboolean hide);
gboolean on_main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_main_window_destroy(GtkObject *object, gpointer user_data);
void on_scope_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
void on_spectrum_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data);
gboolean on_spectrum_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
gboolean on_spectrum_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_error_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_error_dialog_destroy(GtkObject *object, gpointer user_data);
void on_error_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_error_quit_button_clicked(GtkButton *button, gpointer user_data);
void on_quit_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_quit_button_clicked(GtkButton *button, gpointer user_data);
gboolean on_scope_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
void on_start_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_skip_button_clicked(GtkButton *button, gpointer user_data);
void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_rpm_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_pix_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_ioc_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_phl_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_enhance_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_deslant_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_in_image_phasing_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_wefax_drawingarea_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_wefax_drawingarea_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
void on_station_list_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_stations_window_destroy(GtkObject *object, gpointer user_data);
void on_down_button_clicked(GtkButton *button, gpointer user_data);
void on_up_button_clicked(GtkButton *button, gpointer user_data);
void on_save_button_clicked(GtkButton *button, gpointer user_data);
void on_delete_button_clicked(GtkButton *button, gpointer user_data);
void on_new_button_clicked(GtkButton *button, gpointer user_data);
void on_stations_treeview_cursor_changed(GtkTreeView *treeview, gpointer user_data);
gboolean on_stations_treeview_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_capture_setup_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_jpeg_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_pgm_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_both_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_zerocrossing_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_bilevel_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_control_drawingarea_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
void on_save_checkbutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
/* cat.c */
gboolean Open_Tcvr_Serial(void);
void Close_Tcvr_Serial(void);
void Cancel_CAT(void);
void Tcvr_Status(int action, tcvr_status_t *status);
gboolean Read_Tcvr_Serial(char *data, size_t len);
gboolean Write_Tcvr_Serial(char *data, size_t len);
gboolean Read_Rx_Freq(int *freq);
gboolean Write_Rx_Freq(int freq);
gboolean Set_Rx_Freq_Idle_Cb(gpointer data);
gboolean Tune_Tcvr(double x);
/* detect.c */
gboolean FM_Detect_Zero_Crossing(unsigned char *signal_level);
gboolean FM_Detect_Bilevel(unsigned char *signal_level);
gboolean Phasing_Detect(void);
void Tone_Detect(double tone_period, unsigned char input, int *tone_level);
gboolean Start_Tone_Detect(void);
gboolean Stop_Tone_Detect(short discr_op);
/* dft.c */
void Idft_Init(int dft_input_size, int dft_bin_size);
void Idft(int dft_input_size, int dft_bin_size);
/* display.c */
void DFT_Input_Data(short sample_val);
int DFT_Bin_Value(int sum_i, int sum_q, gboolean reset);
void Display_Waterfall(void);
void Display_Signal(unsigned char plot);
void Set_Indicators(int flag);
void Set_Menu_Items(void);
void Normalize(unsigned char *line_buffer, int line_len);
void Spectrum_Size_Allocate(GdkRectangle *allocation);
void Set_Sync_Slant(double sync_slant);
void Display_Level_Gauge(int input, int level1, int level2);
/* filters.c */
void Init_Chebyshev_Filter(filter_data_t *filter_data);
void DSP_Filter(filter_data_t *filter_data);
/* interface.c */
GtkWidget *create_main_window(void);
GtkWidget *create_quit_dialog(void);
GtkWidget *create_main_menu(void);
GtkWidget *create_error_dialog(void);
GtkWidget *create_stations_window(void);
/* jpeg.c */
jpec_buffer_t *jpec_buffer_new(void);
jpec_buffer_t *jpec_buffer_new2(int siz);
void jpec_buffer_del(jpec_buffer_t *b);
void jpec_buffer_write_byte(jpec_buffer_t *b, int val);
void jpec_buffer_write_2bytes(jpec_buffer_t *b, int val);
jpec_enc_t *jpec_enc_new(const uint8_t *img, uint16_t w, uint16_t h);
jpec_enc_t *jpec_enc_new2(const uint8_t *img, uint16_t w, uint16_t h, int q);
void jpec_enc_del(jpec_enc_t *e);
const uint8_t *jpec_enc_run(jpec_enc_t *e, int *len);
void jpec_enc_init_dqt(jpec_enc_t *e);
void jpec_enc_open(jpec_enc_t *e);
void jpec_enc_close(jpec_enc_t *e);
void jpec_enc_write_soi(jpec_enc_t *e);
void jpec_enc_write_app0(jpec_enc_t *e);
void jpec_enc_write_dqt(jpec_enc_t *e);
void jpec_enc_write_sof0(jpec_enc_t *e);
void jpec_enc_write_dht(jpec_enc_t *e);
void jpec_enc_write_sos(jpec_enc_t *e);
int jpec_enc_next_block(jpec_enc_t *e);
void jpec_enc_block_dct(jpec_enc_t *e);
void jpec_enc_block_quant(jpec_enc_t *e);
void jpec_enc_block_zz(jpec_enc_t *e);
void jpec_huff_skel_init(jpec_huff_skel_t *skel);
jpec_huff_t *jpec_huff_new(void);
void jpec_huff_del(jpec_huff_t *h);
void jpec_huff_encode_block(jpec_huff_t *h, jpec_block_t *block, jpec_buffer_t *buf);
void jpec_huff_encode_block_impl(jpec_block_t *block, jpec_huff_state_t *s);
void jpec_huff_write_bits(jpec_huff_state_t *s, unsigned int bits, int n);
/* main.c */
int main(int argc, char *argv[]);
/* perseus.c */
gboolean Demodulate_SSB(short *signal_sample);
void Perseus_Set_Center_Frequency(int center_freq);
void Perseus_Close_Device(void);
gboolean Perseus_Initialize(void);
/* shared.c */
/* sound.c */
gboolean Open_PCM(snd_pcm_t **handle, snd_pcm_stream_t stream, char *mesg, int *error);
gboolean Open_Capture(char *mesg, int *error);
gboolean Open_Mixer(char *mesg, int *error);
gboolean Set_Capture_Level(int level, char *mesg, int *error);
void Close_Capture(void);
void Close_Mixer(void);
gboolean Sound_Signal_Sample(short *sample_val);
gboolean Xrun_Recovery(snd_pcm_t *handle, int error);
/* stations.c */
void Create_List_Store(void);
void Insert_Columns(GtkListStore *store, int ncols, char *colname[]);
void List_Stations(void);
gboolean Save_Stations_File(char *stations_file);
void Select_Treeview_Row(int direction);
void Stations_Cursor_Changed(GtkTreeView *treeview);
void Treeview_Button_Press(void);
void New_Station_Row(void);
void Delete_Station_Row(void);
void cell_edited_callback(GtkCellRendererText *cell, gchar *path, gchar *new_text, gpointer user_data);
/* support.c */
GtkWidget *lookup_widget(GtkWidget *widget, const gchar *widget_name);
void add_pixmap_directory(const gchar *directory);
GtkWidget *create_pixmap(GtkWidget *widget, const gchar *filename);
GdkPixbuf *create_pixbuf(const gchar *filename);
void glade_set_atk_action_description(AtkAction *action, const gchar *action_name, const gchar *description);
/* utils.c */
int Load_Line(char *buff, FILE *pfile, char *mesg);
gboolean Load_Config(gpointer data);
void New_Lines_Per_Min(void);
void New_Pixels_Per_Line(void);
void New_IOC(void);
void New_Phasing_Lines(void);
void New_Image_Enhance(void);
void Configure(void);
void File_Name(char *file_name, const char *extn);
char *Fname(char *fpath);
gboolean Open_File(FILE **fp, char *fname, const char *mode);
gboolean Save_Image_pgm(FILE *fp, const char *type, int width, int height, int max_val, unsigned char *buffer);
gboolean Save_Image_jpg(FILE *fp, int width, int height, uint8_t *buffer);
void Usage(void);
void Show_Message(char *mesg, char *attr);
gboolean mem_alloc(void **ptr, size_t req);
gboolean mem_realloc(void **ptr, size_t req);
void free_ptr(void **ptr);
void Cleanup(void);
int isFlagSet(int flag);
int isFlagClear(int flag);
void SetFlag(int flag);
void ClearFlag(int flag);
void ToggleFlag(int flag);
void Strlcpy(char *dest, const char *src, size_t n);
void Strlcat(char *dest, const char *src, size_t n);
/* wefax.c */
gboolean Wefax_Control(gpointer data);
void Receive_Error(void);
gboolean Wefax_Decode(void);
gboolean Wefax_Drawingarea_Button_Press(GdkEventButton *event);
void Start_Button_Toggled(GtkToggleButton *togglebutton);

#endif

