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
//                          avtCosmosFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_COSMOS_FILE_FORMAT_H
#define AVT_COSMOS_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>

class vtkDataSet;

// ****************************************************************************
//  Class: avtCosmosFileFormat
//
//  Purpose:
//      A file format reader for the Cosmos file format.
//
//  Programmer:  Akira Hadodx
//  Creation:    June 4, 2003
//
//  Modifications:
//
//    Akira Haddox, Fri Jun 13 13:50:08 PDT 2003
//    Added data and functions to read and store the compact mesh
//    structure information.
//
//    Akira Haddox, Mon Jun 16 12:33:50 PDT 2003
//    Added in 2D spherical, cylindrical, XZ and YZ cartesian support.
//    Added in support for reading in ghostzones.
//
//    Akira Haddox, Tue Jul 22 09:43:43 PDT 2003
//    Fixed some code to compile on IRIX. Separated time reading code
//    to separate function.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Removed GetCycles as it was implementing the same functionality as
//    the default method in the base class. Added timeState arg to 
//    PopulateDatabaseMetaData to satisfy new interface
// ****************************************************************************

class avtCosmosFileFormat : public avtMTMDFileFormat
{
  public:
                          avtCosmosFileFormat(const char *);
    virtual              ~avtCosmosFileFormat();
    
    virtual const char   *GetType(void) { return "Cosmos File Format"; };
    
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

  protected:
    enum CoordinateType
    {   cartesian, spherical, cylindrical   };

    bool                  readInTimes;
    void                  ReadInTimes();

    void                  ReadMesh(int domain);
    void                  ReadMeshInfo(int domain);

    vtkDataSet * CalculateMesh3DCartesian(double start[3],
                                          double delta[3], int ptDim[3]);
    vtkDataSet * CalculateMesh3DSpherical(double start[3],
                                          double delta[3], int ptDim[3]);

    vtkDataSet * CalculateMesh2DCartesian(double start[3],
                                          double delta[3], int ptDim[3]);
    vtkDataSet * CalculateMesh2DSpherical(double start[3],
                                          double delta[3], int ptDim[3]);
    vtkDataSet * CalculateMesh2DCylindrical(double start[3],
                                            double delta[3], int ptDim[3]);

    void         FillVectorVar3DCartesian(float *ptr, float *values[3], int);
    void         FillVectorVar3DSpherical(float *ptr, float *values[3], 
                                          double start[3], double delta[3],
                                          int dims[3]);
    
    void         FillVectorVar2DCartesian(float *ptr, float *values[3], int);
    void         FillVectorVar2DSpherical(float *ptr, float *values[3], 
                                          double start[3], double delta[3]);
    void         FillVectorVar2DCylindrical(float *ptr, float *values[3], 
                                          double start[3], double delta[3]);
    
    vtkDataSet                        **meshes;
    
    std::string                         dirname;
    
    int                                 rank;
    int                                 ndomains;
    int                                 ntimesteps;
    int                                 nscalars;
    int                                 nvectors;
    int                                 dimensions[3];
    int                                 dropDimension;
    CoordinateType                      coordType;

    //
    // Index by domain
    //  Elements are start x,y,z   delta x,y,z
    // 
    std::vector<std::vector<double> >   meshInfo;
    std::vector<bool>                   meshInfoLoaded;

    std::string                         timeFileName;
    
    //
    // Ghost zones are set on certain internal faces. Ordering is by
    // domain, then by start(x), end(x), start(y), end(y), start(z), end(z).
    //
    bool                                 existGhostZones;
    std::vector<std::vector<bool> >           ghostSet;
    
    std::vector<std::string>            scalarVarNames;
    std::vector<std::string>            vectorVarNames;


    std::vector<std::string>            gridFileNames;

    //
    // Following matrixes indexed: [domain][varId]
    //
    
    std::vector<std::vector<std::string> >      scalarFileNames;
    struct TripleString
    {
        std::string x,y,z;
    };
    std::vector<std::vector<TripleString> >     vectorFileNames;

    void ReadString(ifstream &ifile, std::string &str);
};

#endif
