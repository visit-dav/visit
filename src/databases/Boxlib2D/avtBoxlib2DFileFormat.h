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
//                         avtBoxlib2DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_BOXLIB_2D_FILE_FORMAT_H
#define AVT_BOXLIB_2D_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>


class VisMF;

// ****************************************************************************
//  Class: avtBoxlib2DFileFormat
//
//  Purpose:
//      A file format reader for single-timestep, 2D AMR Boxlib files.
//
//  Notes:       The original 2D reader was written by Akira Haddox.  Then
//               the 3D reader was greatly enhanced.  On Dec. 10th, 2003 the 3D
//               reader was used to make a new version of the 2D reader.
//               The origination of this 3D reader is from Akira Haddox, so he
//               should be considered the author of this reader as well.
//
//  Programmer:  Hank Childs (Akira Haddox)
//  Creation:    December 10, 2003
//
//  Modifications:
//
//    Hank Childs, Sun Mar  6 16:21:15 PST 2005
//    Add support for GeoDyne material names.
//
//    Hank Childs, Thu Jun 23 11:16:52 PDT 2005
//    Added ActivateTimestep.
//
//    Mark C. Miller, Wed Nov  9 12:35:15 PST 2005
//    Added GetCycleFromFilename
//
// ****************************************************************************

class avtBoxlib2DFileFormat : public avtSTMDFileFormat
{
  public:
                          avtBoxlib2DFileFormat(const char *);
    virtual              ~avtBoxlib2DFileFormat();
    
    virtual const char   *GetType(void) { return "Boxlib2D File Format"; };
    virtual bool          HasInvariantSIL(void) const { return false; };
    virtual bool          HasInvariantMetaData(void) const { return false; };
    
    virtual int           GetCycle(void) { return cycle; };
    virtual int           GetCycleFromFilename(const char *f) const;
 
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void         *GetAuxiliaryData(const char *var, int,
                                           const char *type, void *args,
                                           DestructorFunction &);
    
    virtual void          FreeUpResources(void);
    virtual void          ActivateTimestep(void);

  protected:
    // This relative location of the multifab files.  It contains entries for
    // all levels. There is no predefined length.
    std::vector<std::string>                multifabFilenames;

    // For each level and variable, which multifab file the variable
    // is stored in, and at what index (component in the file it is stored as.)
    // Indexed by level, variable.
    std::vector<std::vector<int> >          fabfileIndex;
    std::vector<std::vector<int> >          componentIds;

    std::string                             rootPath;

    int                                     nLevels;
    std::vector<int>                        patchesPerLevel;
    // These entries are per patch.
    std::vector<double>                     xMin;
    std::vector<double>                     xMax;
    std::vector<double>                     yMin;
    std::vector<double>                     yMax;
    // These entries are per level.
    std::vector<double>                     deltaX;
    std::vector<double>                     deltaY;
    // This entry is per level, but level 0 is omitted.
    std::vector<int>                        refinement_ratio;

    bool                                    haveReadTimeAndCycle;
    double                                  time;
    int                                     cycle;
    std::string                             timestepPath;
    bool                                    initializedReader;
    bool                                    vf_names_for_materials;
    
    // Scalar vars listed in header.
    int                                     nVars;
    std::vector<std::string>                varNames;
    std::vector<int>                        varCentering;
    std::vector<bool>                       varUsedElsewhere;

    // Vectors are combinations of scalar vars. The index to those
    // variables are stored in vectorComponents.
    int                                     nVectors;
    std::vector<std::string>                vectorNames;
    std::vector<std::vector<int> >          vectorComponents;
    std::vector<int>                        vectorCentering;

    // Variable readers.
    // Indexed by multifabFile
    std::vector<VisMF *>                    mfReaders;

    // The VisMF class is buggy when it comes to freeing up
    // memory.  The "clearlist" is used to sidestep those issues.
    std::vector<int>                        clearlist;

    // Problem range
    double                                  probLo[2];
    double                                  probHi[2];
    
    int                                     nMaterials;

    static const int                        dimension = 2;

    void                                   *GetMaterial(const char *, int,
                                                        const char *, 
                                                        DestructorFunction &);
    void                                   *GetSpatialIntervalTree(
                                                        DestructorFunction &);
    VisMF*                                  GetVisMF(int index);
    void                                    ReadHeader(void);
    void                                    InitializeReader(void);
    void                                    CalculateDomainNesting(void);

    vtkDataSet *CreateGrid(double lo[2], double hi[2], double delta[2]) const;

    int                 GetGlobalPatchNumber(int level, int patch) const;
    void                GetLevelAndLocalPatchNumber(int global_patch,
                                  int &level, int &local_patch) const;
    void                GetDimensions(int *, int level, int patch) const;
    void                GetDimensions(int *, double*, double*, double*) const;
};


#endif


