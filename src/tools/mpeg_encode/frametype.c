/*===========================================================================*
 * frametype.c                                     *
 *                                         *
 *    procedures to keep track of frame types (I, P, B)             *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    FType_Type                                     *
 *    FType_FutureRef                                     *
 *    FType_PastRef                                     *
 *                                         *
 * SYNOPSIS                                     *
 *    FType_Type    returns the type of the given numbered frame         *
 *    FType_FutureRef    returns the number of the future reference frame     *
 *    FType_PastRef    returns the number of the past reference frame         *
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


/*==============*
 * HEADER FILES *
 *==============*/

#include "all.h"
#include "prototypes.h"
#include "frames.h"
#include "frame.h"
#include "param.h"


static FrameTable *frameTable=NULL;
static boolean use_cache = FALSE;
static int firstI = 0;

/*==================*
 * GLOBAL VARIABLES *
 *==================*/

boolean        forceEncodeLast = FALSE;
extern int framePatternLen;
extern char *framePattern;


/*=====================*
 * EXPORTED PROCEDURES *
 *=====================*/

/*===========================================================================*
 *
 * FType_Type
 *
 *    returns the type of the given numbered frame
 *
 * RETURNS:    the type
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
FType_Type(frameNum)
    int frameNum;
{
  if (use_cache) return (int)frameTable[frameNum].typ;
  
  if ( forceEncodeLast && (frameNum+1 == numInputFiles) ) {
    int result;
    
    result = framePattern[frameNum % framePatternLen];
    if ( result == 'b' ) return 'i';
    else return result;
  } else {
    if (specificsOn) {
      static int lastI = -1;
      int newtype;
      
      if (lastI > frameNum) lastI = -1;
      newtype = SpecTypeLookup(frameNum);
      switch (newtype) {
      case 1:
    lastI = frameNum;
    return 'i';
      case 2:
    return 'p';
      case 3:
    return 'b';
      default:
    if (lastI != -1) return framePattern[(frameNum-lastI+firstI) % framePatternLen];
    else return framePattern[frameNum % framePatternLen];
      }
    } else return framePattern[frameNum % framePatternLen];
  }
}


/*===========================================================================*
 *
 * FType_FutureRef
 *
 *    returns the number of the future reference frame
 *
 * RETURNS:    the number; -1 if none
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
FType_FutureRef(currFrameNum)
    int currFrameNum;
{
    int        index;
    int        futureIndex;
    int        result;

    if (use_cache) {
      return frameTable[currFrameNum].next->number;
    } else {
      index = currFrameNum % framePatternLen;
      futureIndex = frameTable[index].next->number;
      
      result = currFrameNum +
    (((futureIndex-index)+framePatternLen) % framePatternLen);
      
      if ( (result >= numInputFiles) && forceEncodeLast ) {
    return numInputFiles-1;
      } else {
    return result;
      }
    }
}


/*===========================================================================*
 *
 * FType_PastRef
 *
 *    returns the number of the past reference frame
 *
 * RETURNS:    the number
 *
 * SIDE EFFECTS:    none
 *
 *===========================================================================*/
int
FType_PastRef(currFrameNum)
    int currFrameNum;
{
    int        index;
    int        pastIndex;

    if (use_cache) {
      return frameTable[currFrameNum].prev->number;
    } else {
      index = currFrameNum % framePatternLen;
      pastIndex = frameTable[index].prev->number;
      
      return currFrameNum -
    (((index-pastIndex)+framePatternLen) % framePatternLen);
    }
}


/*===========================================================================*
 *
 * SetFramePattern
 *
 *    set the IPB pattern; calls ComputeFrameTable to set up table
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    framePattern, framePatternLen, frameTable
 *
 *===========================================================================*/
