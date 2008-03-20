/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                            avtXDMFFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_XDMF_FILE_FORMAT_H
#define AVT_XDMF_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <XDMFParser.h>

#include <hdf5.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

class vtkDataObject;

class DataItem
{
  public:
                       DataItem() {cdata = NULL;};
    virtual           ~DataItem() {if (cdata != NULL) delete [] cdata;};

    // Enums for the class.
    enum               Type {DATA_TYPE_CHAR, DATA_TYPE_UCHAR,
                             DATA_TYPE_INT, DATA_TYPE_UINT,
                             DATA_TYPE_FLOAT};
    enum               Format {DATA_FORMAT_XML, DATA_FORMAT_HDF};

    // Data elements for the class.
    Type               type;
    int                precision;
    Format             format;
    int                nDims;
    int                dims[4];
    char              *cdata;
    int                cdataOffset;
    int                cdataLength;
};

class MeshInfo
{
  public:
                       MeshInfo()
                       {
                           extents = NULL;
                           ghostOffsets = NULL;
                           for (int i = 0; i < 3; i++)
                               meshData[i] = NULL;
                       };
    virtual           ~MeshInfo()
                      {
                          if (extents != NULL) delete [] extents;
                          if (ghostOffsets != NULL) delete [] ghostOffsets;
                          for (int i = 0; i < 3; i++)
                              if (meshData[i] != NULL) delete meshData[i];
                          for (unsigned int i = 0; i < blockList.size(); i++)
                              if (blockList[i] != NULL) delete blockList[i];
                      };

    // Enums for the struct.
    enum               GeometryType {TYPE_XYZ, TYPE_XY, TYPE_X_Y_Z,
                                     TYPE_VXVYVZ, TYPE_ORIGIN_DXDYDZ};

    // Data elements for the struct.
    string             name;
    avtMeshType        type;
    int                iDomain;

    // For Multimeshes.
    int                nBlocks;
    vector<MeshInfo*>  blockList;

    // For normal meshes.
    GeometryType       geometryType;
    int                topologicalDimension;
    int                spatialDimension;
    int                dimensions[3];
    double            *extents;
    int               *ghostOffsets;
    DataItem          *meshData[3];
};

class VarInfo
{
  public:
                       VarInfo()
                       {
                           extents = NULL;
                           varData = NULL;
                       };
    virtual           ~VarInfo()
                       {
                           if (extents != NULL) delete [] extents;
                           if (varData != NULL) delete varData;
                           for (unsigned int i = 0; i < blockList.size(); i++)
                               if (blockList[i] != NULL) delete blockList[i];
                       };

    // Enums for the class.
    enum               VariableType {TYPE_SCALAR, TYPE_VECTOR, TYPE_TENSOR};

    // Data elements for the class.
    string             name;
    string             meshName;
    VariableType       variableType;
    avtCentering       centering;
    int                varDimension;
    int                iDomain;
    int                iGrid;

    // For Multivars.
    int                nBlocks;
    vector<VarInfo*>   blockList;

    // For normal variables.
    double            *extents;
    DataItem          *varData;
};

class DomainInfo
{
  public:
                       DomainInfo() {};
    virtual           ~DomainInfo() {};

    string             name;
    int                nGrids;
};

// ****************************************************************************
//  Class: avtXDMFFileFormat
//
//  Purpose:
//      Reads in XDMF files as a plugin to VisIt.
//
//  Programmer: brugger -- generated by xml2avt
//  Creation:   Wed Nov 14 11:28:35 PDT 2007
//
//  Modifications:
//    Eric Brugger, Mon Mar 17 13:20:14 PDT 2008
//    Added ParseGridInformation and GetStructuredGhostZones.
//
//    Eric Brugger, Wed Mar 19 12:37:11 PDT 2008
//    Added OpenFile and CloseFile.
//
// ****************************************************************************

class avtXDMFFileFormat : public avtSTMDFileFormat
{
  public:
                       avtXDMFFileFormat(const char *);
    virtual           ~avtXDMFFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int domain,
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    //
    // virtual int         GetCyle(void);
    //

    void                   ActivateTimestep(void);

    virtual const char    *GetType(void)   { return "XDMF"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    string                 fname;
    XDMFParser             xdmfParser;

    bool                   fileRead;

    vector<MeshInfo*>      fileMeshList;
    vector<VarInfo*>       fileVarList;

    vector<DomainInfo*>    fileDomainList;

    char                   filename[1024], datasetname[1024];

    hid_t                 *hdfFiles;

    hid_t                  OpenFile(int);
    hid_t                  OpenFile(const char *);
    virtual void           CloseFile(int);

    int                    DetermineFileAndDataset(const char *, char *,
                               char *) const;
    int                    ReadHDFDataItem(DataItem *, void *, int);
    void                   AddVarInfo(bool, int, VarInfo *);
    DataItem              *ParseDataItem();
    void                   ParseTopology(string &, string &);
    void                   ParseGeometry(string &, int &, DataItem **);
    VarInfo               *ParseAttribute(int, int, const string &);
    void                   ParseGridInformation(string &);
    void                   ParseUniformGrid(vector<MeshInfo*> &, int,
                               int, bool, const string &);
    void                   ParseGrid(vector<MeshInfo*> &, int, int,
                               string &, bool);
    void                   ParseDomain(int);
    void                   ParseXdmf();
    void                   ParseXMLFile(void);

    void                   GetStructuredGhostZones(MeshInfo *, vtkDataSet *ds);
    vtkDataSet            *GetCurvilinearMesh(MeshInfo *);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
