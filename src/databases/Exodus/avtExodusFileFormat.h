// ************************************************************************* //
//                          avtExodusFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_EXODUS_FILE_FORMAT_H
#define AVT_EXODUS_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>


class     vtkExodusReader;

class     avtVariableCache;


// ****************************************************************************
//  Class: avtExodusFileFormat
//
//  Purpose:
//      An Exodus file format reader.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
// ****************************************************************************

class avtExodusFileFormat : public avtMTSDFileFormat
{
  public:
                                avtExodusFileFormat(const char *);
    virtual                    ~avtExodusFileFormat();

    virtual void                FreeUpResources(void);
    const char                 *GetType(void) { return "Exodus File Format"; };

    virtual void                GetCycles(std::vector<int> &);
    virtual int                 GetNTimesteps(void);
 
    virtual vtkDataSet         *GetMesh(int, const char *);
    virtual vtkDataArray       *GetVar(int, const char *);
    virtual vtkDataArray       *GetVectorVar(int, const char *);

    virtual void                PopulateDatabaseMetaData(avtDatabaseMetaData*);
    virtual bool                PerformsMaterialSelection(void) {return true;};

  protected:
    vtkExodusReader            *reader;
    int                         numBlocks;
    std::vector<bool>           validBlock;
    std::vector<int>            blockId;
    std::vector<std::string>    pointVars;
    std::vector<std::string>    cellVars;
    avtVariableCache           *cache;
    bool                        readInFile;

    vtkExodusReader            *GetReader(void);
    bool                        GetBlockInformation(int &);
    void                        SetTimestep(int);
    void                        LoadVariable(vtkExodusReader *, const char *);
    vtkDataSet                 *ForceRead(const char *);
    void                        ReadInFile(void);
};


#endif

