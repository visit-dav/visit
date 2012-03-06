/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                            avtPuReMDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_PuReMD_FILE_FORMAT_H
#define AVT_PuReMD_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <utility>
#include <set>


// ****************************************************************************
//  Class: avtPuReMDFileFormat
//
//  Purpose:
//      Reads trajectory files from the PuReMD code.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 16, 2010
//
// ****************************************************************************

class avtPuReMDFileFormat : public avtMTSDFileFormat
{
  public:
                       avtPuReMDFileFormat(const char *);
    virtual           ~avtPuReMDFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "PuReMD"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual bool          HasInvariantMetaData(void) const { return false; };

  protected:
    enum AtomFormat
    {
        AF_NONE,
        AF_POS_Q,
        AF_POS_FORCE_Q,
        AF_POS_VEL_Q,
        AF_POS_VEL_FORCE_Q
    };

    struct UCV
    {
        double v[3][3];
        double *operator[](int i) { return v[i]; }
        UCV()
        {
            v[0][0] = 1; v[0][1] = 0; v[0][2] = 0;
            v[1][0] = 0; v[1][1] = 1; v[1][2] = 0;
            v[2][0] = 0; v[2][1] = 0; v[2][2] = 1;
        }
    };

    ifstream                       in;
    std::string                    filename;

    std::vector<int>               cycles;
    std::vector<double>            times;
    std::vector<int>               nBonds;
    std::vector<UCV>               unitCell;
    std::vector<istream::pos_type> filePositions;
    bool                           metaDataRead;
    int                            nTimeSteps;
    int                            nAtoms;
    AtomFormat                     atomFormat;

    std::vector<int>               atomElement;
    std::vector<int>               atomSpecies;
    std::vector<double>            atomWeight;

    std::set<int>                  existingElements;

    int                            currentTimestep;
    std::vector<float>             x;
    std::vector<float>             y;
    std::vector<float>             z;
    std::vector<float>             vx;
    std::vector<float>             vy;
    std::vector<float>             vz;
    std::vector<float>             fx;
    std::vector<float>             fy;
    std::vector<float>             fz;
    std::vector<float>             q;
    std::vector<std::pair<int,int> > bonds;

    virtual void    PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    void            OpenFileAtBeginning();
    void            ReadTimeStep(int);
    void            ReadAllMetaData();

    virtual void           GetCycles(std::vector<int>&);
    virtual void           GetTimes(std::vector<double>&);
};


#endif
