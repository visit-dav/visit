// ************************************************************************* //
//                       avtTimeVaryingExodusFileFormat.h                    //
// ************************************************************************* //

#ifndef AVT_TIME_VARYING_EXODUS_FILE_FORMAT_H
#define AVT_TIME_VARYING_EXODUS_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


class     vtkExodusReader;

class     avtVariableCache;


// ****************************************************************************
//  Class: avtTimeVaryingExodusFileFormat
//
//  Purpose:
//      An time-varying-exodus file format reader.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 08:20:17 PST 2002 
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
// ****************************************************************************

class avtTimeVaryingExodusFileFormat : public avtSTSDFileFormat
{
  public:
                                avtTimeVaryingExodusFileFormat(const char *);
    virtual                    ~avtTimeVaryingExodusFileFormat();

    virtual void                FreeUpResources(void);
    const char                 *GetType(void) { return "Exodus File Format"; };

    virtual vtkDataSet         *GetMesh(const char *);
    virtual vtkDataArray       *GetVar(const char *);
    virtual vtkDataArray       *GetVectorVar(const char *);

    virtual void                PopulateDatabaseMetaData(avtDatabaseMetaData*);
    virtual bool                PerformsMaterialSelection(void) {return true;};

  protected:
    vtkExodusReader            *reader;
    int                         numBlocks;
    bool                        readInDataset;
    std::vector<bool>           validBlock;
    std::vector<int>            blockId;
    std::vector<std::string>    pointVars;
    std::vector<std::string>    cellVars;
    avtVariableCache           *cache;

    vtkExodusReader            *GetReader(void);
    bool                        GetBlockInformation(int &);
    void                        LoadVariable(vtkExodusReader *, const char *);
    vtkDataSet                 *ForceRead(const char *);
    void                        ReadInDataset(void);
};


#endif

