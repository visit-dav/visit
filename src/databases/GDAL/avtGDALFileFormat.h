
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
