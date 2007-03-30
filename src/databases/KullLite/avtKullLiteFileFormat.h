// ************************************************************************* //
//                         avtKullLiteFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_KULL_LITE_FILE_FORMAT_H
#define AVT_KULL_LITE_FILE_FORMAT_H

#include <string>
#include <map>

//#include <pdb.h>
#include <KullFormatStructures.h>

#include <avtSTMDFileFormat.h>

#include <database_exports.h>


struct s_PDBfile;
typedef struct s_PDBfile PDBfile;
class vtkDataSet;
class vtkDataArray;


// ****************************************************************************
//  Class: avtKullLiteFileFormat
//
//  Purpose:
//      Handles files of the pdb file format.
//
//  Programmer: Akira Haddox
//  Creation:   June 18, 2002
//
// ****************************************************************************

class DATABASE_API avtKullLiteFileFormat : public avtSTMDFileFormat
{
  public:
                          avtKullLiteFileFormat(const char *);
    virtual              ~avtKullLiteFileFormat();

    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual const char   *GetType(void)  { return "KullLite File Format"; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

    virtual bool          PerformsMaterialSelection() {    return true; }

  protected:
// Datasets stored by domain
    vtkDataSet          **dataset;
// Datasets also stored by domain * tag/material name.
    std::vector<std::map<std::string, vtkDataSet *> > materials;
    std::vector<std::string>       m_names;
    std::vector<std::string>       tag_names;
    std::vector<int>            tag_dim;
    
// m_names is loaded at construction, so this will always be right
    inline int NumberOfMaterials()  { return m_names.size(); }

    PDBfile *m_pdbFile;
    pdb_mesh *m_kullmesh;
    pdb_taglist *m_tags;
    
    static const char    *MESHNAME;

    void          ReadInFile(int);
    void          ReadInAllFiles();
    void          ReadInMaterialNames();
    void          ReadInMaterialName(int);

    void          Close();
};


#endif


