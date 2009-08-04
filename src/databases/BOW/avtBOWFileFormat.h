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
//                            avtBOWFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_WPP_FILE_FORMAT_H
#define AVT_WPP_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vectortypes.h>


// ****************************************************************************
//  Class: avtBOWFileFormat
//
//  Purpose:
//      Reads in BOW (brick of wavelet) files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
// ****************************************************************************

class avtBOWFileFormat : public avtMTMDFileFormat
{
public:
                       avtBOWFileFormat(const char *);
    virtual           ~avtBOWFileFormat();

    virtual void           ActivateTimestep(int ts);

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);
    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "BOW files"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

protected:
    class HeaderData
    {
    public:
        HeaderData();
        ~HeaderData();
        int Read(const char *filename);

        int nx, ny, nz;
        int dx, dy, dz;
        int topdirmin;
        int topdirmod;
        int numblocks;
        double q;
        char *filenamepat;
        char varname[100];
        bool delogify;

        int stride[3];

        int   timeMethod;
        char  *m1_file;
        int    m2_nStates;
        int    m2_startCycle;
        int    m2_deltaCycle;
        double m2_startTime;
        double m2_deltaTime;
    };

    bool         headerRead;
    HeaderData   header;
    bool         cyclesAndTimesDetermined;
    intVector    cycles;
    doubleVector times;
    bool         domainConnectivtyComputed;
    int         *domainConnectivity;

    bool DetermineCyclesAndTimes();
    void ReadDomainConnectivity();

    void GetFilenameForDomain(char *filename, int len, int dom, int ts);
    char *GetBOWBytesForDomainAtTime(int dom, int ts, bool infoOnly);
    bool ReadSingleBOFDimensions(int dom, int ts, int *dimsize);
    void ComputeStrideAdjustedSize(const int *domainSize, int *adjustedSize) const;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
