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
//                           avtTetradFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_TETRAD_FILE_FORMAT
#define AVT_TETRAD_FILE_FORMAT


#include <avtMTSDFileFormat.h>

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>


// ****************************************************************************
//  Class: avtTetradFileFormat
//
//  Purpose:
//      A file format reader that reads in Tetrad files for INEEL.  Tetrad
//      files are written using HDF5.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2002
//
//  Modifications:
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added timeState arg to PopulateDatabaseMetaData
// ****************************************************************************

class avtTetradFileFormat : public avtMTSDFileFormat
{
  public:
                               avtTetradFileFormat(const char *);
    virtual                   ~avtTetradFileFormat();

    virtual void               GetCycles(std::vector<int> &);
    virtual int                GetNTimesteps(void);

    virtual const char        *GetType(void) { return "Tetrad"; };
    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    
    virtual void               GetVars(const char *);
    virtual void               GetTimesteps(const char *);

  protected:
    hid_t                      file_handle;
    vtkDataSet                *mesh_for_all_timesteps;

    std::vector<std::string>   variables;
    std::vector<std::string>   timesteps;

    vtkDataSet                *ConstructMesh(void);
};


#endif


