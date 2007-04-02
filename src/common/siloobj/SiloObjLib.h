/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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


