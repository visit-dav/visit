// ************************************************************************* //
//                             avtMiliFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MILI_FILE_FORMAT_H
#define AVT_MILI_FILE_FORMAT_H

#include <vector>
#include <string>
#include <fstream.h>

extern "C" {
#include <mili.h>
}

#include <avtMTSDFileFormat.h>
#include <avtTypes.h>

class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtMiliFileFormat
//
//  Purpose:
//      A file format reader for Mili.
//
//  Notes:       Much of the code was taken from Doug Speck's GRIZ reader.
//      
//  Programmer:  Hank Childs
//  Creation:    April  11, 2003
//
// ****************************************************************************

class avtMiliFileFormat : public avtMTSDFileFormat
{
  public:
                          avtMiliFileFormat(const char *);
    virtual              ~avtMiliFileFormat();
    
    virtual const char   *GetType(void) { return "Mili File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    Famid                 dbid;
    int                   ntimesteps;

    bool                  gottenGeneralInfo;
    int                   dims;
    int                   nnodes;
    int                   ncells;
    vtkUnstructuredGrid  *connectivity;

    std::vector< std::string >         sub_records;
    std::vector< int >                 sub_record_ids;

    std::vector< std::string >         element_group_name;
    std::vector< int >                 connectivity_offset;

    std::vector< std::string >         vars;
    std::vector< avtCentering >        centering;
    std::vector< std::vector<bool> >   vars_valid;
    std::vector< avtVarType >          vartype;

    void                  GetGeneralInfo(void);
    void                  ConstructMaterials(std::vector< std::vector<int*> >&,
                                            std::vector< std::vector<int> > &);
    int                   GetVariableIndex(const char *);
    void                  GetSizeInfoForGroup(const char *, int &, int &);
};


#endif


