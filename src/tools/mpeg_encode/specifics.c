/*===========================================================================*
 * specifics.c                                     *
 *                                         *
 *    basic procedures to deal with the specifics file                     *
 *                                         *
 * EXPORTED PROCEDURES:                                 *
 *    Specifics_Init                                 *
 *      Spec_Lookup                                                          *
 *      SpecTypeLookup                                                       *
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
#include "mtypes.h"
#include "frames.h"
#include "frame.h"
#include "fsize.h"
#include "dct.h"
#include "specifics.h"
#include <stdio.h>
#include <string.h>
#include "prototypes.h"

/*====================*
 * System Information *
 *====================*/

#define CPP_LOC "/lib/cpp"

/*==================*
 * GLOBAL VARIABLES *
 *==================*/

extern boolean specificsOn;
extern char specificsFile[];
extern char specificsDefines[];
FrameSpecList *fsl;

/*=====================*
 * Internal procedures *
 *=====================*/

void Parse_Specifics_File _ANSI_ARGS_((FILE *fp));
void Parse_Specifics_File_v1 _ANSI_ARGS_((FILE *fp));
void Parse_Specifics_File_v2 _ANSI_ARGS_((FILE *fp));
FrameSpecList *MakeFslEntry _ANSI_ARGS_((void));
void AddSlc _ANSI_ARGS_((FrameSpecList *c,int snum, int qs));
Block_Specifics *AddBs _ANSI_ARGS_((FrameSpecList *c,int bnum, 
                    boolean rel, int qs));
FrameSpecList *MakeFslEntry _ANSI_ARGS_((void));
#define my_upper(c) (((c>='a') && (c<='z')) ? (c-'a'+'A') : c)
#define CvtType(x) ReallyCvt(my_upper(x))
#define ReallyCvt(x) (x=='I' ? 1 : (x=='P')?2: ((x=='B')?3:-1))
#define SkipToSpace(lp) while ((*lp != ' ') && (*lp != '\n') && (*lp != '\0')) lp++
#define EndString(lp)  ((*lp == '\n') || (*lp == '\0'))

/*=============================================================
 * SPEC FILE FORMAT (version 1):

Specs files are processed with the c preprecoessor, so use C style comments
and #defines if you wish.

frames, blocks, and slices are numbered from 0.
(sorry)

In order by frame number, slice number, block number
(if you skip slices it's fine).
Can have specifics for any frame, block, or slice.
Format:
version N
  Specify the version of the specifics file format (this is 1)
frame N T M
  Sets frame number N to type T and Qscale M
  (type T is I,B,P,other, other means unspec.  I recomend - )
slice M Q
  Sets slice M (in frame N as defined by a previous frame command)
  to qscale Q
block M Q
  Sets block M to qscale Q, in frame N previously specified.

Unspecified frame types are set via the last I frame set, which is forced
to act as the first I of the GOP.
FORCE_ENCODE_LAST_FRAME overrides specifics on the final frame type.
Note that Qscale changes in skipped blocks will be lost!

Version 2:
frames and slices are the same as above, but Q in blocks can be relative, i.e.
+N or -N.  Clipping to 1..31 is done but sequences like
block 1 2
block 2 -3
block 3 +3

has undefined results (as present block 3 would be Qscale 2).

In addition motion vectors can be specified:
block M Q skip
  Says to skip the block  (not really an MV, but....)
block M Q bi fx fy bx by
  Sets block M to quality Q.  It will be a bidirectional block.
  fx/fy is the forward (like a P frame) vector, bx/y is the back
block M Q forw fx fy
block M Q back bx by
  Single directional.

All vectors are specified in HALF PIXEL fixed point units, i.e.
3.5 pixels is 7
To specify a vector but not touch the q factor, set Q to 0

*=============================================================*/


/*=============*
 * Local State *
 *=============*/

static char version = -1;

/*================================================================
 *
 *   Specifics_Init
 *
 *   Cpp's and reads in the specifics file.  Creates fsl data structure.
 *
 *   Returns: nothing
 * 
 *   Modifies: fsl, file specificsFile".cpp"
 *
 *================================================================
 */
void Specifics_Init()
{
  char command[1100];
  FILE *specificsFP;
  
  sprintf(command, "/bin/rm -f %s.cpp", specificsFile);
  system(command);
  sprintf(command, "%s -P %s %s %s.cpp",
      CPP_LOC, specificsDefines, specificsFile, specificsFile);
  system(command);
  strcat(specificsFile, ".cpp");
  if ((specificsFP = fopen(specificsFile, "r")) == NULL) {
    fprintf(stderr, "Error with specifics file, cannot open %s\n", specificsFile);
    exit(1);
  }
  printf("Specifics file: %s\n", specificsFile);
  Parse_Specifics_File(specificsFP);
  sprintf(command, "/bin/rm -f %s.cpp", specificsFile);
  system(command);

}




