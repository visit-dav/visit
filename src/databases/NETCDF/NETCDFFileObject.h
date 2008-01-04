/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef NETCDF_FILE_OBJECT_H
#define NETCDF_FILE_OBJECT_H
#include <string>
#include <visitstream.h>

typedef enum {NO_TYPE, CHARARRAY_TYPE, UCHARARRAY_TYPE, SHORTARRAY_TYPE,
              INTEGERARRAY_TYPE, FLOATARRAY_TYPE,
              DOUBLEARRAY_TYPE, LONGARRAY_TYPE} TypeEnum;

// ****************************************************************************
// Class: NETCDFFileObject
//
// Purpose:
//   Abstract the NETCDF file a little.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 12 10:07:08 PDT 2005
//
// Modifications:
//   
//   Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//   Added method for partial read to support on-the-fly domain decomp
// ****************************************************************************

class NETCDFFileObject
{
public:
    NETCDFFileObject(const char *name); 
    virtual ~NETCDFFileObject();

    bool IsOpen() const;
    bool Open();
    void Close();
    const std::string &GetName() const;
    int  GetFileHandle();

    bool ReadAttribute(const char *attname, TypeEnum *type, int *ndims,
                       int **dims, void **value);
    bool ReadAttribute(const char *varname, const char *attname,
                       TypeEnum *type, int *ndims, int **dims, void **value);
    // Convenience functions
    bool ReadStringAttribute(const char *varname, const char *attname,
                             std::string &attval);
    bool ReadStringAttribute(const char *attname, std::string &attval);

    bool InqVariable(const char *varname, TypeEnum *, int *ndims, int **dims);
    bool ReadVariable(const char *varname, TypeEnum *type, int *ndims,
                      int **dims, void **values);
    bool ReadVariableInto(const char *varname, TypeEnum t, void *arr);
    bool ReadVariableInto(const char *varname, TypeEnum t, 
             const int *const starts, const int *const counts, void *arr);
    bool ReadVariableIntoAsFloat(const char *varname, float *arr);

    bool GetVarId(const char *name, int *varid);

    void HandleError(int status) const;
    void PrintFileContents(ostream &os);
private:
    bool AutoOpen();

    std::string filename;
    int         ncid;
};

//
// Functions to free memory.
//

template <class T>
void free_mem(T *);

void free_void_mem(void *, TypeEnum);

#endif
