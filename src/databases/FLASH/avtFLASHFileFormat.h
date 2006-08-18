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
//                            avtFLASHFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_FLASH_FILE_FORMAT_H
#define AVT_FLASH_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <map>
#include <string>
#include <vector>

#include <hdf5.h>


// ****************************************************************************
//  Class: avtFLASHFileFormat
//
//  Purpose:
//      Reads in FLASH files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 23, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 25 15:07:36 PDT 2005
//    Added particle support.
//
//    Jeremy Meredith, Tue Sep 27 14:22:29 PDT 2005
//    Added "new" style particle support.
//
//    Hank Childs, Fri Apr 28 14:20:35 PDT 2006
//    Added activate timestep.
//
//    Kathleen Bonnell, Thu Jul 20 11:22:13 PDT 2006
//    Added methods and structs to support for FLASH3 formats.
//
// ****************************************************************************

class avtFLASHFileFormat : public avtSTMDFileFormat
{
  public:
                       avtFLASHFileFormat(const char *);
    virtual           ~avtFLASHFileFormat() {;};

    virtual bool           HasInvariantMetaData(void) const { return false; };
    virtual bool           HasInvariantSIL(void) const { return false; };

    virtual const char    *GetType(void)   { return "FLASH"; };
    virtual void           FreeUpResources(void); 
    virtual void           ActivateTimestep(void);

    virtual void          *GetAuxiliaryData(const char *var, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    void ReadAllMetaData();

    void BuildDomainNesting();

    void ReadSimulationParameters();
    void ReadBlockStructure();
    void ReadBlockExtents();
    void ReadRefinementLevels();
    void ReadUnknownNames();
    void DetermineGlobalLogicalExtentsForAllBlocks();
    void ReadParticleAttributes();

    // FLASH3 support
    void ReadParticleAttributes_FLASH3();
    void ReadVersionInfo();
    void ReadIntegerScalars();
    void ReadRealScalars();
    void ReadParticleVar(hid_t pointId, const char *, double *);

  protected:
    struct SimParams
    {
        int total_blocks;
        double time;
        double timestep;
        double redshift;
        int nsteps;
        int nxb;
        int nyb;
        int nzb;
    };

    struct Block
    {
        int ID;
        int level;
        int parentID;
        int childrenIDs[8];
        int neighborIDs[6];
        double minSpatialExtents[3];
        double maxSpatialExtents[3];
        int    minGlobalLogicalExtents[3];
        int    maxGlobalLogicalExtents[3];

        void Print(ostream&);
    };

    // FLASH3 support
    struct IntegerScalars
    {
        char name[20];
        int value;
    };
    
    struct RealScalars
    {
        char name[20];
        double value;
    }; 

  protected:
    std::string               filename;
    int                       dimension;
    int                       numBlocks;
    int                       numLevels;
    int                       numParticles;
    int                       fileFormatVersion;
    std::string               particleHDFVarName;
    hid_t                     fileId;
    SimParams                 simParams;
    std::vector<Block>        blocks;
    int                       numChildrenPerBlock;
    int                       numNeighborsPerBlock;
    int                       block_ndims[3];
    int                       block_zdims[3];
    double                    minSpatialExtents[3];
    double                    maxSpatialExtents[3];
    std::vector<std::string>  varNames;
    std::vector<std::string>  particleVarNames;
    std::vector<hid_t>        particleVarTypes;
    std::map<std::string,int> particleOriginalIndexMap;
};


#endif
