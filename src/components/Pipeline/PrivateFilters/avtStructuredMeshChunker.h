// ************************************************************************* //
//                       avtStructuredMeshChunker.h                          //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_MESH_CHUNKER_H
#define AVT_STRUCTURED_MESH_CHUNKER_H

#include <pipeline_exports.h>

class vtkCellData;
class vtkDataArray;
class vtkDataSet;
class vtkPointData;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

#include <vector>

#include <avtGhostData.h>


// ****************************************************************************
//  Class: avtStructuredMeshChunker
//
//  Purpose:
//      This will take a structured mesh (vtkRectilinearGrid or 
//      vtkStructuredGrid) and an array declaring the designation of each zone
//      in the mesh.  Each zone either should be "retained", "discarded",
//      or "to be processed".  For example, with an isovolume operation, many
//      of the zones fall within the acceptable range.  They are "retained".
//      Others are fully outside the acceptable range.  They are "discarded".
//      Some zones are partly in the range and partly out of the range.  They
//      require further processing ("to be processed"), which is not done by
//      this module.
//
//      This module will break a structured mesh into many smaller structured
//      meshes and one unstructured mesh.  The unstructured mesh is of type
//      "to be processed" plus those zones that could not be easily put into
//      the other structured meshes.  Ghost zones are created for all of the
//      meshes so that there are no internal faces.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2004
//
// ****************************************************************************

class PIPELINE_API avtStructuredMeshChunker
{
  public:
    typedef enum
    {
        RETAIN,
        DISCARD,
        TO_BE_PROCESSED,
        PUT_IN_GRID // FOR INTERNAL USES
    } ZoneDesignation;

    static void ChunkStructuredMesh(vtkDataSet *,
                                    std::vector<ZoneDesignation> &,
                                    std::vector<vtkDataSet *> &,
                                    vtkUnstructuredGrid *&,
                                    avtGhostDataType, bool);

    static void ChunkCurvilinearMesh(vtkStructuredGrid *,
                                     std::vector<ZoneDesignation> &,
                                     std::vector<vtkDataSet *> &,
                                     vtkUnstructuredGrid *&,
                                     avtGhostDataType, bool);
    static void ChunkRectilinearMesh(vtkRectilinearGrid *,
                                     std::vector<ZoneDesignation> &,
                                     std::vector<vtkDataSet *> &,
                                     vtkUnstructuredGrid *&,
                                     avtGhostDataType, bool);

  protected:
    class MeshDescription
    {
      public:
        MeshDescription();
        int start_index[3];
        int index_size[3];
        bool lowIIsGhost;
        bool hiIIsGhost;
        bool lowJIsGhost;
        bool hiJIsGhost;
        bool lowKIsGhost;
        bool hiKIsGhost;
        int  NumPoints(void) { return index_size[0]*index_size[1]*
                                      index_size[2]; };
        int  NumCells(void)  { int nI = (index_size[0] > 1 
                                            ? index_size[0]-1 
                                            : 1);
                               int nJ = (index_size[1] > 1 
                                            ? index_size[1]-1 
                                            : 1);
                               int nK = (index_size[2] > 1 
                                            ? index_size[2]-1 
                                            : 1);
                               return nI*nJ*nK; };
    };

    static ZoneDesignation *SplitIntoSubgrids(const int *dims, 
                                              vtkDataSet *in_ds,
                                              std::vector<ZoneDesignation> &,
                                              avtGhostDataType,
                                              std::vector<MeshDescription> 
                                                                     &outGrids, 
                                              vtkUnstructuredGrid *&, bool);
    static void CreateUnstructuredGrid(vtkDataSet *in_ds, 
                                       ZoneDesignation *, avtGhostDataType,
                                       vtkUnstructuredGrid *&,const int *dims);

    static void CreateGhostData(MeshDescription &, const int *, 
                                avtGhostDataType, ZoneDesignation *,
                                vtkPointData *, vtkCellData *);
    static void ModifyGridsForGhostZones(std::vector<MeshDescription> &, 
                                         const int *, ZoneDesignation *);
    static void GetUnstructuredCellList(ZoneDesignation *,
                                        const int *, avtGhostDataType ,
                                        std::vector<int> &,std::vector<int> &);
    static bool NodeIsGhost(int, int, int, const int *, ZoneDesignation*,bool);
};


#endif


