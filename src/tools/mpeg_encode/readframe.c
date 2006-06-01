/*===========================================================================*
 * readframe.c                                     *
 *                                         *
 *    procedures to read in frames                         *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    ReadFrame                                 *
 *    SetFileType                                 *
 *    SetFileFormat                                 *
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
 *  $Header: /n/picasso/project/mpeg/mpeg_dist/mpeg_encode/RCS/readframe.c,v 1.27 1995/08/14 22:31:40 smoot Exp $
 *  $Log: readframe.c,v $
 *  Revision 1.27  1995/08/14 22:31:40  smoot
 *  reads training info from PPms now (needed for piping reads)
 *
 *  Revision 1.26  1995/08/07 21:48:36  smoot
 *  better error reporting, JPG == JPEG now
 *
 *  Revision 1.25  1995/06/12 20:30:12  smoot
 *  added popen for OS2
 *
 * Revision 1.24  1995/06/08  20:34:36  smoot
 * added "b"'s to fopen calls to make MSDOS happy
 *
 * Revision 1.23  1995/05/03  10:16:01  smoot
 * minor compile bug with static f
 *
 * Revision 1.22  1995/05/02  22:00:12  smoot
 * added TUNEing, setting near-black values to black
 *
 * Revision 1.21  1995/03/27  21:00:01  eyhung
 * fixed bug with some long jpeg names
 *
 * Revision 1.20  1995/02/02  01:05:54  eyhung
 * Fixed aAdded error checking for stdin
 *
 * Revision 1.19  1995/02/01  05:01:12  eyhung
 * Removed troubleshooting printf
 *
 * Revision 1.18  1995/01/31  21:08:16  eyhung
 * Improved YUV_FORMAT strings with better algorithm
 *
 * Revision 1.17  1995/01/27  23:34:09  eyhung
 * Removed temporary JPEG files created by JMOVIE input
 *
 * Revision 1.16  1995/01/27  21:57:43  eyhung
 * Added case for reading original JMOVIES
 *
 * Revision 1.14  1995/01/24  23:47:51  eyhung
 * Confusion with Abekas format fixed : all other YUV revisions are wrong
 *
 * Revision 1.13  1995/01/20  00:02:30  smoot
 * added gamma correction
 *
 * Revision 1.12  1995/01/19  23:09:21  eyhung
 * Changed copyrights
 *
 * Revision 1.11  1995/01/17  22:23:07  aswan
 * AbekasYUV chrominance implementation fixed
 *
 * Revision 1.10  1995/01/17  21:26:25  smoot
 * Tore our average on Abekus/Phillips reconstruct
 *
 * Revision 1.9  1995/01/17  08:22:34  eyhung
 * Debugging of ReadAYUV
 *
 * Revision 1.8  1995/01/16  13:18:24  eyhung
 * Interlaced YUV format (e.g. Abekas) support added (slightly buggy)
 *
 * Revision 1.7  1995/01/16  06:58:23  eyhung
 * Added skeleton of ReadAYUV (for Abekas YUV files)
 *
 * Revision 1.6  1995/01/13  23:22:23  smoot
 * Added ReadY, so we can make black&white movies (how artsy!)
 *
 * Revision 1.5  1994/12/16  00:20:40  smoot
 * Now errors out on too small an input file
 *
 * Revision 1.4  1994/11/12  02:11:59  keving
 * nothing
 *
 * Revision 1.3  1994/03/15  00:27:11  keving
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

#include "all.h"
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "mtypes.h"
#include "frames.h"
#include "prototypes.h"
#include "parallel.h"
#include "param.h"
#include "readframe.h"
#include "fsize.h"
#include "rgbtoycc.h"
#include "jpeg.h"
#include "opts.h"

#define PPM_READ_STATE_MAGIC    0
#define PPM_READ_STATE_WIDTH    1
#define PPM_READ_STATE_HEIGHT    2
#define PPM_READ_STATE_MAXVAL    3
#define PPM_READ_STATE_DONE    4


/*==================*
 * STATIC VARIABLES *
 *==================*/

