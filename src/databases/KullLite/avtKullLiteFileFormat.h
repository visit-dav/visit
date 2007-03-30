// ************************************************************************* //
//                         avtKullLiteFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_KULL_LITE_FILE_FORMAT_H
#define AVT_KULL_LITE_FILE_FORMAT_H

#include <string>

#include <KullFormatStructures.h>

#include <avtSTMDFileFormat.h>

#include <database_exports.h>
#include <void_ref_ptr.h>

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
//  Modifications:
//
//    Akira Haddox, Tue May 20 08:49:59 PDT 2003
//    Added in for dealing with mixed materials, removed code
//    that tried to deal with data variables.
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

    virtual void *        GetAuxiliaryData(const char *var, int domain,
                                           const char *type, void *,
                                           DestructorFunction &df);

  protected:
    // Datasets stored by domain
    vtkDataSet          **dataset;
    
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
    pdb_mesh *m_kullmesh;
    pdb_taglist *m_tags;
   
    inline int ReadNumberRecvZones();
    
    static const char    *MESHNAME;

    void          ReadInFile(int);
    void          ReadInMaterialNames();
    void          ReadInMaterialName(int);

    void          Close();
};


#endif

