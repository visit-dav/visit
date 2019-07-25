// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ATOMIC_PROPERTIES_H
#define ATOMIC_PROPERTIES_H
#include <utility_exports.h>

#define MAX_ELEMENT_NUMBER 109
#define KNOWN_AMINO_ACIDS  23

//  Modifications:
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1. 
//


extern UTILITY_API unsigned char aminocolors[KNOWN_AMINO_ACIDS][3];
extern UTILITY_API unsigned char shapelycolors[KNOWN_AMINO_ACIDS][3];
extern UTILITY_API unsigned char jmolcolors[MAX_ELEMENT_NUMBER+1][3];
extern UTILITY_API unsigned char rasmolcolors[MAX_ELEMENT_NUMBER+1][3];
extern UTILITY_API float         atomic_radius[MAX_ELEMENT_NUMBER+1];
extern UTILITY_API float         covalent_radius[MAX_ELEMENT_NUMBER+1];

extern UTILITY_API const char   *element_names[MAX_ELEMENT_NUMBER+1];
extern UTILITY_API const char   *residue_names[KNOWN_AMINO_ACIDS];

int  UTILITY_API ElementNameToAtomicNumber(const char *element);
int  UTILITY_API ResiduenameToNumber(const char *name);
UTILITY_API const char *NumberToResiduename(int num);
int UTILITY_API NumberOfKnownResidues();
int  UTILITY_API ResiduenameToAbbr(const char *name);
void UTILITY_API AddResiduename(const char *name, const char *longname);
UTILITY_API const char *ResiduenameToLongName(const char *name);
int UTILITY_API ResidueLongnameMaxlen();

#endif