static int  fileType = BASE_FILE_TYPE;
struct YuvLine {
    uint8_mpeg_t    data[3072];
    uint8_mpeg_t    y[1024];
    int8_mpeg_t    cr[1024];
    int8_mpeg_t    cb[1024];
};


/*==================*
 * Portability      *
 *==================*/
#ifdef __OS2__
  #define popen _popen
#endif
   

/*==================*
 * Global VARIABLES *
 *==================*/

extern boolean GammaCorrection;
extern float GammaValue;
extern int outputWidth,outputHeight;
boolean resizeFrame;
char *CurrFile;

/*===============================*
 * INTERNAL PROCEDURE prototypes *
 *===============================*/

static char *ScanNextString _ANSI_ARGS_((char *inputLine, char *string));
static void ReadPNM _ANSI_ARGS_((FILE * fp, MpegFrame * mf));
static boolean    ReadPPM _ANSI_ARGS_((MpegFrame *mf, FILE *fpointer));
static void ReadEYUV _ANSI_ARGS_((MpegFrame * mf, FILE *fpointer,
                 int width, int height));
static void ReadAYUV _ANSI_ARGS_((MpegFrame * mf, FILE *fpointer,
                 int width, int height));
static void SeparateLine _ANSI_ARGS_((FILE *fpointer, struct YuvLine *lineptr,
                     int width));
static void ReadY _ANSI_ARGS_((MpegFrame * mf, FILE *fpointer,
                 int width, int height));
static void ReadSub4 _ANSI_ARGS_((MpegFrame * mf, FILE *fpointer,
                  int width, int height));
static void DoGamma  _ANSI_ARGS_((MpegFrame *mf, int width, int height));

static void DoKillDim _ANSI_ARGS_((MpegFrame *mf, int w, int h));

#define safe_fread(ptr,sz,len,fileptr)                           \
    if ((safe_read_count=fread(ptr,sz,len,fileptr))!=sz*len) {   \
      fprintf(stderr,"Input file too small! (%s)\n",CurrFile);   \
      exit(1);}                                                  \

/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/



void    SetResize(set)
    boolean    set;
{
    resizeFrame = set;
}



