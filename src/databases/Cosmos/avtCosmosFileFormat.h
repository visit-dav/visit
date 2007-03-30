// ************************************************************************* //
//                          avtCosmosFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_COSMOS_FILE_FORMAT_H
#define AVT_COSMOS_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>

// ****************************************************************************
//  Class: avtCosmosFileFormat
//
//  Purpose:
//      A file format reader for the Cosmos file format.
//
//  Programmer:  Akira Hadodx
//  Creation:    June 4, 2003
//
// ****************************************************************************

class avtCosmosFileFormat : public avtMTMDFileFormat
{
  public:
                          avtCosmosFileFormat(const char *);
    virtual              ~avtCosmosFileFormat();
    
    virtual const char   *GetType(void) { return "Cosmos File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    void                  ReadMesh(int domain);
    
    vtkDataSet                        **meshes;
    
    std::string                         dirname;
    
    int                                 rank;
    int                                 ndomains;
    int                                 ntimesteps;
    int                                 nscalars;
    int                                 nvectors;
    int                                 npoints;
    int                                 dimensions[3];

    bool                                sphericalCoordinates;
    
    std::string                         timeFileName;
    
    std::vector<std::string>            scalarVarNames;
    std::vector<std::string>            vectorVarNames;


    std::vector<std::string>            gridFileNames;

    //
    // Following matrixes indexed: [domain][varId]
    //
    
    std::vector<std::vector<std::string> >      scalarFileNames;
    struct TripleString
    {
        std::string x,y,z;
    };
    std::vector<std::vector<TripleString> >     vectorFileNames;
};


#endif