/*================================================================
 *
 *   Parse_Specifics_File
 *
 *   Read through the file passed in creating the fsl data structure
 *   There is a primary routine, and helpers for the specific versions.
 *
 *   Returns: Nothing
 *
 *   Modifies: fsl
 *
 *================================================================
 */
void Parse_Specifics_File(fp)
FILE *fp;
{
  char line[1024], *lp;
  int vers;

  while ((fgets(line, 1023, fp)) != NULL) {
    lp = &line[0];
    while ((*lp == ' ') || (*lp == '\t')) lp++;
    if (( *lp == '#' ) || (*lp=='\n')) {
      continue;
    }

    switch (my_upper(*lp)) {
    case 'F': case 'S': case 'B':
      fprintf(stderr, "Must specify version at beginning of specifics file\n");
      exit(0);
      break;
    case 'V':
      lp += 7;
      if (1 != sscanf(lp, "%d", &vers)) {
    fprintf(stderr," Improper version line in specs file: %s\n", line);
      } else {
    switch (vers) {
    case 1:
      version = vers;
      Parse_Specifics_File_v1(fp);
      break;
    case 2:
      version = vers;
      Parse_Specifics_File_v2(fp);
      break;
    default:
      fprintf(stderr, "Improper version line in specs file: %s\n", line);
      fprintf(stderr, "\tSpecifics file will be IGNORED.\n");
      specificsOn = FALSE;
      return;
      break;
    }}
      break;
    default:
      fprintf(stderr, "Specifics file: What? *%s*\n", line);
      break;
    }}
  
}

/* Version 1 */
void Parse_Specifics_File_v1(fp)
FILE *fp;
{
  char line[1024],*lp;
  FrameSpecList *current, *new;
  char typ; 
  int fnum,snum, bnum, qs, newqs;
  int num_scanned;

  fsl = MakeFslEntry();
  current = fsl;

  while ((fgets(line,1023, fp)) != NULL) {
    lp = &line[0];
    while ((*lp == ' ') || (*lp == '\t')) lp++;
    if (( *lp == '#' ) || (*lp=='\n')) {
      continue;
    }

    switch (my_upper(*lp)) {
    case 'F':
      lp += 6;
      sscanf(lp, "%d %c %d", &fnum, &typ, &qs);
      if (current->framenum != -1) {
    new=MakeFslEntry();
    current->next = new;
    current = new;
      }
      current->framenum = fnum;
      current->frametype = CvtType(typ);
      if (qs <= 0) qs = -1;
      current->qscale = qs;
      break;
    case 'S':
      lp += 6;
      sscanf(lp, "%d %d", &snum, &newqs);
      if (qs == newqs) break;
      qs = newqs;
      AddSlc(current, snum, qs);
      break;
    case 'B':
      lp += 6;
      num_scanned = sscanf(lp, "%d %d", &bnum, &newqs);
      if (qs == newqs) break;
      qs = newqs;
      AddBs(current, bnum, FALSE, qs);
      break;
    case 'V':
      fprintf(stderr, "Cannot specify version twice!  Taking first (%d)\n", version);
      break;
    default:
      fprintf(stderr," What? *%s*\n", line);
      break;
    }}
  
}