/*===========================================================================*
 *
 * ReadFrame
 *
 *    reads the given frame, performing conversion as necessary
 *    if addPath = TRUE, then must add the current path before the
 *    file name
 *
 * RETURNS:    frame modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
void
ReadFrame(frame, fileName, conversion, addPath)
    MpegFrame *frame;
    char *fileName;
    char *conversion;
    boolean addPath;
{
    FILE    *ifp;
    char    command[1024];
    char    fullFileName[1024];
    MpegFrame    tempFrame;
    MpegFrame    *framePtr;
#ifdef BLEAH
    static int32_mpeg_t    readDiskTime = 0;
    int32_mpeg_t    diskStartTime, diskEndTime;

time(&diskStartTime);
#endif

    if ( resizeFrame ) {
      tempFrame.inUse = FALSE;
      tempFrame.ppm_data = NULL;
      tempFrame.rgb_data = NULL;
      tempFrame.orig_y = NULL;
      tempFrame.y_blocks = NULL;
      tempFrame.decoded_y = NULL;
      tempFrame.halfX = NULL;
      framePtr = &tempFrame;
    } else {
      framePtr = frame;
    }

    if ( addPath ) {
      sprintf(fullFileName, "%s/%s", currentPath, fileName);
    } else {
      sprintf(fullFileName, "%s", fileName);
    }

    CurrFile = fullFileName;

#ifdef BLEAH
    if ( ! childProcess ) {
    fprintf(stdout, "+++++READING Frame %d  (type %d):  %s\n", framePtr->id,
            framePtr->type, fullFileName);
    }
#endif

    if ( fileType == ANY_FILE_TYPE ) {
    char *convertPtr, *commandPtr, *charPtr;

      if ( stdinUsed ) {
        fprintf(stderr, "ERROR : cannot use stdin with INPUT_CONVERT.\n");
        exit(1);
      }

      /* replace every occurrence of '*' with fullFileName */
      convertPtr = conversion;
      commandPtr = command;
      while ( *convertPtr != '\0' ) {
        while ( (*convertPtr != '\0') && (*convertPtr != '*') ) {
          *commandPtr = *convertPtr;
          commandPtr++;
          convertPtr++;
        }

        if ( *convertPtr == '*' ) {
          /* copy fullFileName */
          charPtr = fullFileName;
          while ( *charPtr != '\0' ) {
            *commandPtr = *charPtr;
            commandPtr++;
            charPtr++;
          }

          convertPtr++;   /* go past '*' */
        }
      }
      *commandPtr = '\0';

      if ( (ifp = popen(command, "r")) == NULL ) {
        fprintf(stderr, "ERROR:  Couldn't execute input conversion command:\n");
        fprintf(stderr, "\t%s\n", command);
        fprintf(stderr, "errno = %d\n", errno);
        if ( ioServer ) {
          fprintf(stderr, "IO SERVER:  EXITING!!!\n");
        } else {
          fprintf(stderr, "SLAVE EXITING!!!\n");
        }
        exit(1);
      }
    } else if (stdinUsed) {
      ifp = stdin;
    } else if ( (ifp = fopen(fullFileName, "rb")) == NULL ) {
      fprintf(stderr, "ERROR:  Couldn't open input file %s\n",
              fullFileName);
      exit(1);
    }

    switch(baseFormat) {
    case YUV_FILE_TYPE:

        /* Encoder YUV */
        if ((strncmp (yuvConversion, "EYUV", 4) == 0) ||
            (strncmp (yuvConversion, "UCB", 3) == 0) ) 
        {
            ReadEYUV(framePtr, ifp, realWidth, realHeight);
        }

        /* Abekas-type (interlaced) YUV */
        else {
            ReadAYUV(framePtr, ifp, realWidth, realHeight);
        }

        break;
    case Y_FILE_TYPE:
        ReadY(framePtr, ifp, realWidth, realHeight);
        break;
    case PPM_FILE_TYPE:
        if ( ! ReadPPM(framePtr, ifp) ) {
        fprintf(stderr, "Error reading PPM input file!!! (%s)\n", CurrFile);
        exit(1);
        }
        PPMtoYUV(framePtr);
        break;
    case PNM_FILE_TYPE:
        ReadPNM(ifp, framePtr);
        PNMtoYUV(framePtr);
        break;
    case SUB4_FILE_TYPE:
        ReadSub4(framePtr, ifp, yuvWidth, yuvHeight);
        break;
    case JPEG_FILE_TYPE:
    case JMOVIE_FILE_TYPE:
        ReadJPEG(framePtr, ifp);
        break;
    default:
        break;
    }

    if (! stdinUsed) {
      if ( fileType == ANY_FILE_TYPE ) {
    int errorcode;
    if ( (errorcode = pclose(ifp)) != 0) {
      fprintf(stderr, "WARNING:  Pclose reported error (%d)\n", errorcode);
    }
      } else {
        fclose(ifp);
      }
    }
    
    if ( baseFormat == JMOVIE_FILE_TYPE ) {
      remove(fullFileName);
    }

    if ( resizeFrame ) {
      Frame_Resize(frame, &tempFrame, Fsize_x, Fsize_y, outputWidth, outputHeight);
    }

#ifdef BLEAH
time(&diskEndTime);

readDiskTime += (diskEndTime-diskStartTime);

fprintf(stdout, "cumulative disk read time:  %d seconds\n", readDiskTime);
#endif

    if ( GammaCorrection ) {
      DoGamma(frame, Fsize_x, Fsize_y);
    }

    if ( kill_dim ) {
      DoKillDim(frame, Fsize_x, Fsize_y);
    }

    MotionSearchPreComputation(frame);
}


/*===========================================================================*
 *
 * SetFileType
 *
 *    set the file type to be either a base type (no conversion), or
 *    any type (conversion required)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    fileType
 *
 *===========================================================================*/
void
SetFileType(conversion)
    char *conversion;
{
    if ( strcmp(conversion, "*") == 0 ) {
    fileType = BASE_FILE_TYPE;
    } else {
    fileType = ANY_FILE_TYPE;
    }
}


