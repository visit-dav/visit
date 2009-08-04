// reconstructs material interfaces from a mesh containing mixed cells (where several materials are mixed)
// this implementation is based on the youngs algorithm, generalized to arbitrary cell types and works
// on both 2D and 3D meshes. the main advantage of the youngs algorithm is it guarantees the material volume correctness.
// for 2D meshes, the AxisSymetric flag allows to switch between a pure 2D (plannar) algorithm and an axis symetric 2D algorithm
// handling volumes of revolution.

// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France

#ifndef VTK_YOUNGS_MATERIAL_INTERFACE__C
#define VTK_YOUNGS_MATERIAL_INTERFACE__C

#include <vtkObjectFactory.h>
#include <vtkSetGet.h>
#include <vtkDataSet.h>

//BTX
#include <list>
#include <vector>
#include <string>
//ETX

/*!
\class vtkYoungsMaterialInterface
Detailed description of class vtkYoungsMaterialInterface ...
*/

class vtkYoungsMaterialInterface
{
  public:
    vtkYoungsMaterialInterface ();
    ~vtkYoungsMaterialInterface ();

    virtual int Execute(vtkDataSet *in,
                        vtkDataSet **out);

    void SetInverseNormal(bool v) { InverseNormal = v; }
    bool GetInverseNormal() { return InverseNormal; }

    void SetReverseMaterialOrder(bool v) { ReverseMaterialOrder = v; }
    bool GetReverseMaterialOrder() { return ReverseMaterialOrder; }

    void SetOnionPeel(bool v) { OnionPeel = v; }
    bool GetOnionPeel() { return OnionPeel; }

    void SetAxisSymetric(bool v) { AxisSymetric = v; }
    bool GetAxisSymetric() { return AxisSymetric; }

    void SetFillMaterial(bool v) { FillMaterial = v; }
    bool GetFillMaterial() { return FillMaterial; }

    void SetTwoMaterialsOptimization(bool v) { TwoMaterialsOptimization = v; }
    bool GetTwoMaterialsOptimization() { return TwoMaterialsOptimization; }

    void SetUseFractionAsDistance(bool v) { UseFractionAsDistance = v; }
    bool GetUseFractionAsDistance() { return UseFractionAsDistance; }

    void SetVolumeFractionRange(double *v)
    {
        VolumeFractionRange[0] = v[0];
        VolumeFractionRange[1] = v[1];
    }
    double *GetVolumeFractionRange() { return VolumeFractionRange; }

    virtual void SetNumberOfMaterials(int n);
    virtual int GetNumberOfMaterials();

    // Description:
    // Add a material by its description.  parameters are names of cell arrays used to compute interfaces.
    virtual void AddMaterial( const char* volume,
                              const char* normal,
                              const char* ordering );
    /*virtual void AddMaterial( const char* volume,
                              const char* normalX,
                              const char* normalY,
                              const char* normalZ,
                              const char* ordering );*/

    // Description:
    // Removes all meterials previously added.
    virtual void RemoveAllMaterials();

    enum
        {
            MAX_CELL_POINTS=256
        };

  protected:
    int CellProduceInterface(int dim, int np, double fraction, double minFrac, double maxFrac);

    int FillMaterial;
    int InverseNormal;
    int AxisSymetric;
    int OnionPeel;
    int ReverseMaterialOrder;
    int UseFractionAsDistance;
    int TwoMaterialsOptimization;
    double VolumeFractionRange[2];

    struct MaterialDescription
    {
        std::string volume, normal, normalX, normalY, normalZ, ordering;
    };
    std::vector<MaterialDescription> Materials;

};

#endif /* VTK_YOUNGS_MATERIAL_INTERFACE__C */