/* Version 2 */
void Parse_Specifics_File_v2(fp)
FILE *fp;
{
  char line[1024], *lp;
  FrameSpecList *current, *new;
  char typ;
  int fnum, snum, bnum, qs, newqs;
  int num_scanned, fx=0, fy=0, sx=0, sy=0;
  char kind[100];
  Block_Specifics *new_blk;
  boolean relative;

  fsl = MakeFslEntry();
  current = fsl;

  while ((fgets(line,1023,fp))!=NULL) {
    lp = &line[0];
    while ((*lp == ' ') || (*lp == '\t')) lp++;
    if (( *lp == '#' ) || (*lp=='\n')) {
      continue;
    }

    switch (my_upper(*lp)) {
    case 'F':
      lp += 6;
      sscanf(lp,"%d %c %d", &fnum, &typ, &qs);
      new = MakeFslEntry();
      if (current->framenum != -1) {
    current->next = new;
    current = new;
      }
      current->framenum = fnum;
      current->frametype = CvtType(typ);
      if (qs <= 0) qs = -1;
      current->qscale = qs;
      break;
    case 'S':
      lp += 6;
      sscanf(lp,"%d %d", &snum, &newqs);
      if (qs == newqs) break;
      qs = newqs;
      AddSlc(current, snum, qs);
      break;
    case 'B':
      lp += 6;
      num_scanned = 0;
      bnum = atoi(lp);
      SkipToSpace(lp);
      while ((*lp != '-') && (*lp != '+') &&
         ((*lp < '0') || (*lp > '9'))) lp++;
      relative = ((*lp == '-') || (*lp == '+'));
      newqs = atoi(lp);
      SkipToSpace(lp);
      if (EndString(lp)) {
    num_scanned = 2;
      } else {
    num_scanned = 2+sscanf(lp, "%s %d %d %d %d", kind, &fx, &fy, &sx, &sy); 
      }

      qs = newqs;
      new_blk = AddBs(current, bnum, relative, qs);
      if (num_scanned > 2) {
    BlockMV *tmp;
    tmp = (BlockMV *) malloc(sizeof(BlockMV));
    switch (num_scanned) {
    case 7:
      tmp->typ = TYP_BOTH;
      tmp->fx = fx;
      tmp->fy = fy;
      tmp->bx = sx;
      tmp->by = sy;
      new_blk->mv = tmp;
      break;
    case 3:
      tmp->typ = TYP_SKIP;
      new_blk->mv = tmp;
      break;
    case 5:
      if (my_upper(kind[0]) == 'B') {
        tmp->typ = TYP_BACK;
        tmp->bx = fx;
        tmp->by = fy;
      } else {
        tmp->typ = TYP_FORW;
        tmp->fx = fx;
        tmp->fy = fy;
      }
      new_blk->mv = tmp;
      break;
    default:
      fprintf(stderr,
          "Bug in specifics file!  Skipping short/long entry: %s\n",line);
      break;
    }
      } else {
    new_blk->mv = (BlockMV *) NULL;
      }

      break;
    case 'V':
      fprintf(stderr,
          "Cannot specify version twice!  Taking first (%d).\n",
          version);
      break;
    default:
      printf("What? *%s*\n",line);
      break;
    }}
  
}




/*=================================================================
 *
 *     MakeFslEntry
 *
 *     Makes a single entry in for the fsl linked list (makes a frame)
 *
 *     Returns: the new entry
 *
 *     Modifies: nothing
 *
 *=================================================================
 */
FrameSpecList *MakeFslEntry()
{
  FrameSpecList *fslp;
  fslp = (FrameSpecList *) malloc(sizeof(FrameSpecList));
  fslp->framenum = -1;
  fslp->slc = (Slice_Specifics *) NULL;
  fslp->bs = (Block_Specifics *) NULL;
  return fslp;
}





/*================================================================
 *
 *   AddSlc
 *
 *   Adds a slice to framespeclist c with values snum and qs
 *
 *   Returns: nothing
 *
 *   Modifies: fsl's structure
 *
 *================================================================
 */
void AddSlc(c, snum, qs)
FrameSpecList *c;
int snum,qs;
{
  Slice_Specifics *new;
  static Slice_Specifics *last;

  new = (Slice_Specifics *) malloc(sizeof(Slice_Specifics));
  new->num = snum;
  new->qscale = qs;
  new->next = (Slice_Specifics *)NULL;
  if (c->slc == (Slice_Specifics *)NULL) {
    last = new;
    c->slc = new;
  } else {
    last->next = new;
    last = new;
  }
}





/*================================================================
 *
 *   AddBs
 *
 *   Adds a sliceblock to framespeclist c with values bnum and qs
 *
 *   Returns: pointer to the new block spec
 *
 *   Modifies: fsl's structure
 *
 *================================================================
 */
Block_Specifics *AddBs(c,bnum,rel,qs)
FrameSpecList *c;
boolean rel;
int bnum,qs;
{
  Block_Specifics *new;
  static Block_Specifics *last;

  new = (Block_Specifics *) malloc(sizeof(Block_Specifics));
  new->num = bnum;
  if (qs == 0) rel = TRUE;
  new->relative = rel;
  new->qscale = qs;
  new->next = (Block_Specifics *)NULL;
  new->mv = (BlockMV *) NULL;
  if (c->bs == (Block_Specifics *)NULL) {
    last = new;
    c->bs = new;
  } else {
    last->next = new;
    last = new;
  }
  return new;
}






/*================================================================
 *
 *  SpecLookup
 *
 *  Find out if there is any changes to be made for the qscale
 *  at entry fn.num (which is of type typ).  Sets info to point to
 *  motion vector info (if any), else NULL.
 *
 *  Returns: new qscale or -1
 *
 *  Modifies: *info (well, internal cache can change)
 *
 *================================================================
 */

