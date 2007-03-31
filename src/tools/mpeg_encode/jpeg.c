/*===========================================================================*
 * jpeg.c                                     *
 *                                         *
 *    procedures to deal with JPEG files                     *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    JMovie2JPEG                                 *
 *      ReadJPEG                                 *
 *                                         *
 *===========================================================================*/

/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*  
 *  $Header: /u/smoot/md/mpeg_encode/RCS/jpeg.c,v 1.6 1995/06/08 20:36:00 smoot Exp $
 *  $Log: jpeg.c,v $
 * Revision 1.6  1995/06/08  20:36:00  smoot
 * added "b"'s to fopen()s for MSDOS
 *
 * Revision 1.5  1995/02/02  21:24:02  eyhung
 * slight cleanup of unused variables
 *
 * Revision 1.4  1995/01/19  23:08:33  eyhung
 * Changed copyrights
 *
 * Revision 1.3  1995/01/19  22:58:34  smoot
 * fixes (I dont know what)
 *
 * Revision 1.2  1994/11/12  02:11:50  keving
 * nothing
 *
 * Revision 1.1  1994/03/15  00:27:11  keving
 * nothing
 *
 * Revision 1.2  1993/12/22  19:19:01  keving
 * nothing
 *
 * Revision 1.1  1993/07/22  22:23:43  keving
 * nothing
 *
 */


/*==============*
 * HEADER FILES *
 *==============*/

#include <stdio.h>
#include "all.h"
#include "mtypes.h"
#include "frames.h"
#include "prototypes.h"
#include "param.h"
#include "readframe.h"
#include "fsize.h"
#include "rgbtoycc.h"
#include "jpeg.h"

/* make it happier.... */
#undef DCTSIZE2
#include "jpeg/jpeglib.h"


#define HEADER_SIZE 607   /*JFIF header size used on output images*/



/*=======================================================================*
 *                                                                       *
 * JMovie2JPEG                                                           *
 *                                                                       *
 *      Splits up a Parallax J_Movie into a set of JFIF image files      *
 *                                                                       *
 * RETURNS:     nothing                                                  *
 *                                                                       *
 * SIDE EFFECTS:    none                                                 *
 *                                                                       *
 *   Contributed By James Boucher(jboucher@flash.bu.edu)                 *
 *               Boston University Multimedia Communications Lab         *
 * This code was adapted from the Berkeley Playone.c and Brian Smith's   *
 * JGetFrame code after being modified on 10-7-93 by Dinesh Venkatesh    *
 * of BU.                                                                *
 *       This code converts a version 2 Parallax J_Movie into a          *
 * set of JFIF compatible JPEG images. It works for all image            *
 * sizes and qualities.                                                  *
 ************************************************************************/
void
  JMovie2JPEG(infilename,obase,start,end)
