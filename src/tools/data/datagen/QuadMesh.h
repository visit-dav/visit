// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QUADMESH_H
#define QUADMESH_H
#include <vector>
#include <string>

#include <silo.h>

// ****************************************************************************
// Class: QuadMesh
//
// Purpose:
//   Abstract base class for  quad meshes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:59 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Oct 4 14:25:40 PST 2002
//   I added methods to create the gradient of a scalar field.
//
//   Eric Brugger, Tue Mar 25 15:56:59 PST 2003
//   I corrected an out of range index error in ZonalGradientAt.
//
//   Hank Childs, Mon Dec  1 09:24:23 PST 2003
//   Added support for tensors.
//
//   Hank Childs, Thu Jul 21 16:49:02 PDT 2005
//   Added support for array variables.
//
//   Brad Whitlock, Tue Mar 10 13:12:12 PST 2009
//   I moved the internal classes into the implementation so we can focus
//   on the interface that matters.
//
//   Eric Brugger, Wed May 18 13:37:22 PDT 2016
//   I added support for adding time and cycle.
//
// ****************************************************************************

class QuadMesh
{
protected:
    class ScalarData;
    class VectorData;
    class TensorData;
    class SliceInfo;
    friend class SliceInfo;
    class MaterialList;
public:
    QuadMesh(int nx, int ny, int nz, bool rect = true);
    virtual ~QuadMesh();

    void SetMeshName(const std::string &name);

    int XDim() const;
    int YDim() const;
    int ZDim() const;

    void SetMeshTime(float time);
    void SetMeshCycle(int cycle);

    void SetMeshLabels(const std::string &labelX, 
                       const std::string &labelY, 
                       const std::string &labelZ); 

    void SetMeshUnits(const std::string &unitsX, 
                      const std::string &unitsY, 
                      const std::string &unitsZ);

    virtual float GetX(int, int, int) const = 0;
    virtual float GetY(int, int, int) const = 0;
    virtual float GetZ(int, int, int) const = 0;

    void CreateZonalData(const std::string &name, 
                         float (*zonal)(int,int,int, QuadMesh *),
                         const std::string &units = std::string());

    void CreateNodalData(const std::string &name,
                         float (*nodal)(float *, QuadMesh *),
                         const std::string &units = std::string());

    void CreateNodalVectorData(const std::string &name,
                               void (*nodal)(float *, int, int, int, QuadMesh *),
                               const std::string &units = std::string());

    void CreateGradient(const std::string &name,
                        const std::string &gradName,
                        const std::string &units = std::string());

    void CreateGradientTensor(const std::string &varName, 
                              const std::string &outputName);

    void AddMaterial(const std::string &matname);
    void CreateMaterialData(void (*createmat)(int,int,int, int *, double*, int *, QuadMesh *));

    void AddSlice(const std::string &varName, const std::string &nmn, const std::string &nvn,
        int sliceVal, int sliceDimension);

    void WriteFile(DBfile *db);

protected:
    virtual void WriteMesh(DBfile *db) = 0;
    virtual void WriteMeshSlice(DBfile *db, const std::string &, int, int) = 0;

    DBoptlist *MakeOptionList() const;

    int xdim;
    int ydim;
    int zdim;
    float *coordX;
    float *coordY;
    float *coordZ;
    std::vector<ScalarData *> scalarData;
    std::vector<VectorData *> vectorData;
    std::vector<TensorData *> tensorData;
    std::vector<SliceInfo *>  sliceInfo;
    MaterialList     *mats;
    bool              writeMaterial;
    float             meshTime;
    int               meshCycle;
    std::string       meshName;
    std::string       meshLabelX, meshLabelY, meshLabelZ;
    std::string       meshUnitsX, meshUnitsY, meshUnitsZ;
};

// ****************************************************************************
// Class: RectilinearMesh
//
// Purpose:
//   Represents a  rectilinear mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:54:28 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 14 09:33:46 PDT 2003
//   I added the WriteMeshSlice method.
//
// ****************************************************************************

class RectilinearMesh : public QuadMesh
{
public:
    RectilinearMesh(int nx, int ny, int nz);
    virtual ~RectilinearMesh();

    virtual float GetX(int x, int, int) const
    {
        return coordX[x];
    }

    virtual float GetY(int, int y, int) const
    {
        return coordY[y];
    }

    virtual float GetZ(int, int, int z) const
    {
        return coordZ[z];
    }

    void SetX(int x, float val)
    {
        coordX[x] = val;
    }

    void SetY(int y,float val)
    {
        coordY[y] = val;
    }

    void SetZ(int z, float val)
    {
        coordZ[z] = val;
    }

    void SetXValues(float minX, float maxX)
    {
        SetRange(coordX, minX, maxX, xdim);
    }

    void SetYValues(float minY, float maxY)
    {
        SetRange(coordY, minY, maxY, ydim);
    }

    void SetZValues(float minZ, float maxZ)
    {
        SetRange(coordZ, minZ, maxZ, zdim);
    }

protected:
    virtual void WriteMesh(DBfile *db);
    virtual void WriteMeshSlice(DBfile *db, const std::string &newMeshName,
         int sliceVal, int sliceDimension);
    void SetRange(float *coord, float minval, float maxval, int steps);
};

// ****************************************************************************
// Class: CurvilinearMesh
//
// Purpose:
//   Represents a  curvilinear mesh
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:55:27 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 14 09:33:32 PDT 2003
//   I added the WriteMeshSlice method.
//
// ****************************************************************************

class CurvilinearMesh : public QuadMesh
{
public:
    CurvilinearMesh(int X, int Y, int Z);
    virtual ~CurvilinearMesh();

    virtual float GetX(int x, int y, int z) const
    {
        return coordX[z*(ydim*xdim) + y*xdim + x];
    }

    virtual float GetY(int x, int y, int z) const
    {
        return coordY[z*(ydim*xdim) + y*xdim + x];
    }

    virtual float GetZ(int x, int y, int z) const
    {
        return coordZ[z*(ydim*xdim) + y*xdim + x];
    }

    void SetX(int x, int y, int z, float val)
    {
        coordX[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void SetY(int x, int y, int z, float val)
    {
        coordY[z*(ydim*xdim) + y*xdim + x] = val;
    }

    void SetZ(int x, int y, int z, float val)
    {
        coordZ[z*(ydim*xdim) + y*xdim + x] = val;
    }

private:
    virtual void WriteMesh(DBfile *db);
    virtual void WriteMeshSlice(DBfile *db, const std::string &newMeshName,
         int sliceVal, int sliceDimension);
};

#endif