int SpecLookup(fn,typ,num,info,start_qs)
int fn,typ,num;
BlockMV **info;
int start_qs;
{
  static FrameSpecList *last = (FrameSpecList *) NULL;
  Slice_Specifics *sptr=(Slice_Specifics *) NULL;
  Block_Specifics *bptr=(Block_Specifics *) NULL;
  FrameSpecList *tmp;
  boolean found_it;
  static int leftovers = 0;  /* Used in case of forced movement into 1..31 range */
  
  *info = (BlockMV * )NULL;
  if (last == (FrameSpecList *) NULL){
    /* No cache, try to find number fn */
    tmp = fsl;
    found_it = FALSE;
    while (tmp != (FrameSpecList *) NULL) {
      if (tmp->framenum == fn) {
    found_it = TRUE;
    break;
      } else tmp = tmp->next;
    }
    if (!found_it) return -1;
    last=tmp;
  } else {
    if (last->framenum != fn) { /* cache miss! */
      /* first check if it is next */
      if ((last->next != (FrameSpecList *) NULL) && 
      (last->next->framenum == fn)) {
    last = last->next;
      } else {
    /* if not next, check from the start.
       (this allows people to put frames out of order,even
       though the spec doesnt allow it.) */
    tmp = fsl;
    found_it = FALSE;
    while (tmp != (FrameSpecList *) NULL) {
      if (tmp->framenum==fn) {found_it = TRUE; break;}
      tmp = tmp->next;
    }
    if (!found_it) return -1;
    last = tmp;
      }
    }
  }
  /* neither of these should ever be true, unless there is a bug above */
  if (last == (FrameSpecList *) NULL) {
    fprintf(stderr, "PROGRAMMER ERROR: last is null!\n");
    return -1;
  }
  if (last->framenum!=fn) {
    fprintf(stderr, "PROGRAMMER ERROR: last has wrong number!\n");
    return -1; /* no data on it */
  }
  
  switch(typ) {
  case 0: /* Frame: num is ignored */
    leftovers = 0;
#ifdef BLEAH
    printf("QSchange frame %d to %d\n", fn, last->qscale);
#endif 
    return last->qscale;
    break;

  case 1: /* Slice */
    leftovers = 0;
    /* So, any data on slices? */
    if (last->slc == (Slice_Specifics *) NULL) return -1;
    for (sptr = last->slc; sptr != (Slice_Specifics *) NULL; sptr = sptr->next) {
      if (sptr->num == num) {
#ifdef BLEAH
    printf("QSchange Slice %d.%d to %d\n", fn, num, sptr->qscale);
#endif
    if (sptr->qscale == 0) return -1;
    return sptr->qscale;
      }
    }
    break;

  case 2:  /* block */
    /* So, any data on blocks? */
    if (last->bs == (Block_Specifics *) NULL) {
      return -1;
    }
    for (bptr=last->bs; bptr != (Block_Specifics *) NULL; bptr=bptr->next) {
      if (bptr->num == num) {
    int new_one;
#ifdef BLEAH
    printf("QSchange Block %d.%d to %d\n", fn, num, bptr->qscale);
#endif
    *info = bptr->mv;
    if (bptr->relative) {
      if (bptr->qscale == 0) {
        /* Do nothing! */
        new_one = start_qs;
      } else {
        new_one = start_qs + bptr->qscale + leftovers;
        if (new_one < 1) {
          leftovers = new_one - 1;
          new_one = 1;
        } else if (new_one > 31) {
          leftovers = new_one - 31;
          new_one = 31;
        } else leftovers = 0;
      }}
    else {
      new_one = bptr->qscale;
      leftovers = 0;
    }
    return new_one;
      }
    }
    break;
  default:
    fprintf(stderr, "PROGRAMMER ERROR:  reached unreachable code in SpecLookup\n");
    break;
  }
  /* no luck */
  return -1;
}
     
    
/*================================================================
 *
 *  SpecTypeLookup
 *
 *  Find out if there is any changes to be made for the type of frame
 *  at frame fn.
 *
 *  Returns: new type or -1 (unspecified)
 *
 *================================================================
 */
int SpecTypeLookup(fn)
int fn;
{
  FrameSpecList *tmp;

  /* try to find number fn */
  tmp = fsl;
  do {
    if (tmp->framenum == fn) break;
    else tmp = tmp->next;
  } while (tmp != (FrameSpecList *) NULL);
  if (tmp == (FrameSpecList *) NULL) {
#ifdef BLEAH
    printf("Frame %d type not specified\n", fn);
#endif
    return -1;
  }
#ifdef BLEAH
  printf("Frame %d type set to %d\n", fn, tmp->frametype);
#endif
  return tmp->frametype;
}
