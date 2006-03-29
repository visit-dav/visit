#ifndef ATOMIC_PROPERTIES_H
#define ATOMIC_PROPERTIES_H
#include <utility_exports.h>

#define MAX_ELEMENT_NUMBER 109
#define KNOWN_AMINO_ACIDS  23

extern unsigned char aminocolors[KNOWN_AMINO_ACIDS][3];
extern unsigned char shapelycolors[KNOWN_AMINO_ACIDS][3];
extern unsigned char jmolcolors[MAX_ELEMENT_NUMBER][3];
extern unsigned char rasmolcolors[MAX_ELEMENT_NUMBER][3];
extern float         atomic_radius[MAX_ELEMENT_NUMBER];
extern float         covalent_radius[MAX_ELEMENT_NUMBER];

extern const char   *element_names[MAX_ELEMENT_NUMBER];
extern const char   *residue_names[KNOWN_AMINO_ACIDS];

int  UTILITY_API ElementNameToAtomicNumber(const char *element);
int  UTILITY_API ResiduenameToNumber(const char *name);
UTILITY_API const char *NumberToResiduename(int num);
int UTILITY_API NumberOfKnownResidues();
int  UTILITY_API ResiduenameToAbbr(const char *name);
void UTILITY_API AddResiduename(const char *name, const char *longname);
UTILITY_API const char *ResiduenameToLongName(const char *name);
int UTILITY_API ResidueLongnameMaxlen();

#endif
