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
//                            avtEnzoFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Enzo_FILE_FORMAT_H
#define AVT_Enzo_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>


// ****************************************************************************
//  Class: avtEnzoFileFormat
//
//  Purpose:
//      Reads in Enzo files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 3, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 11 18:15:49 PST 2005
//    Added HDF5 support to the existing HDF4 support.
//
//    Jeremy Meredith, Fri Jul 15 15:27:49 PDT 2005
//    Added fixes for multi-timestep Enzo runs.
//
//    Jeremy Meredith, Wed Aug  3 10:21:56 PDT 2005
//    Added support for 2D Enzo files.
//
//    Jeremy Meredith, Thu Aug 11 14:35:39 PDT 2005
//    Added new routine to unify global extents.
//
//    Jeremy Meredith, Mon Apr  6 14:36:58 EDT 2009
//    Added support for particle and grid file names.
//    This is for support for the new "Packed AMR" format.
//
// ****************************************************************************

class avtEnzoFileFormat : public avtSTMDFileFormat
{
  public:
                           avtEnzoFileFormat(const char *);
    virtual               ~avtEnzoFileFormat();

    virtual bool           HasInvariantMetaData(void) const { return false; };
    virtual bool           HasInvariantSIL(void) const { return false; };

    virtual int            GetCycle(void);

    virtual const char    *GetType(void)   { return "Enzo"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual void          *GetAuxiliaryData(const char *var, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    void                   ActivateTimestep(void);
    virtual int            GetCycleFromFilename(const char *f) const;
    
  protected:
    enum FileType { ENZO_FT_UNKNOWN, ENZO_FT_HDF4, ENZO_FT_HDF5 };
    FileType fileType;

    // DATA MEMBERS
    struct Grid
    {
        int              ID;
        std::vector<int> childrenID;
        int              parentID;
        int              level;

        int              dimension;

        int              numberOfParticles;

        double           minSpatialExtents[3];
        double           maxSpatialExtents[3];

        int              zdims[3];
        int              ndims[3];
        int              minLogicalExtentsInParent[3];
        int              maxLogicalExtentsInParent[3];
        int              minLogicalExtentsGlobally[3];
        int              maxLogicalExtentsGlobally[3];
        double           refinementRatio[3];

        std::string      gridFileName;
        std::string      particleFileName;

      public:
        void PrintRecursive(std::vector<Grid> &grids, int level = 0);
        void Print();
        void DetermineExtentsInParent(std::vector<Grid> &grids);
        void DetermineExtentsGlobally(int numLevels,std::vector<Grid> &grids);
    };

    std::string fname_dir;
    std::string fname_base;
    std::string fnameB;
    std::string fnameH;

    int dimension;

    std::vector<Grid> grids;
    int numGrids;
    int numLevels;

    int curCycle;
    double curTime;

    std::vector<std::string> varNames;
    std::vector<std::string> particleVarNames;
    std::vector<std::string> tracerparticleVarNames;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    void ReadAllMetaData();
    void ReadHierachyFile();
    void ReadParameterFile();
    void UnifyGlobalExtents();
    void DetermineVariablesFromGridFile();
    void BuildDomainNesting();
};


#endif
