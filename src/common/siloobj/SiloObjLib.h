// ************************************************************************* //
//                               SiloObjLib.h                                //
// ************************************************************************* //

#ifndef SILO_OBJ_LIB
#define SILO_OBJ_LIB


//
// Defines
//

#define     LONG_STRING    1000


//
// Function Prototypes
//

char            *CondenseStringArray(char **, int, int *);
void             Construct1DArray(int, int *, int **, int **, int *);
void             InsertionSort(int *, int);
char            *MakeVisitFileList(int, char **, int, int *, char ***, int *);
void             ParseVisitFileList(char *, int &, char **&, int &, int *&,
                                     char ***&);
inline char     *ParsePDBNameString(char *pdbName);
inline double    ParsePDBNameDouble(char *pdbName);
inline float     ParsePDBNameFloat(char *pdbName);
inline int       ParsePDBNameInt(char *pdbName);
void             UncondenseStringArray(char **, int, char *);


// 
// Enumerated types
//

typedef enum
{
    DERIVED_TYPE_CONSTRUCTOR
}   FOR_DERIVED_TYPES_e;


//
// Inline functions
//

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <Utility.h>


// ****************************************************************************
//  Function: ParsePDBNameInt
//
//  Purpose:
//      Parses the pdb_names from a DBobject and returns the integer in the
//      string after checking the type.  Because exception handling is not
//      yet implemented, it just exits on failure.
//
//  Return:     The integer in the argument string.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

inline int
ParsePDBNameInt(char *pdbName)
{
    char   *SILOIntType   = "'<i>";
    int     SILOIntLength = 4;  // = strlen(SILOIntType)

    //
    // Check to make sure the type is truly an int.
    //
    if (strncmp(pdbName, SILOIntType, SILOIntLength) != 0)
    {
        cerr << "Int-typed pdb name has incorrect type." << endl;
        exit(EXIT_FAILURE);
    }   
    
    return atoi(pdbName + SILOIntLength);
}


// ****************************************************************************
//  Function: ParsePDBNameFloat
//
//  Purpose:
//      Parses the pdb_names from a DBobject and returns the float in the
//      string after checking the type.  Because exception handling is not
//      yet implemented, it just exits on failure.
//
//  Return:     The float in the argument string.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

inline float
ParsePDBNameFloat(char *pdbName)
{
    char   *SILOFloatType   = "'<f>";
    int     SILOFloatLength = 4;  // = strlen(SILOFloatType)

    //
    // Check to make sure the type is truly a float.
    //
    if (strncmp(pdbName, SILOFloatType, SILOFloatLength) != 0)
    {
        cerr << "Float-typed pdb name has incorrect type." << endl;
        exit(EXIT_FAILURE);
    }   
    
    //
    // atof actually returns a double.
    //
    double   d = atof(pdbName + SILOFloatLength);
    return static_cast< float >(d);
}


// ****************************************************************************
//  Function: ParsePDBNameDouble
//
//  Purpose:
//      Parses the pdb_names from a DBobject and returns the double in the
//      string after checking the type.  Because exception handling is not
//      yet implemented, it just exits on failure.
//
//  Return:     The double in the argument string.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

inline double
ParsePDBNameDouble(char *pdbName)
{
    char   *SILODoubleType   = "'<d>";
    int     SILODoubleLength = 4;  // = strlen(SILODoubleType)

    //
    // Check to make sure the type is truly a double.
    //
    if (strncmp(pdbName, SILODoubleType, SILODoubleLength) != 0)
    {
        cerr << "Double-typed pdb name has incorrect type." << endl;
        exit(EXIT_FAILURE);
    }   
    
    return atof(pdbName + SILODoubleLength);
}


// ****************************************************************************
//  Function: ParsePDBNameString
//
//  Purpose:
//      Parses the pdb_names from a DBobject and returns the string in the
//      name after checking the type.  Because exception handling is not
//      yet implemented, it just exits on failure.
//
//  Return:     A copy of the sub-string in the argument string.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

inline char *
ParsePDBNameString(char *pdbName)
{
    char   *SILOStringType   = "'<s>";
    int     SILOStringLength = 4;  // = strlen(SILOStringType)

    //
    // Check to make sure the type is truly a String.
    //
    if (strncmp(pdbName, SILOStringType, SILOStringLength) != 0)
    {
        cerr << "String-typed pdb name has incorrect type." << endl;
        exit(EXIT_FAILURE);
    }   
    
    char  *s = CXX_strdup(pdbName + SILOStringLength);
  
    //
    // There is a matching ' at the end of the string.  Take it out.
    //
    int    length = strlen(s);
    s[length-1] = '\0';

    return s;
}


#endif 


