// ************************************************************************* //
//                            avtDatabaseWriter.h                            //
// ************************************************************************* //

#ifndef AVT_DATABASE_WRITER_H
#define AVT_DATABASE_WRITER_H

#include <avtOriginatingDatasetSink.h>

#include <string>
#include <vector>

#include <pipeline_exports.h>


class     avtDatabaseMetaData;


// ****************************************************************************
//  Class: avtDatabaseWriter
//
//  Purpose:
//      This is a general, abstract interface for writing out databases.  It is
//      designed to be utilized by the plugin writers.
//      This differs from avtDatasetFileWriter, in that avtDatasetFileWriter is
//      specialized to specific polygonal file formats.  That writer should
//      probably be renamed.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Hank Childs, Sat Sep 11 10:03:49 PDT 2004
//    Add support for repartitioning the chunks.  Also support reading only
//    one variable.
//
//    Brad Whitlock, Mon Nov 1 15:51:32 PST 2004
//    I replaced long long with VISIT_LONG_LONG so the code can build 
//    on Windows using the MSVC 6.0 compiler.
//
//    Jeremy Meredith, Wed Feb 16 15:01:40 PST 2005
//    Added ability to disable MIR and Expressions.
//
//    Hank Childs, Thu May 26 15:45:48 PDT 2005
//    Add an "all-variables" option.
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
// ****************************************************************************

class PIPELINE_API avtDatabaseWriter : public virtual avtOriginatingDatasetSink
{
  public:
                       avtDatabaseWriter();
    virtual           ~avtDatabaseWriter();
   
    void               Write(const std::string &, const avtDatabaseMetaData *);
    void               Write(const std::string &, const avtDatabaseMetaData *,
                             std::vector<std::string> &, bool allVars = true);

    void               SetShouldAlwaysDoMIR(bool s)
                             { shouldAlwaysDoMIR = s; };

    void               SetShouldNeverDoMIR(bool s)
                             { shouldNeverDoMIR = s; };

    void               SetShouldNeverDoExpressions(bool s)
                             { shouldNeverDoExpressions = s; };

    bool               SetTargetChunks(int nChunks);
    bool               SetTargetZones(VISIT_LONG_LONG nTotalZones);
    void               SetVariableList(std::vector<std::string> &);

  protected:
    bool               shouldAlwaysDoMIR;
    bool               shouldNeverDoMIR;
    bool               shouldNeverDoExpressions;
    bool               mustGetMaterialsAdditionally;
    bool               hasMaterialsInProblem;

    bool               shouldChangeChunks;
    bool               shouldChangeTotalZones;
    int                nTargetChunks;
    VISIT_LONG_LONG    targetTotalZones;

    virtual bool       CanHandleMaterials(void) { return false; };

    virtual void       OpenFile(const std::string &) = 0;
    virtual void       WriteHeaders(const avtDatabaseMetaData *, 
                           std::vector<std::string>&,std::vector<std::string>&,
                           std::vector<std::string> &) = 0;
    virtual void       WriteChunk(vtkDataSet *, int) = 0;
    virtual void       CloseFile(void) = 0;

    virtual bool       SupportsTargetChunks(void) { return false; };
    virtual bool       SupportsTargetZones(void) { return false; };
};


#endif


