/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            avtVistaAle3dFileFormat.h                      //
// ************************************************************************* //

#ifndef AVT_VistaAle3d_FILE_FORMAT_H
#define AVT_VistaAle3d_FILE_FORMAT_H

#include <database_exports.h>

#include <avtMaterial.h>
#include <avtVistaFileFormat.h>

#include <string>
#include <vector>

class vtkFloatArray;

using std::string;
using std::vector;

// ****************************************************************************
//  Class: avtVistaAle3dFileFormat
//
//  Purpose:
//      Reads in Vista files as a plugin to VisIt. 
//
//      Note that a Vista file can be written by either Silo or HDF5 natively.
//      We support both here. Where there is a choice in API data-types, we
//      favor use of HDF5's data-types.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 17, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr 29 12:14:37 PDT 2004
//    Added data members to remember material names/numbers
//    Added GetMaterial method
//    Added GetAuxiliaryData method
//    Added GetFileNameForRead method
//
//    Eric Brugger, Wed May 12 13:42:01 PDT 2004
//    Prefixed some uses of "vector" with "std::".
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added spatialDim data member
//
//    Mark C. Miller, Wed Oct 27 16:02:46 PDT 2004
//    Added GetFileNameForRead
//
// ****************************************************************************

class avtVistaAle3dFileFormat : public avtVistaFileFormat
{

  public:
    static avtFileFormatInterface *CreateInterface(avtVistaFileFormat *vff,
                                       const char *const *filenames, int nList);

                               avtVistaAle3dFileFormat(const char *);
                               avtVistaAle3dFileFormat(const char *,
                                                       avtVistaFileFormat *vff);
    virtual                   ~avtVistaAle3dFileFormat();

    virtual void               FreeUpResources(void); 

    virtual void              *GetAuxiliaryData(const char *var, int,
                                                const char *type, void *args,
                                                DestructorFunction &);

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:

    vtkFloatArray       *ReadVar(int domain, const char *visitName);

    void                 GetFileNameForRead(int dom, char *fileName, int size);


    avtMaterial         *GetMaterial(int, const char *);

    int                  numPieces;
    Node               **pieceNodes;

    int                  spatialDim;

    int                  numMaterials;
    std::vector<int>     materialNumbers;
    std::vector<string>  materialNames;
    int                 *materialNumbersArray;
    const char         **materialNamesArray;

};

#endif