/*===========================================================================*
 *
 * SetFileFormat
 *
 *    set the file format (PPM, PNM, YUV, JPEG)
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    baseFormat
 *
 *===========================================================================*/
void
SetFileFormat(format)
    char *format;
{
    if ( strcmp(format, "PPM") == 0 ) {
    baseFormat = PPM_FILE_TYPE;
    } else if ( strcmp(format, "YUV") == 0 ) {
    baseFormat = YUV_FILE_TYPE;
    } else if ( strcmp(format, "Y") == 0 ) {
    baseFormat = Y_FILE_TYPE;
    } else if ( strcmp(format, "PNM") == 0 ) {
    baseFormat = PNM_FILE_TYPE;
    } else if (( strcmp(format, "JPEG") == 0 ) || ( strcmp(format, "JPG") == 0 )) {
    baseFormat = JPEG_FILE_TYPE;
    } else if ( strcmp(format, "JMOVIE") == 0 ) {
    baseFormat = JMOVIE_FILE_TYPE;
    } else if ( strcmp(format, "SUB4") == 0 ) {
    baseFormat = SUB4_FILE_TYPE;
    } else {
    fprintf(stderr, "ERROR:  Invalid file format:  %s\n", format);
    exit(1);
    }
}


/*===========================================================================*
 *
 * ReadPNM
 *
 *    read a PNM file
 *
 * RETURNS:    mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ReadPNM(fp, mf)
    FILE *fp;
    MpegFrame *mf;
{
    int x, y;
    xelval maxval;
    int format;

    if (mf->rgb_data) {
    pnm_freearray(mf->rgb_data, Fsize_y);
    }
    mf->rgb_data = pnm_readpnm(fp, &x, &y, &maxval, &format);
    ERRCHK(mf, "pnm_readpnm");

    if (format != PPM_FORMAT) {
    if (maxval < 255) {
        pnm_promoteformat(mf->rgb_data, x, y, maxval, format, 255, PPM_FORMAT);
        maxval = 255;
    } else {
        pnm_promoteformat(mf->rgb_data, x, y, maxval, format, maxval, PPM_FORMAT);
    }
    }
    if (maxval < 255) {
    pnm_promoteformat(mf->rgb_data, x, y, maxval, format, 255, format);
    maxval = 255;
    }
    /*
     * if this is the first frame read, set the global frame size
     */
    Fsize_Note(mf->id, x, y);

    mf->rgb_maxval = maxval;
    mf->rgb_format = PPM_FORMAT;
}



/*===========================================================================*
 *
 * ReadIOConvert
 *
 *    do conversion; return a pointer to the appropriate file
 *
 * RETURNS:    pointer to the appropriate file
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
FILE *
ReadIOConvert(fileName)
    char *fileName;
{
    FILE    *ifp;
    char    command[1024];
    char    fullFileName[1024];
    char *convertPtr, *commandPtr, *charPtr;

    sprintf(fullFileName, "%s/%s", currentPath, fileName);

#ifdef BLEAH
    if ( ! childProcess ) {
    fprintf(stdout, "+++++READING (IO CONVERT) Frame %d  (type %d):  %s\n", frame->id,
        frame->type, fullFileName); }
#endif

    if ( strcmp(ioConversion, "*") == 0 ) {
      char buff[1024];
      ifp = fopen(fullFileName, "rb");
      sprintf(buff,"fopen \"%s\"",fullFileName);
      ERRCHK(ifp, buff);
      return ifp;
    }

    /* replace every occurrence of '*' with fullFileName */
    convertPtr = ioConversion;
    commandPtr = command;
    while ( *convertPtr != '\0' ) {
    while ( (*convertPtr != '\0') && (*convertPtr != '*') ) {
        *commandPtr = *convertPtr;
        commandPtr++;
        convertPtr++;
    }

    if ( *convertPtr == '*' ) {
        /* copy fullFileName */
        charPtr = fullFileName;
        while ( *charPtr != '\0' ) {
        *commandPtr = *charPtr;
        commandPtr++;
        charPtr++;
        }

        convertPtr++;   /* go past '*' */
    }
    }
    *commandPtr = '\0';

    if ( (ifp = popen(command, "r")) == NULL ) {
    fprintf(stderr, "ERROR:  Couldn't execute input conversion command:\n");
    fprintf(stderr, "\t%s\n", command);
    fprintf(stderr, "errno = %d\n", errno);
    if ( ioServer ) {
        fprintf(stderr, "IO SERVER:  EXITING!!!\n");
    } else {
        fprintf(stderr, "SLAVE EXITING!!!\n");
    }
    exit(1);
    }

    return ifp;
}