char *infilename;       /* input filename string */
char *obase;            /* output filename base string=>obase##.jpg */ 
int start;              /* first frame to be extracted */
int end;                /* last frame to be extracted */
{
  FILE *inFile;            /* Jmovie file pointer */
  FILE *outFile;        /* JPEG file pointer for output file */
  int fd, i;            /* input file descriptor and a counting variable*/
  char ofname[256];        /* output filename string */
  int Temp = 0, temp = 0;    /* dummy variables */
  int image_offset = 0;        /* counting variable */
  /* J_Movie header infomation */
  int ver_no;            /* version number - expected to be 2 */
  int fps;            /* frame rate - frames per second */
  int no_frames;        /* total number of frames in jmovie */
  int bandwidth;        /* bandwidth required for normal playback*/
  int qfactor;            /* quality factor used to scale Q matrix */
  int mapsize;            /* number of color map entries - 2^24 */
  int audio_tracks;        /* number of audio tracks ==1    */
  int audiosize;        /*number of bytes in audio tracks */
  int *inoffsets;        /* input frame offsets from start of jmovie*/
  int width;            /* image width */
  int height;            /* image height */
  int size;            /* total image size in bytes */
  char op_code;            /* jmovie op_code */
  char jpeg_size[4];        /* jpeg data size */
  static char junk[1000];    /* data sink for audio data */

  /* The next array represents the default JFIF header for
     quality = 100 and size = 320x240. The values are
     adjusted as the J_Movie header is read.  The default
     size of this array is set large so as to make room
     for the appending of the jpeg bitstream. It can be
     made smaller if you have a better idea of its expected size*/
  static char inbuffer[300000] = {    
    0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,  
    0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x11,  
    0x08, 0x00, 0xF0, 0x01, 0x40, 0x03, 0x01, 0x21,
    0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xFF,  
    0xDB, 0x00, 0x84, 0x00, 0x10, 0x0B, 0x0C, 0x0E,
    0x0C, 0x0A, 0x10, 0x0E, 0x0D, 0x0E, 0x12,  
    0x11, 0x10, 0x13, 0x18, 0x28, 0x1A, 0x18, 0x16,
    0x16, 0x18, 0x31, 0x23, 0x25, 0x1D, 0x28, 0x3A,  
    0x33, 0x3D, 0x3C, 0x39, 0x33, 0x38, 0x37, 0x40,
    0x48, 0x5C, 0x4E, 0x40, 0x44, 0x57, 0x45, 0x37,  
    0x38, 0x50, 0x6D, 0x51, 0x57, 0x5F, 0x62, 0x67,
    0x68, 0x67, 0x3E, 0x4D, 0x71, 0x79, 0x70, 0x64,  
    0x78, 0x5C, 0x65, 0x67, 0x63, 0x01, 0x11, 0x12,
    0x12, 0x18, 0x15, 0x18, 0x2F, 0x1A, 0x1A, 0x2F,  
    0x63, 0x42, 0x38, 0x42, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,  
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,  
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,  
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,  
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,  
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0xFF, 0xC4,
    0x01, 0xA2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01,  
    0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,  
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04,  
    0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01,
    0x7D, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05,  
    0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61,
    0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1,  
    0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1,
    0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09,   
    0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26,
    0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37,  
    0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57,  
    0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77,  
    0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96,  
    0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
    0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,  
    0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3,
    0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2,  
    0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
    0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,  
    0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,
    0xF7, 0xF8, 0xF9, 0xFA, 0x01, 0x00, 0x03, 0x01,  
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,  
    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
    0x0A, 0x0B, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04,  
    0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00,
    0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11,  
    0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51,
    0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08,  
    0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23,
    0x33, 0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1,   
    0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17,
    0x18, 0x19, 0x1A, 0x26, 0x27, 0x28, 0x29, 0x2A,  
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
    0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54,  
    0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74,  
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83,
    0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92,  
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A,
    0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9,  
    0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8,
    0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,  
    0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5,  
    0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFF, 0xDA,  
    0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03,
    0x11, 0x00, 0x3F, 0x00  

    };
    
  if(start > end)
    {
      fprintf(stderr,"bad frame numbers\n");
      exit(1);
    }
    
  /* open J_Movie */
  inFile = fopen(infilename, "rb");
  if (inFile == NULL) 
    {
      perror (infilename);
      exit (1);
    }
    
  /* get file descriptor */    
  fd = fileno(inFile);
    
  /* The following lines parse the jpeg_movie header and recover the */
  /* relavant information */

  fseek (inFile, (8*sizeof(char)),0);
    
  if (fread (&ver_no,sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading version");
      exit(1);
    }  
  if(ver_no != 2){
    perror("Unrecognized version - Quantization tables may be wrong\n");
  }
  if (fread (&(fps),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading fps");
      exit(1);
    }  
  if (fread (&(no_frames),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading no_frames");
      exit(1);
    }  

  inoffsets = (int *)malloc(no_frames*sizeof(int));
    
  if (fread (&(width),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading width");
      exit(1);
    }  
  /* set image width in JFIF header */
  inbuffer[27] = (char)(0xFF & (width >> 8));
  inbuffer[28] = (char)(0xFF & width);
 
  if (fread (&(height),sizeof(int), 1,inFile) != 1)
    {
      perror("Error in reading height");
      exit(1);
    }  
  /* set image height in JFIF header */
  inbuffer[25] = (char)(0xFF & (height >> 8));
  inbuffer[26] = (char)(0xFF & height);
    
  if (fread (&(bandwidth),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading bandwidth");
      exit(1);
    }  
    
  if (fread (&(qfactor),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading qfactor");
      exit(1);
    }  
  /* The default quality factor = 100, therefore, if
     our quality factor does not equal 100 we must
     scale the quantization matrices in the JFIF header*/    
  /* Note values are clipped to a max of 255 */
  if(qfactor != 100){
    for(Temp=44;Temp<108;Temp++){
      temp= (inbuffer[Temp]*qfactor)/100;
      inbuffer[Temp] = (char)((temp<255) ? temp : 255);
    }
    for(Temp=109;Temp<173;Temp++){
      temp = (inbuffer[Temp]*qfactor)/100;
      inbuffer[Temp] = (char)((temp<255) ? temp : 255);
    }    
  }
  
  if (fread (&(mapsize),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading mapsize");
      exit(1);
    }  
  if (fread (&(image_offset),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading image offset");
      exit(1);
    }
  if (fread (&(audio_tracks),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading audio tracks");
      exit(1);
    }
    
  fread(junk,sizeof(int),1,inFile);
    
  if (fread (&(audiosize),sizeof(int),1,inFile) != 1)
    {
      perror("Error in reading audiosize");
      exit(1);
    }
    
  fseek (inFile,(image_offset),0);
    

  if(no_frames <= end)
    {
      end = no_frames - 1;
    }
    

  for(i=0;i<no_frames;i++) 
    {
      fread(&(inoffsets[i]),sizeof(int),1,inFile);
    } /* Reads in the frame sizes into the array */
    
  rewind(inFile);

  /* Extract JFIF files from J_Movie */    
  for (i=start; i<=end ; i++) 
    {
      size = inoffsets[i]- inoffsets[i-1]- 5;
      lseek(fd, inoffsets[i-1],0); 
      read(fd, &(op_code), 1);
      while( op_code !=  0xffffffec)
    {
      read(fd,junk,audiosize);
      read(fd, &(op_code), 1);  
      size = size - audiosize ;
    } /* To skip the audio bytes in each frame */
      read(fd,jpeg_size,4);
      read(fd,&(inbuffer[607]),(size));
      sprintf(ofname,"%s%d.jpg",obase,i);
      outFile = fopen(ofname, "wb");
      fwrite(inbuffer,(size+607),sizeof(char),outFile);
      fclose(outFile);        
    }
  free(inoffsets);
  fclose(inFile);
}




/*===========================================================================*
 *
 * ReadJPEG  contributed by James Arthur Boucher of Boston University's
 *                                Multimedia Communications Lab
 *
 *      read a JPEG file and copy data into frame original data arrays
 *
 * RETURNS:     mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
/*************************JPEG LIBRARY INTERFACE*********************/
/*
 * THE BIG PICTURE:
 *
 * The rough outline this JPEG decompression operation is:
 *
 *      allocate and initialize JPEG decompression object
 *      specify data source (eg, a file)
 *      jpeg_read_header();     // obtain image dimensions and other parameters
 *      set parameters for decompression
 *      jpeg_start_decompress();
 *      while (scan lines remain to be read)
 *              jpeg_read_scanlines(...);
 *      jpeg_finish_decompress();
 *      release JPEG decompression object
 *
 */
void
ReadJPEG(mf, fp)
    MpegFrame *mf;
    FILE *fp;
{

  /* This struct contains the JPEG decompression parameters and pointers to
   * working data (which is allocated as needed by the JPEG library).
   */
  static struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  /* More stuff */
  JSAMPARRAY scanarray[3];
  int ci,cd,cp;
  JDIMENSION ncols[3];
  JDIMENSION nrows[3];
  jpeg_component_info *compptr;
  int buffer_height;
  int current_row[3];
  uint8_mpeg_t **orig[3];
  int h_samp[3],v_samp[3];
  int max_h_samp,max_v_samp;
  int temp_h, temp_v;
  int temp;

  /* Allocate and initialize JPEG decompression object */
   cinfo.err = jpeg_std_error(&jerr);

  /*
  ** If we're reading from stdin we want to create the cinfo struct
  ** ONCE (during the first read).  This is because when reading jpeg
  ** from stdin we will not release the cinfo struct, because doing
  ** so would totally screw up the read buffer and make it impossible
  ** to read jpegs from stdin.
  ** Dave Scott (dhs), UofO, 7/19/95
  */
  {
    static int first_stdin = 1;
    if( (fp != stdin) || first_stdin)
      {
    first_stdin = 0;
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);
    /* specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, fp);
      }
  }
  
  /* specify data source (eg, a file) */
  
  jpeg_stdio_src(&cinfo, fp);
  
  /* read file parameters with jpeg_read_header() */
  
  
  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   */
  
  /* set parameters for decompression */
#ifdef JPEG4
  cinfo.want_raw_output = TRUE;
#else
  cinfo.raw_data_out = TRUE;
#endif
  cinfo.out_color_space = JCS_YCbCr;
  
  /* calculate image output dimensions */
  jpeg_calc_output_dimensions(&cinfo);
  /* the above calculation will set these soon */
  /* for now we'll set them ourselves */
  
  
  /* tell mpeg_encode the size of the JPEG Image*/
  Fsize_Note(mf->id,(int)(cinfo.image_width),(int)(cinfo.image_height));
  
  /* Allocate memory for the raw YCbCr data to occupy*/
  Frame_AllocYCC(mf);      /*allocate space for mpeg frame*/
  
  /* copy pointers to array structure- this make the following
     code more compact  */
  orig[0] = mf->orig_y;
  orig[1] = mf->orig_cb;
  orig[2] = mf->orig_cr;
  
  /* Note that we can use the info obtained from jpeg_read_header.
   */
  
  /* Start decompressor */
  
  jpeg_start_decompress(&cinfo);
  
  
  /* JSAMPLEs per row in output buffer  */
  /* collect component subsample values*/
  for(cp=0,compptr = cinfo.comp_info;cp<cinfo.num_components;
      cp++,compptr++) {
    h_samp[cp] = compptr->h_samp_factor;
    v_samp[cp] = compptr->v_samp_factor;
  }
  /* calculate max subsample values*/
  temp_h = (h_samp[0]<h_samp[1]) ? h_samp[1] : h_samp[0];
  max_h_samp = (temp_h<h_samp[2]) ? h_samp[2]:temp_h;
  temp_v = (v_samp[0]<v_samp[1]) ? v_samp[1] : v_samp[0];
  max_v_samp = (temp_v<v_samp[2]) ? v_samp[2]:temp_v;
  
  /* Make an 8-row-high sample array that will go away when done with image */
#ifdef JPEG4
  buffer_height = 8;  /* could be 2, 4,8 rows high */
#else
  buffer_height = cinfo.max_v_samp_factor * cinfo.min_DCT_scaled_size;
#endif
  
  for(cp=0,compptr = cinfo.comp_info;cp<cinfo.num_components;
      cp++,compptr++) {
    ncols[cp] = (JDIMENSION)((cinfo.image_width*compptr->h_samp_factor)/
                 max_h_samp);
    
    nrows[cp] = (JDIMENSION)((buffer_height*compptr->v_samp_factor)/
                 max_v_samp);
    
    scanarray[cp] = (*cinfo.mem->alloc_sarray)
      ((j_common_ptr) &cinfo, JPOOL_IMAGE, ncols[cp], nrows[cp]);
    
  }
  
  
  /*  while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */
  
  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */

  while (cinfo.output_scanline < cinfo.output_height) {

#ifdef JPEG4
    (void) jpeg_read_raw_scanlines(&cinfo, scanarray, buffer_height);
#else
    (void) jpeg_read_raw_data(&cinfo, scanarray, buffer_height);
#endif

/* alter subsample ratio's if neccessary */
    if((h_samp[0]==2)&&(h_samp[1]==1)&&(h_samp[2]==1)&&
       (v_samp[0]==2)&&(v_samp[1]==1)&&(v_samp[2]==1)){
      /* we are 4:1:1 as expected by the encoder*/
    }else if((h_samp[0]==2)&&(h_samp[1]==1)&&(h_samp[2]==1)&&
         (v_samp[0]==1)&&(v_samp[1]==1)&&(v_samp[2]==1)){
      /* must subsample 2:1 vertically and adjust params*/
      for(ci=1; ci<3; ci++){
    for(cp=0; cp<(buffer_height/2);cp=cp+1){
      for(cd=0;cd<ncols[ci];cd++){
        temp =((scanarray[ci][cp*2][cd]+scanarray[ci][(cp*2)+1][cd])/2);
        scanarray[ci][cp][cd] = (JSAMPLE)(temp);
      }
    }
      }
      /* only reset values the first time through*/
      if(cinfo.output_scanline==buffer_height){
    nrows[1] = nrows[1]/2;
    nrows[2] = nrows[2]/2;
    max_v_samp = 2;
    v_samp[0] = 2;
      }
    }else{
      fprintf(stderr, "Not a supported subsampling ratio\n");
      exit(1);
    }
    
    /* transfer data from jpeg buffer to MPEG frame */
    /* calculate the row we wish to output into */
    for(ci=0,compptr=cinfo.comp_info;ci<cinfo.num_components;
    ci++,compptr++){
      current_row[ci] =((cinfo.output_scanline - buffer_height)*
            (v_samp[ci])/max_v_samp);  
      
      jcopy_sample_rows(scanarray[ci],0,(JSAMPARRAY)(orig[ci]),
            current_row[ci],nrows[ci],ncols[ci]);
    }
    
  }  
  
  /* Step 7: Finish decompression */
  
  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */
  
  /* Step 8: Release JPEG decompression object */
  
  /*
   ** DO NOT release the cinfo struct if we are reading from stdin, this
   ** is because the cinfo struct contains the read buffer, and the read
   ** buffer may (and almost always does) contain the end of one image and
   ** the beginning of another.  If we throw away the read buffer then
   ** we loose the beginning of the next image, and we're screwed.
  ** Dave Scott (dhs), UofO, 7/19/95
  */
  if( fp == stdin) {
    static int no_from_stdin = 0;
    no_from_stdin++;
    /* fprintf( stderr, "%d jpeg images read from stdin\n", no_from_stdin); */
  }
  else {
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);
  }

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  
  
  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   * If you prefer to treat corrupt data as a fatal error, override the
   * error handler's emit_message method to call error_exit on a warning.
   */

  /* And we're done! */
  
}


/*
 * SOME FINE POINTS:
 *
 * In the above loop, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this for the same reasons discussed in the compression example.  Actually
 * there is one perfectly normal situation in which jpeg_read_scanlines may
 * return fewer lines than you asked for: at the bottom of the image.  But the
 * loop above can't ask for more lines than there are in the image since it
 * reads only one line at a time.
 *
 * In some high-speed operating modes, some data copying can be saved by
 * making the buffer passed to jpeg_read_scanlines be cinfo.rec_outbuf_height
 * lines high (or a multiple thereof).  This will usually be 1, 2, or 4 lines.
 *
 * To decompress multiple images, you can repeat the whole sequence, or you
 * can keep the JPEG object around and just repeat steps 2-7.  This will
 * save a little bit of startup/shutdown time.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must have data supplied
 * bottom-to-top, you can use one of the virtual arrays provided by the
 * JPEG memory manager to invert the data.  See wrrle.c for an example.
 */
