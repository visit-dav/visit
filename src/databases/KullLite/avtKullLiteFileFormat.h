// ************************************************************************* //
//                         avtKullLiteFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_KULL_LITE_FILE_FORMAT_H
#define AVT_KULL_LITE_FILE_FORMAT_H

#include <string>

#include <KullFormatStructures.h>

#include <avtSTMDFileFormat.h>

#include <void_ref_ptr.h>

struct s_PDBfile;
typedef struct s_PDBfile PDBfile;
class vtkDataSet;
class vtkDataArray;
class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtKullLiteFileFormat
//
//  Purpose:
//      Handles files of the pdb file format.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
//  Modifications:
//
//    Akira Haddox, Tue May 20 08:49:59 PDT 2003
//    Added in for dealing with mixed materials, removed code
//    that tried to deal with data variables.
//    
//    Hank Childs, Fri Jul 23 09:42:09 PDT 2004
//    Extended format to support 2D meshes.
//
//    Hank Childs, Mon Jul 26 08:59:40 PDT 2004
//    Add support for mesh tags.
//
//    Hank Childs, Tue Jun 14 16:31:33 PDT 2005
//    Add support for IsRZ.
//
//    Hank Childs, Thu May 11 08:52:27 PDT 2006
//    Add support for reading densities.
//
// ****************************************************************************

class avtKullLiteFileFormat : public avtSTMDFileFormat
{
  public:
                          avtKullLiteFileFormat(const char *);
    virtual              ~avtKullLiteFileFormat();

    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual const char   *GetType(void)  { return "KullLite File Format"; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

    virtual void *        GetAuxiliaryData(const char *var, int domain,
                                           const char *type, void *,
                                           DestructorFunction &df);

  protected:
    // Datasets stored by domain
    vtkUnstructuredGrid          **dataset;
    std::vector<std::string>       my_filenames;
    
    std::vector<std::string>       m_names;
    
    // m_names is loaded at construction, so this will always be right
    inline int NumberOfMaterials()  { return m_names.size(); }

    // Returns true if the string holds to the material string standard:
    // Must start with mat_, and must have one other underscore. 
    inline bool IsMaterialName(const std::string &str)
    {
        return (str[0] == 'm') && (str[1] == 'a') && (str[2] == 't')
               && (str[3] == '_') && (str.find_last_of('_') != 4);
    }

    // Precondition: IsMaterialName(str) is true
    // Returns the string contained between mat_ and the last underscore.
    // Eg: "pure_gold" in "mat_pure_gold_zones"
    inline std::string GetMaterialName(const std::string &str)
    {   return str.substr(4, str.find_last_of('_') - 4);    }

    PDBfile *m_pdbFile;
    pdb_mesh3d *m_kullmesh3d;
    pdb_mesh2d *m_kullmesh2d;
    pdb_taglist *m_tags;
   
    std::vector<std::string> zone_tags;
    std::vector<std::string> face_tags;
    std::vector<std::string> edge_tags;
    std::vector<std::string> node_tags;

    inline int ReadNumberRecvZones();
    
    static const char    *MESHNAME;

    void          ReadInPrimaryMesh(int);
    void          ReadInMaterialNames();
    void          ReadInMaterialName(int);
    bool          ReadMeshFromFile(void);
    bool          GetMeshDimension(void);
    bool          IsRZ(void);
    bool          ContainsDensities(void);

    bool          ClassifyAndAdd2DZone(pdb_mesh2d *, int, 
                                       vtkUnstructuredGrid *);
    bool          ClassifyAndAdd3DZone(pdb_mesh3d *, int, 
                                       vtkUnstructuredGrid *);

    vtkDataSet   *CreateMeshTags(const char *, int);
    void          CreateNodeMeshTags(vtkUnstructuredGrid *, pdb_taglist *);
    void          CreateEdgeMeshTags(vtkUnstructuredGrid *, pdb_taglist *,
                                     pdb_mesh2d *);
    void          CreateFaceMeshTags(vtkUnstructuredGrid *, pdb_taglist *,
                                     pdb_mesh3d *);
    void          CreateZoneMeshTags(vtkUnstructuredGrid *, pdb_taglist *,
                                     vtkDataSet *);

    void         *GetRealMaterial(int);
    void         *GetMeshTagMaterial(const char *, int);

    void          Close();
};


#endif