/*===========================================================================*
 *
 * ReadPPM
 *
 *    read a PPM file
 *
 * RETURNS:    TRUE if successful; FALSE otherwise; mf modified
 *
 * SIDE EFFECTS:    none
 *
 * MODIFICATIONS:
 *  
 *   Hank Childs, Thu Jun  1 11:29:35 PDT 2006
 *   Fix buffer overwrite.
 *
 *===========================================================================*/
static boolean
ReadPPM(mf, fpointer)
    MpegFrame *mf;
    FILE *fpointer;
{
    char    inputBuffer[71];
    char    string[71];
    char    *inputLine;
    int        height = 0, width = 0, maxVal=255;
    uint8_mpeg_t   junk[16384];
    register int y;
    int        state;
    int     safe_read_count;

    state = PPM_READ_STATE_MAGIC;

    while ( state != PPM_READ_STATE_DONE ) {
    if ( fgets(inputBuffer, 71, fpointer) == NULL ) {
        return FALSE;
    }
    
        inputLine = inputBuffer;
 
    if ( inputLine[0] == '#' ) {
        continue;
    }

    if ( inputLine[strlen(inputLine)-1] != '\n' ) {
        return FALSE;
    }

    switch(state) {
        case PPM_READ_STATE_MAGIC:
            if ( (inputLine = ScanNextString(inputLine, string)) == NULL ) {
            return FALSE;
        }

        if ( strcmp(string, "P6") != 0 ) {
            return FALSE;
        }
        state = PPM_READ_STATE_WIDTH;
        /* no break */
        case PPM_READ_STATE_WIDTH:
            if ( (inputLine = ScanNextString(inputLine, string)) == NULL ) {
            if ( inputLine == inputBuffer ) {
                return FALSE;
            } else {
                break;
            }
        }

        width = atoi(string);

        state = PPM_READ_STATE_HEIGHT;

        /* no break */
        case PPM_READ_STATE_HEIGHT:
            if ( (inputLine = ScanNextString(inputLine, string)) == NULL ) {
            if ( inputLine == inputBuffer ) {
                return FALSE;
            } else {
                break;
            }
        }

        height = atoi(string);

        state = PPM_READ_STATE_MAXVAL;

        /* no break */
        case PPM_READ_STATE_MAXVAL:
            if ( (inputLine = ScanNextString(inputLine, string)) == NULL ) {
            if ( inputLine == inputBuffer ) {
                return FALSE;
            } else {
                break;
            }
        }

        maxVal = atoi(string);

        state = PPM_READ_STATE_DONE;
        break;
    } /* end of switch */
    }

    Fsize_Note(mf->id, width, height);

    mf->rgb_maxval = maxVal;

    Frame_AllocPPM(mf);

    for ( y = 0; y < Fsize_y; y++ ) {
    safe_fread(mf->ppm_data[y], sizeof(char), 3*Fsize_x, fpointer);

    /* read the leftover stuff on the right side */
    safe_fread(junk, sizeof(char), 3*(width-Fsize_x), fpointer);
    }

    /* read the leftover stuff to prevent broken pipe */
    for ( y=Fsize_y; y<height; ++y ) {
      safe_fread(junk, sizeof(char), 3*Fsize_x, fpointer);
    }
    return TRUE;
}


