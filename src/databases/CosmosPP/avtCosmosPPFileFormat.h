// ************************************************************************* //
//                          avtCosmosPPFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_COSMOS_PP_FILE_FORMAT_H
#define AVT_COSMOS_PP_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <fstream.h>

class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtCosmosPPFileFormat
//
//  Purpose:
//      A file format reader for the CosmosPP file format.
//
//  Programmer:  Hank Childs
//  Creation:    November 24, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jul 19 16:57:59 PDT 2004
//    Added haveIssuedWarning.
//
// ****************************************************************************

class avtCosmosPPFileFormat : public avtMTMDFileFormat
{
  public:
                          avtCosmosPPFileFormat(const char *);
    virtual              ~avtCosmosPPFileFormat();
    
    virtual const char   *GetType(void) { return "Cosmos++ File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual void          GetTimes(std::vector<double> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    std::string                         dirname;
    
    std::vector<int>                    cycles;
    std::vector<double>                 times;
    int                                 rank;
    int                                 ndomains;
    int                                 ntimesteps;
    int                                 nscalars;
    int                                 nvectors;
    bool                                haveIssuedWarning;

    std::vector<std::string>            scalarVarNames;
    std::vector<std::string>            vectorVarNames;
    std::vector<std::string>            proc_names;
    std::vector<std::string>            dump_names;

    std::vector<std::vector<bool> >                   readDataset;
    std::vector<std::vector<vtkUnstructuredGrid *> >  dataset;

    void ReadDataset(int, int);

    void ReadString(ifstream &ifile, std::string &str);
};

#endif
