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

// ************************************************************************* //
//                            avtPFLOTRANFileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_PFLOTRAN_FILE_FORMAT_H
#define AVT_PFLOTRAN_FILE_FORMAT_H

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
#include <avtMTMDFileFormat.h>

#include <vector>


// ****************************************************************************
//  Class: avtPFLOTRANFileFormat
//
//  Purpose:
//      Reads in PFLOTRAN files as a plugin to VisIt.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  1 12:43:11 EDT 2008
//    Added support for automatic parallel decomposition and parallel I/O
//    via hyperslab reading.
//
//    Jeremy Meredith, Fri Apr  3 12:52:57 EDT 2009
//    Added support for zonal variables, and recentering old-style files
//    that used cell centers for the coordinate arrays.
//
// ****************************************************************************

class avtPFLOTRANFileFormat : public avtMTMDFileFormat
{
  public:
                       avtPFLOTRANFileFormat(const char *);
    virtual           ~avtPFLOTRANFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int timestep, 
    //                                     int domain, const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    // virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "PFLOTRAN"; };
    virtual void           FreeUpResources(void); 


    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    char *filename;
    bool opened;

    int nTime;
    std::vector< std::pair<float,std::string> > times;
    int domainCount[3];
    int domainIndex[3];
    int globalDims[3];
    int domainGlobalStart[3];
    int domainGlobalCount[3];
    int localRealStart[3];
    int localRealCount[3];
    hid_t fileID;
    hid_t dimID[3];

    bool    oldFileNeedingCoordFixup;

    void LoadFile(void);
    void AddGhostCellInfo(vtkDataSet *ds);
    void DoDomainDecomposition();

    //HDF5 helper functions.
    bool ReadAttribute( hid_t parentID, const char *attr, void *value );
    bool ReadStringAttribute( hid_t parentID, const char *attr, std::string *value );
    hid_t NormalizeH5Type( hid_t type );


    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