/*===========================================================================*
 *
 * ReadEYUV
 *
 *    read a Encoder-YUV file (concatenated Y, U, and V)
 *
 * RETURNS:    mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ReadEYUV(mf, fpointer, width, height)
    MpegFrame *mf;
    FILE *fpointer;
    int width;
    int height;
{
    register int y;
    uint8_mpeg_t   junk[4096];
    int     safe_read_count;

    Fsize_Note(mf->id, width, height);

    Frame_AllocYCC(mf);

    for (y = 0; y < Fsize_y; y++) {            /* Y */
    safe_fread(mf->orig_y[y], 1, Fsize_x, fpointer);

    /* read the leftover stuff on the right side */
    if ( width != Fsize_x ) {
        safe_fread(junk, 1, width-Fsize_x, fpointer);
    }
    }

    /* read the leftover stuff on the bottom */
    for (y = Fsize_y; y < height; y++) {
    safe_fread(junk, 1, width, fpointer);
    }

    for (y = 0; y < (Fsize_y >> 1); y++) {            /* U */
    safe_fread(mf->orig_cb[y], 1, Fsize_x >> 1, fpointer);

    /* read the leftover stuff on the right side */
    if ( width != Fsize_x ) {
        safe_fread(junk, 1, (width-Fsize_x)>>1, fpointer);
    }
    }

    /* read the leftover stuff on the bottom */
    for (y = (Fsize_y >> 1); y < (height >> 1); y++) {
    safe_fread(junk, 1, width>>1, fpointer);
    }

    for (y = 0; y < (Fsize_y >> 1); y++) {            /* V */
    safe_fread(mf->orig_cr[y], 1, Fsize_x >> 1, fpointer);

    /* read the leftover stuff on the right side */
    if ( width != Fsize_x ) {
        safe_fread(junk, 1, (width-Fsize_x)>>1, fpointer);
    }
    }

    /* ignore leftover stuff on the bottom */
}

/*===========================================================================*
 *
 * ReadAYUV
 *
 *    read an Abekas-YUV file
 *
 * RETURNS:    mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ReadAYUV(mf, fpointer, width, height)
    MpegFrame *mf;
    FILE *fpointer;
    int width;
    int height;
{
    register int x, y;
    struct  YuvLine line1, line2;
    uint8_mpeg_t   junk[4096];
    int8_mpeg_t    *cbptr, *crptr;
    int     safe_read_count;

    Fsize_Note(mf->id, width, height);

    Frame_AllocYCC(mf);

    for (y = 0; y < Fsize_y; y += 2) {
    SeparateLine(fpointer, &line1, width);
    SeparateLine(fpointer, &line2, width);

    /* Copy the Y values for each line to the frame */
    for (x = 0; x < Fsize_x; x++) {
        mf->orig_y[y][x]   = line1.y[x];
        mf->orig_y[y+1][x] = line2.y[x];
    }

    cbptr = &(mf->orig_cb[y>>1][0]);
    crptr = &(mf->orig_cr[y>>1][0]);

    /* One U and one V for each two pixels horizontal as well */
    /* Toss the second line of Cr/Cb info, averaging was worse,
       so just subsample */
    for (x = 0; x < (Fsize_x >> 1); x ++) {
        cbptr[x] =  line1.cb[x];
        crptr[x] =  line1.cr[x];

    }
    }

    /* read the leftover stuff on the bottom */
    for (y = Fsize_y; y < height; y++) {
    safe_fread(junk, 1, width<<1, fpointer);
    }

}