#define SIMPLE_ASCII_UPPER(x)  (((x)>='a') ? ((x)-'a'+'A') : (x))
void
SetFramePattern(pattern)
    char *pattern;
{
    int len = strlen(pattern);
    char *buf;
    int index;

    if ( ! pattern ) {
    fprintf(stderr, "pattern cannot be NULL\n");
    exit(1);
    }

    if ( SIMPLE_ASCII_UPPER(pattern[0]) != 'I' ) {
      for (index=0; index < len; index++) {

    if (SIMPLE_ASCII_UPPER(pattern[index]) == 'I') {
      break;
    } else if (SIMPLE_ASCII_UPPER(pattern[index]) == 'P') {
      fprintf(stderr, "first reference frame must be 'i'\n");
      exit(1);
    }
      }
    }

    buf = (char *)malloc(sizeof(char)*(len+1));
    ERRCHK(buf, "malloc");

    firstI = -1;
    for ( index = 0; index < len; index++ ) {
      switch( SIMPLE_ASCII_UPPER(pattern[index]) ) {
      case 'I':    
    buf[index] = 'i';
    if (firstI == -1) firstI = index;
    break;
      case 'P':    
    buf[index] = 'p'; 
    break;
      case 'B':    
    buf[index] = 'b';
    break;
      default:
    fprintf(stderr, "Frame type '%c' not supported.\n", pattern[index]);
    exit(1);
      }
    }
    buf[len] = 0;

    if (firstI == -1) {
      fprintf(stderr, "Must have an I-frame in PATTERN\n");
      exit(1);
    }

    framePattern = buf;
    framePatternLen = len;

    /* Used to ComputeFrameTable(), but now must wait until param parsed. (STDIN or not)*/
}


/*===========================================================================*
 *
 * ComputeFrameTable
 *
 *    compute a table of I, P, B frames to help in determining dependencies
 *
 * RETURNS:    nothing
 *
 * SIDE EFFECTS:    frameTable
 *
 *===========================================================================*/
void
ComputeFrameTable()
{
    register int index;
    FrameTable    *lastI, *lastIP, *firstB, *secondIP;
    FrameTable    *ptr;
    char typ;
    int table_size;

    if (!stdinUsed) {
      table_size = numInputFiles;
    } else {
      table_size = framePatternLen;
    }

    frameTable = (FrameTable *) malloc((1+table_size)*sizeof(FrameTable));
    ERRCHK(frameTable, "malloc");

    lastI = NULL;
    lastIP = NULL;
    firstB = NULL;
    secondIP = NULL;
    for ( index = 0; index < table_size; index++ ) {
    frameTable[index].number = index;
    typ = FType_Type(index);
    frameTable[index].typ = typ;
    switch( typ ) {
        case 'i':
        ptr = firstB;
        while ( ptr != NULL ) {
            ptr->next = &(frameTable[index]);
            ptr = ptr->nextOutput;
        }
        frameTable[index].nextOutput = firstB;
        frameTable[index].prev = lastIP;    /* for freeing */
        if ( lastIP != NULL ) {
            lastIP->next = &(frameTable[index]);
            if ( secondIP == NULL ) {
            secondIP = &(frameTable[index]);
            }
        }
        lastIP = &(frameTable[index]);
        firstB = NULL;
        break;
        case 'p':
        ptr = firstB;
        while ( ptr != NULL ) {
            ptr->next = &(frameTable[index]);
            ptr = ptr->nextOutput;
        }
        frameTable[index].nextOutput = firstB;
        frameTable[index].prev = lastIP;
        if ( lastIP != NULL ) {
            lastIP->next = &(frameTable[index]);
            if ( secondIP == NULL ) {
            secondIP = &(frameTable[index]);
            }
        }
        lastIP = &(frameTable[index]);
        firstB = NULL;
        break;
        case 'b':
        if ( (index+1 == framePatternLen) ||
             (FType_Type(index+1) != 'b') ) {
            frameTable[index].nextOutput = NULL;
        } else {
            frameTable[index].nextOutput = &(frameTable[index+1]);
        }
        frameTable[index].prev = lastIP;
        if ( firstB == NULL ) {
            firstB = &(frameTable[index]);
        }
        break;
        default:
            fprintf(stderr, "Programmer Error in ComputeFrameTable (%d)\n",
            framePattern[index]);
            exit(1);
            break;
    }
    }
    
    /* why? SRS */
    frameTable[table_size].number = framePatternLen;
    ptr = firstB;
    while ( ptr != NULL ) {
    ptr->next = &(frameTable[table_size]);
    ptr = ptr->nextOutput;
    }
    frameTable[table_size].nextOutput = firstB;
    frameTable[table_size].prev = lastIP;
    if ( secondIP == NULL )
    frameTable[table_size].next = &(frameTable[0]);
    else
    frameTable[table_size].next = secondIP;

    frameTable[0].prev = lastIP;
    if ( lastIP != NULL ) {
    lastIP->next = &(frameTable[table_size]);
    }

    if (!stdinUsed) {
      use_cache = TRUE;
    }
}

