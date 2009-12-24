/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef PDBFILEOBJECT_H
#define PDBFILEOBJECT_H

#include <visit-config.h>

#ifdef HAVE_PDB_PROPER
#include <pdb.h>
#define PDBLIB_ERRORSTRING PD_err
#else
#include <lite_pdb.h>
#ifdef _WIN32
#define PDBLIB_ERRORSTRING "(don't have the PD_err error string)"
#else
#define PDBLIB_ERRORSTRING PD_err
#endif
#endif

#include <string>

//
// Enum for array type definitions.
//
typedef enum {NO_TYPE, CHAR_TYPE, INTEGER_TYPE, FLOAT_TYPE, DOUBLE_TYPE,
              LONG_TYPE, CHARARRAY_TYPE, INTEGERARRAY_TYPE, FLOATARRAY_TYPE,
              DOUBLEARRAY_TYPE, LONGARRAY_TYPE, OBJECT_TYPE} TypeEnum;

// ****************************************************************************
// Class: PDBFileObject
//
// Purpose:
//   This class encapsulates a PDB file and provides methods to read out
//   certain types of values.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:00:58 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Jul 12 10:24:09 PDT 2004
//   Added a static Destruct function.
//
// ****************************************************************************

class PDBFileObject
{
public:
    PDBFileObject(const char *name);
    virtual ~PDBFileObject();

    bool IsOpen() const;
    bool Open();
    void Close();
    const std::string &GetName() const;
    PDBfile *filePointer();

    bool GetDouble(const char *name, double *val);
    bool GetDoubleArray(const char *name, double **val, int *nvals);
    bool GetInteger(const char *name, int *val);
    bool GetIntegerArray(const char *name, int **val, int *nvals);
    bool GetString(const char *name, char **str, int *len = 0);
    bool SymbolExists(const char *name);
    bool SymbolExists(const char *, TypeEnum *, int *, int **, int*);
    bool SymbolExists(const char *, TypeEnum *, std::string &, int *, int **, int*);
    void *ReadValues(const char *, TypeEnum *, int *, int **, int*, int=0);

    static void Destruct(void *ptr);
private:
    bool AutoOpen();

    std::string  filename;
    PDBfile     *pdb;
};

//
// Functions to free memory.
//

template <class T>
void free_mem(T *);

void pdb_free_void_mem(void *, TypeEnum);

#endif