/*===========================================================================*
 *
 * SeparateLine
 *
 *    Separates one line of pixels into Y, U, and V components
 *
 * RETURNS:    lineptr modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
SeparateLine(fpointer, lineptr, width)
    FILE *fpointer;
    struct YuvLine *lineptr;
    int width;
{
    uint8_mpeg_t   junk[4096];
    int8_mpeg_t    *crptr, *cbptr;
    uint8_mpeg_t   *yptr;
    int     num, length;
    int     safe_read_count;


    /* Sets the deinterlacing pattern */

    /* shorthand for UYVY */
    if (strncmp(yuvConversion, "ABEKAS", 6) == 0) {
    strcpy(yuvConversion, "UYVY");

    /* shorthand for YUYV */
    } else if (strncmp(yuvConversion, "PHILLIPS", 8) == 0) {
    strcpy(yuvConversion, "YUYV");
    }

    length = strlen (yuvConversion);

    if ((length % 2) != 0) {
    fprintf (stderr, "ERROR : YUV_FORMAT must represent two pixels, hence must be even in length.\n");
    exit(1);
    }

    /* each line in 4:2:2 chroma format takes 2X bytes to represent X pixels.
     * each line in 4:4:4 chroma format takes 3X bytes to represent X pixels.
     * Therefore, half of the length of the YUV_FORMAT represents 1 pixel.
     */
    safe_fread(lineptr->data, 1, Fsize_x*(length>>1), fpointer);

    /* read the leftover stuff on the right side */
    if ( width != Fsize_x ) {
    safe_fread(junk, 1, (width-Fsize_x)*(length>>1), fpointer);
    }

    crptr = &(lineptr->cr[0]);
    cbptr = &(lineptr->cb[0]);
    yptr = &(lineptr->y[0]);

    for (num = 0; num < (Fsize_x*(length>>1)); num++) {
    switch (yuvConversion[num % length]) {
    case 'U':
    case 'u':
        *(cbptr++) = (lineptr->data[num]);
        break;
    case 'V':
    case 'v':
        *(crptr++) = (lineptr->data[num]);
        break;
    case 'Y':
    case 'y':
        *(yptr++) = (lineptr->data[num]);
        break;
    default:
            fprintf(stderr, "ERROR: YUV_FORMAT must be one of the following:\n");
            fprintf(stderr, "       ABEKAS\n");
            fprintf(stderr, "       EYUV\n");
            fprintf(stderr, "       PHILLIPS\n");
            fprintf(stderr, "       UCB\n");
        fprintf(stderr, "       or any even-length string consisting of the letters U, V, and Y.\n");
            exit(1);
        }
    
    }

}


/*===========================================================================*
 *
 * ReadY
 *
 *    read a Y file
 *
 * RETURNS:    mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ReadY(mf, fpointer, width, height)
    MpegFrame *mf;
    FILE *fpointer;
    int width;
    int height;
{
    register int y;
    uint8_mpeg_t   junk[4096];
    int     safe_read_count;

    Fsize_Note(mf->id, width, height);

    Frame_AllocYCC(mf);

    for (y = 0; y < Fsize_y; y++) {            /* Y */
    safe_fread(mf->orig_y[y], 1, Fsize_x, fpointer);

    /* read the leftover stuff on the right side */
    if ( width != Fsize_x ) {
        safe_fread(junk, 1, width-Fsize_x, fpointer);
    }
    }

    /* read the leftover stuff on the bottom */
    for (y = Fsize_y; y < height; y++) {
    safe_fread(junk, 1, width, fpointer);
    }
    
    for (y = 0 ; y < (Fsize_y >> 1); y++) {
      memset(mf->orig_cb[y], 128, (Fsize_x>>1));
      memset(mf->orig_cr[y], 128, (Fsize_x>>1));
    }
}


/*===========================================================================*
 *
 * ReadSub4
 *
 *    read a YUV file (subsampled even further by 4:1 ratio)
 *
 * RETURNS:    mf modified
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
static void
ReadSub4(mf, fpointer, width, height)
    MpegFrame *mf;
    FILE *fpointer;
    int width;
    int height;
{
    register int y;
    register int x;
    uint8_mpeg_t   buffer[1024];
    int     safe_read_count;

    Fsize_Note(mf->id, width, height);

    Frame_AllocYCC(mf);

    for (y = 0; y < (height>>1); y++) {            /* Y */
    safe_fread(buffer, 1, width>>1, fpointer);
    for ( x = 0; x < (width>>1); x++ ) {
        mf->orig_y[2*y][2*x] = buffer[x];
        mf->orig_y[2*y][2*x+1] = buffer[x];
        mf->orig_y[2*y+1][2*x] = buffer[x];
        mf->orig_y[2*y+1][2*x+1] = buffer[x];
    }
    }

    for (y = 0; y < (height >> 2); y++) {            /* U */
    safe_fread(buffer, 1, width>>2, fpointer);
    for ( x = 0; x < (width>>2); x++ ) {
        mf->orig_cb[2*y][2*x] = buffer[x];
        mf->orig_cb[2*y][2*x+1] = buffer[x];
        mf->orig_cb[2*y+1][2*x] = buffer[x];
        mf->orig_cb[2*y+1][2*x+1] = buffer[x];
    }
    }

    for (y = 0; y < (height >> 2); y++) {            /* V */
    safe_fread(buffer, 1, width>>2, fpointer);
    for ( x = 0; x < (width>>2); x++ ) {
        mf->orig_cr[2*y][2*x] = buffer[x];
        mf->orig_cr[2*y][2*x+1] = buffer[x];
        mf->orig_cr[2*y+1][2*x] = buffer[x];
        mf->orig_cr[2*y+1][2*x+1] = buffer[x];
    }
    }
}


