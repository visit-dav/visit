// ************************************************************************* //
//                          avtExodusFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_EXODUS_FILE_FORMAT_H
#define AVT_EXODUS_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <string>


class     vtkVisItExodusReader;

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
//    Hank Childs, Sat Apr 17 07:42:40 PDT 2004
//    Added support for times.
//
//    Hank Childs, Thu Jul 22 11:29:31 PDT 2004
//    Make materials go throw the standard generic database path.  Also add
//    support for registering file lists.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Added a ReadMesh mehtod
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Removed GetCycles method since all it did was the same thing the
//    default implementation did. Added timeState arg to
//    PopulateDatabaseMetaData
//
// ****************************************************************************

class avtExodusFileFormat : public avtMTSDFileFormat
{
  public:
                                avtExodusFileFormat(const char *);
    virtual                    ~avtExodusFileFormat();
 
    static int                  RegisterFileList(const char *const *, int);
    void                        SetFileList(int fl) { fileList = fl; };

    virtual void                FreeUpResources(void);
    const char                 *GetType(void) { return "Exodus File Format"; };

    virtual void                GetTimes(std::vector<double> &);
    virtual int                 GetNTimesteps(void);
 
    virtual vtkDataSet         *GetMesh(int, const char *);
    virtual vtkDataArray       *GetVar(int, const char *);
    virtual vtkDataArray       *GetVectorVar(int, const char *);

    virtual void                PopulateDatabaseMetaData(avtDatabaseMetaData*, int);

    virtual void         *GetAuxiliaryData(const char *var, int, 
                                           const char *type, void *args,
                                           DestructorFunction &);

  protected:
    vtkVisItExodusReader            *reader;
    int                         numBlocks;
    std::vector<bool>           validBlock;
    std::vector<int>            blockId;
    std::vector<std::string>    pointVars;
    std::vector<std::string>    cellVars;
    avtVariableCache           *exodusCache;
    bool                        readInFile;
    int                         fileList;

    // Note: this needs to be a pointer because there are issues with 
    // constructors being called in shared libraries for static objects.
    static std::vector< std::vector<std::string> > *globalFileLists;

    vtkVisItExodusReader            *GetReader(void);
    void                        SetTimestep(int);
    void                        LoadVariable(vtkVisItExodusReader *, const char *);
    vtkDataSet                 *ForceRead(const char *);
    void                        ReadInFile(void);
    vtkDataSet                 *ReadMesh(int, const char *, bool);
};


#endif

