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
//                            avtGDALFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_GDAL_FILE_FORMAT_H
#define AVT_GDAL_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <string>
#include <map>
#include <gdal_priv.h>

class vtkFloatArray;
class vtkUnsignedCharArray;

// ****************************************************************************
// Class: avtGDALFileFormat
//
// Purpose:
//     Reads GIS files into VisIt using the GDAL library.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 6 11:54:59 PDT 2005
//
// Modifications:
//
// ****************************************************************************

class avtGDALFileFormat : public avtSTMDFileFormat
{
public:
    avtGDALFileFormat(const char *filename);
    virtual ~avtGDALFileFormat();

    virtual const char    *GetType(void)   { return "GDAL"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    static const int n_zones_per_dom;

    struct MeshInfo
    {
        int  xdims;
        int  ydims;
        int  nYPerDomain;
        int  scale;
        int  numDomains;
        bool hasZComponent;
    };

    typedef std::map<std::string, MeshInfo> MeshInfoMap;

    static bool    gdalInit;

    MeshInfoMap    meshInfo;
    GDALDataset   *poDataset;
    int            xdims;
    int            ydims;
    float          xmin;
    float          xmax;
    float          ymin;
    float          ymax;
    bool           invertYValues;

    virtual void   PopulateDatabaseMetaData(avtDatabaseMetaData *);

    GDALDataset   *GetDataset();
    vtkFloatArray *ReadVar(const MeshInfo &info, GDALRasterBand *poBand,
                           int domain);
    void           CalculateNDomains(int xsize, int ysize, int &nY, int &ndoms);
    void           CreateCoordinates(const MeshInfo &info,
                                     int domain, vtkFloatArray **coords, int);
    vtkDataSet    *CreateFlatMesh(const MeshInfo &info, int domain,
                                  const char *meshname);
    vtkDataSet    *CreateElevatedMesh(const MeshInfo &info, int domain,
                                      const char *meshname);
    std::string    GetComponentName(const char *name) const;
    vtkUnsignedCharArray *CreateGhostZonesArray(int nCellsInX, 
                                                int nRealCells, int domain,
                                                int numDomains);
};


#endif