/*=====================*
 * INTERNAL PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * ScanNextString
 *
 *    read a string from a input line, ignoring whitespace
 *
 * RETURNS:    pointer to position in input line after string
 *              NULL if all whitespace
 *              puts string in 'string'
 *
 * SIDE EFFECTS:    file stream munched a bit
 *
 *===========================================================================*/
static char *
ScanNextString(inputLine, string)
    char *inputLine;
    char *string;
{
    /* skip whitespace */
    while ( isspace(*inputLine) && (*inputLine != '\n') ) {
        inputLine++;
    }

    if ( *inputLine == '\n' ) {
        return NULL;
    }

    while ( (! isspace(*inputLine)) && (*inputLine != '\n') ) {
        *string = *inputLine;
    string++;
    inputLine++;
    }

    *string = '\0';

    return inputLine;
}

/*===========================================================================*
 *
 * DoGamma
 *
 *    Gamma Correct the Lum values
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    Raises Y values to power gamma.
 *
 *===========================================================================*/
static void
DoGamma(mf, w, h)
MpegFrame *mf;
int w,h;
{
  static int GammaVal[256];
  static boolean init_done=FALSE;
  int i,j;

  if (!init_done) {
    for(i=0; i<256; i++) 
      GammaVal[i]=(unsigned char) (pow(((double) i)/255.0,GammaValue)*255.0+0.5);
    init_done=TRUE;
  }

  for (i=0; i< h; i++) {  /* For each line */
    for (j=0; j<w; j++) { /* For each Y value */
      mf->orig_y[i][j] = GammaVal[mf->orig_y[i][j]];
    }}
}




/*===========================================================================*
 *
 * DoKillDim
 *
 *    Applies an input filter to small Y values.
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    Changes Y values:
 *
 *  Output    |                 /
              |                /
              |               /
              |              !
              |             /
              |            !
              |           /
              |          -
              |        /
              |      --
              |     /
              |   --
              | /
              ------------------------
                        ^ kill_dim_break
                             ^kill_dim_end
              kill_dim_slope gives the slope (y = kill_dim_slope * x +0)
              from 0 to kill_dim_break                      
 *
 *===========================================================================*/

static void
DoKillDim(mf, w, h)
MpegFrame *mf;
int w,h;
{
  static boolean init_done=FALSE;
  static unsigned char mapper[256];
  register int i,j;
  double slope, intercept;

  slope = (kill_dim_end - kill_dim_break*kill_dim_slope)*1.0 /
    (kill_dim_end - kill_dim_break);
  intercept = kill_dim_end * (1.0-slope);

  if (!init_done) {
    for(i=0; i<256; i++) {
      if (i >= kill_dim_end) {
        mapper[i] = (char) i;
      } else if (i >= kill_dim_break) {
        mapper[i] = (char) (slope*i + intercept);
      } else { /* i <= kill_dim_break */
        mapper[i] = (char) floor(i*kill_dim_slope + 0.49999);
      }
    }
    init_done = TRUE;
  }

  for (i=0;  i < h;  i++) {  /* For each line */
    for (j=0;   j < w;   j++) { /* For each Y value */
      mf->orig_y[i][j] = mapper[mf->orig_y[i][j]];
    }}
}
