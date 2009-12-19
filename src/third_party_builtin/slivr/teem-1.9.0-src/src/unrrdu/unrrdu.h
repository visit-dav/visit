/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef UNRRDU_HAS_BEEN_INCLUDED
#define UNRRDU_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(unrrdu_EXPORTS) || defined(teem_EXPORTS)
#    define UNRRDU_EXPORT extern __declspec(dllexport)
#  else
#    define UNRRDU_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define UNRRDU_EXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define UNRRDU unrrduBiffKey

#define UNRRDU_COLUMNS 78  /* how many chars per line do we allow hest */

/*
******** unrrduCmd
**
** How we associate the one word for the unu command ("name"),
** the one-line info string ("info"), and the single function we
** which implements the command ("main").
*/
typedef struct {
  const char *name, *info;
  int (*main)(int argc, char **argv, char *me, hestParm *hparm);
} unrrduCmd;

/*
** UNRRDU_DECLARE, UNRRDU_LIST, UNRRDU_MAP
** 
** Twisted C-preprocessor tricks.  The idea is to make it as simple
** as possible to add new commands to unu, so that the new commands
** have to be added to only one thing in this source file, and
** the Makefile.
** 
** Associated with each unu command are some pieces of information:
** the single word command (e.g. "slice") that is used by invoke it,
** the short (approx. one-line) description of its function, and the
** "main" function to call with the appropriate argc, argv.  It would
** be nice to use a struct to hold this information, and we can: the
** unrrduCmd struct is defined above.  It would also be nice to have
** all the command's information be held in one array of unrrduCmds.
** Unfortunately, declaring this is not possible unless all the
** unrrduCmds and their fields are IN THIS FILE, because otherwise
** they're not constant expressions, so they can't initialize an
** aggregate data type.  So, we instead make an array of unrrduCmd
** POINTERS, which can be initialized with the addresses of individual
** unrrduCmd structs, declared and defined in the global scope. is
** done in flotsam.c.  Each of the source files for the various unu
** commands are responsible for setting the fields (at compile-time)
** of their associated unrrduCmd.
**
** We use three macros to automate this somewhat:
** UNRRDU_DECLARE: declares unrrdu_xxxCmd as an extern unrrduCmd
**                 (defined in xxx.c), used later in this header file.
** UNRRDU_LIST:    the address of unrrdu_xxxCmd, for listing in the array of
**                 unrrduCmd structs in the (compile-time) definition of 
**                 unrrduCmdList[].  This is used in flotsam.c.
**
** Then, to facilitate running these macros on each of the different
** commands, there is a UNRRDU_MAP macro which is used to essentially map
** the two macros above over the list of unu commands.  Functional
** programming meets the C pre-processor.  Therefore:
***********************************************************
    You add commands to unu by:
    1) adjusting the definition of UNRRDU_MAP()
    2) listing the appropriate object in GNUmakefile and CMakeLists.txt
    That's it.
********************************************************** */
#define UNRRDU_DECLARE(C) UNRRDU_EXPORT unrrduCmd unrrdu_##C##Cmd;
#define UNRRDU_LIST(C) &unrrdu_##C##Cmd,
#define UNRRDU_MAP(F) \
F(about) \
F(env) \
F(make) \
F(head) \
F(data) \
F(convert) \
F(resample) \
F(cmedian) \
F(minmax) \
F(quantize) \
F(unquantize) \
F(project) \
F(slice) \
F(dice) \
F(splice) \
F(join) \
F(crop) \
F(inset) \
F(pad) \
F(reshape) \
F(permute) \
F(swap) \
F(shuffle) \
F(flip) \
F(unorient) \
F(axinfo) \
F(axinsert) \
F(axsplit) \
F(axdelete) \
F(axmerge) \
F(tile) \
F(untile) \
F(histo) \
F(dhisto) \
F(jhisto) \
F(histax) \
F(heq) \
F(gamma) \
F(1op) \
F(2op) \
F(3op) \
F(lut) \
F(mlut) \
F(subst) \
F(rmap) \
F(mrmap) \
F(imap) \
F(lut2) \
F(ccfind) \
F(ccadj) \
F(ccmerge) \
F(ccsettle) \
F(save)
/* these two have been removed since no one uses them
F(block) \
F(unblock) \
*/

/*
******** UNRRDU_CMD
**
** This is used at the very end of the various command sources
** ("xxx.c") to simplify defining a unrrduCmd.  "name" should just be
** the command, UNQUOTED, such as flip or slice.
*/
#define UNRRDU_CMD(name, info) \
unrrduCmd unrrdu_##name##Cmd = { #name, info, unrrdu_##name##Main }

/* xxx.c */
/* Declare the extern unrrduCmds unrrdu_xxxCmd, for all xxx.  These are
   defined in as many different source files as there are commands. */
UNRRDU_MAP(UNRRDU_DECLARE)

/* flotsam.c */
UNRRDU_EXPORT const char *unrrduBiffKey;
UNRRDU_EXPORT int unrrduDefNumColumns;
/* addresses of all unrrdu_xxxCmd */
UNRRDU_EXPORT unrrduCmd *unrrduCmdList[]; 
UNRRDU_EXPORT void unrrduUsage(const char *me, hestParm *hparm);
UNRRDU_EXPORT hestCB unrrduHestPosCB;
UNRRDU_EXPORT hestCB unrrduHestMaybeTypeCB;
UNRRDU_EXPORT hestCB unrrduHestScaleCB;
UNRRDU_EXPORT hestCB unrrduHestBitsCB;
UNRRDU_EXPORT hestCB unrrduHestFileCB;
UNRRDU_EXPORT hestCB unrrduHestEncodingCB;


#ifdef __cplusplus
}
#endif

#endif /* UNRRDU_HAS_BEEN_INCLUDED */
