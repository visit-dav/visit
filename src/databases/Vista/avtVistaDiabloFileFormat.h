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
//                            avtVistaDiabloFileFormat.h                     //
// ************************************************************************* //

#ifndef AVT_VistaDiablo_FILE_FORMAT_H
#define AVT_VistaDiablo_FILE_FORMAT_H

#include <database_exports.h>

#include <avtVistaFileFormat.h>

#include <string>
#include <vector>

class vtkDataArray;
class vtkDataSet;
class vtkFloatArray;

using std::string;
using std::vector;

// ****************************************************************************
//  Class: avtVistaDiabloFileFormat
//
//  Purpose:
//      Reads Vista files written by the Diablo code 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 17, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct 27 16:02:46 PDT 2004
//    Added GetFileNameForRead
//
// ****************************************************************************

class avtVistaDiabloFileFormat : public avtVistaFileFormat
{

  public:
    static avtFileFormatInterface *CreateInterface(avtVistaFileFormat *vff,
                                       const char *const *filenames, int nList);

                               avtVistaDiabloFileFormat(const char *);
                               avtVistaDiabloFileFormat(const char *,
                                                       avtVistaFileFormat *vff);
    virtual                   ~avtVistaDiabloFileFormat();

    virtual void               FreeUpResources(void); 

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:

    typedef struct _fieldInfo
    {
        string         visitVarName;
        string         meshName;
        avtVarType     varType;
        avtCentering   centering;
        vector<string> compNames;
    } FieldInfo_t;

    vtkFloatArray       *ReadVar(int domain, const char *visitName);

    void                 GetFileNameForRead(int dom, char *fileName, int size);

    int                  numPieces;
    Node               **pieceNodes;
    vector<vector<FieldInfo_t> > fieldInfos;

    int                  spatialDim;
};


#endif
