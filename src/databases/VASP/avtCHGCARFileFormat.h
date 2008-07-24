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


// ************************************************************************* //
//                            avtCHGCARFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_CHGCAR_FILE_FORMAT_H
#define AVT_CHGCAR_FILE_FORMAT_H

#include <avtFileFormatInterface.h>
#include <avtMTSDFileFormat.h>

class vtkDoubleArray;

// ****************************************************************************
//  Class: avtCHGCARFileFormat
//
//  Purpose:
//      Reads in CHGCAR files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
//    Jeremy Meredith, Tue Jul 15 15:41:07 EDT 2008
//    Added support for automatic domain decomposition.
//
// ****************************************************************************

class avtCHGCARFileFormat : public avtMTSDFileFormat
{
  public:
    static bool        Identify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);

                       avtCHGCARFileFormat(const char *filename);
    virtual           ~avtCHGCARFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CHGCAR"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::vector<istream::pos_type>   file_positions;

    int globalZDims[3];
    int globalNDims[3];
    int domainCount[3];
    int domainIndex[3];
    int domainGlobalStart[3];
    int domainGlobalCount[3];
    int localRealStart[3];
    int localRealCount[3];

    double unitCell[3][3];
    bool is_rectilinear;
    ifstream in;
    std::string filename;
    bool metadata_read;
    int  values_read;
    int  values_per_line;
    vtkDoubleArray *values;
    int ntimesteps;
    int natoms;

    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadValues(int);
    void AddGhostCellInfo(vtkDataSet *ds);
    void DoDomainDecomposition();
};


#endif
