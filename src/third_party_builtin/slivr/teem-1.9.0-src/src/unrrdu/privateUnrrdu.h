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

#ifndef UNRRDU_PRIVATE_HAS_BEEN_INCLUDED
#define UNRRDU_PRIVATE_HAS_BEEN_INCLUDED

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
** OPT_ADD_...
**
** These macros are used for setting up command-line options for the various
** unu commands.  They define options which are common across many different
** commands, so that the unu interface is as consistent as possible.  They
** all assume a hestOpt *opt variable, but they take the option variable
** and option description as arguments.  The expected type of the variable
** is given before each macro.
*/
/* Nrrd *var */
#define OPT_ADD_NIN(var, desc) \
  hestOptAdd(&opt, "i,input", "nin", airTypeOther, 1, 1, &(var), "-", desc, \
             NULL, NULL, nrrdHestNrrd)

/* char *var */
#define OPT_ADD_NOUT(var, desc) \
  hestOptAdd(&opt, "o,output", "nout", airTypeString, 1, 1, &(var), "-", desc)

/* unsigned int var */
#define OPT_ADD_AXIS(var, desc) \
  hestOptAdd(&opt, "a,axis", "axis", airTypeUInt, 1, 1, &(var), NULL, desc)

/* int *var; int saw */
#define OPT_ADD_BOUND(name, var, desc, saw) \
  hestOptAdd(&opt, name, "pos0", airTypeOther, 1, -1, &(var), NULL, desc, \
             &(saw), NULL, &unrrduHestPosCB)

/* int var */
#define OPT_ADD_TYPE(var, desc, dflt) \
  hestOptAdd(&opt, "t,type", "type", airTypeEnum, 1, 1, &(var), dflt, desc, \
             NULL, nrrdType)

/*
** USAGE, PARSE, SAVE
**
** These are macros at their very worst.  Shudder.  This code is
** basically the same, verbatim, across all the different unrrdu
** functions, and having them as macros just shortens (without
** necessarily clarifying) their code.
**
** They all assume many many variables.
*/
#define USAGE(info) \
  if (!argc) { \
    hestInfo(stderr, me, (info), hparm); \
    hestUsage(stderr, opt, me, hparm); \
    hestGlossary(stderr, opt, hparm); \
    airMopError(mop); \
    return 1; \
  }

  /*

I nixed this because it meant unu invocations with only a 
few args (less than hestMinNumArgs()), which were botched
because they were missing options, were not being described
in the error messages.

**
** NB: below is an unidiomatic use of hestMinNumArgs(), because of
** how unu's main invokes the "main" function of the different
** commands.  Normally the comparison is with argc-1, or argc-2
** the case of cvs-like commands.


  if ( (hparm->respFileEnable && !argc) || \
       (!hparm->respFileEnable && argc < hestMinNumArgs(opt)) ) { \
  */

#define PARSE() \
  if ((pret=hestParse(opt, argc, argv, &err, hparm))) { \
    if (1 == pret) { \
      fprintf(stderr, "%s: %s\n", me, err); free(err); \
      hestUsage(stderr, opt, me, hparm); \
      hestGlossary(stderr, opt, hparm); \
      airMopError(mop); \
      return 1; \
    } else { \
      /* ... like tears ... in rain. Time ... to die. */ \
      exit(1); \
    } \
  }

#define SAVE(outS, nout, io) \
  if (nrrdSave((outS), (nout), (io))) { \
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways); \
    fprintf(stderr, "%s: error saving nrrd to \"%s\":\n%s\n", me, (outS), err); \
    airMopError(mop); \
    return 1; \
  }

#ifdef __cplusplus
}
#endif

#endif /* UNRRDU_PRIVATE_HAS_BEEN_INCLUDED */
