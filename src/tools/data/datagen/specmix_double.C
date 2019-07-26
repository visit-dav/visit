// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/*------------------------------------------------------------------------
 * specmix_double.c -- Species over mixed-material zones.
 *
 *
 * A slightly modified version of specmix.C - which creates species data using
 * doubles instead of floats.
 *
 *
 * Modifications:
 *
 * Mark C. Miller, Thu Sep 16 15:54:01 PDT 2010
 * Avoid wholesale code duplication by using include directive.
 *-----------------------------------------------------------------------*/
#define CMIXTYPE double
#define DBMIXTYPE DB_DOUBLE
#include "specmix.C" 
