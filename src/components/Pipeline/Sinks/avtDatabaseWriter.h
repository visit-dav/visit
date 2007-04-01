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
// ****************************************************************************

class PIPELINE_API avtDatabaseWriter : public avtOriginatingDatasetSink
{
  public:
                       avtDatabaseWriter();
    virtual           ~avtDatabaseWriter();
   
    void               Write(const std::string &, const avtDatabaseMetaData *);
    void               Write(const std::string &, const avtDatabaseMetaData *,
                             std::vector<std::string> &);

    void               SetShouldAlwaysDoMIR(bool s)
                             { shouldAlwaysDoMIR = s; };

    bool               SetTargetChunks(int nChunks);
    bool               SetTargetZones(long long nTotalZones);
    void               SetVariableList(std::vector<std::string> &);

  protected:
    bool               shouldAlwaysDoMIR;
    bool               mustGetMaterialsAdditionally;
    bool               hasMaterialsInProblem;

    bool               shouldChangeChunks;
    bool               shouldChangeTotalZones;
    int                nTargetChunks;
    long long          targetTotalZones;

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


