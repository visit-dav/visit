/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtGULPFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_GULP_FILE_FORMAT_H
#define AVT_GULP_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>


// ****************************************************************************
//  Class: avtGULPFileFormat
//
//  Purpose:
//      Reads in GULP files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 22, 2010
//
//  Modifications:
//    Jeremy Meredith, Mon Jul 30 11:03:21 EDT 2012
//    Added support for binary files, and for forces and potential energies.
// ****************************************************************************

class avtGULPFileFormat : public avtMTSDFileFormat
{
  public:
                       avtGULPFileFormat(const char *filename);
    virtual           ~avtGULPFileFormat() {;};

    virtual int       GetNTimesteps(void);
    virtual void      GetTimes(std::vector<double> &t);

    virtual const char    *GetType(void)   { return "GULP"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::vector<istream::pos_type>   file_positions;

    ifstream            in;
    std::string         filename;
    bool                md_read;
    bool                binary;

    bool                has_force_and_pe;
    int                 ntimesteps;
    int                 current_timestep;

    int                 natoms;
    int                 dimension;
    std::vector<double> times;

    std::vector<float>  x;
    std::vector<float>  y;
    std::vector<float>  z;
    std::vector<float>  vx;
    std::vector<float>  vy;
    std::vector<float>  vz;
    std::vector<float>  fx;
    std::vector<float>  fy;
    std::vector<float>  fz;
    std::vector<float>  pe;


    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadTimestep(int);
};


#endif
