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
// ****************************************************************************

class PIPELINE_API avtDatabaseWriter : public avtOriginatingDatasetSink
{
  public:
                       avtDatabaseWriter();
    virtual           ~avtDatabaseWriter() {;};
   
    void               Write(const std::string &, const avtDatabaseMetaData *);
    void               SetShouldAlwaysDoMIR(bool s)
                             { shouldAlwaysDoMIR = s; };

  protected:
    bool               shouldAlwaysDoMIR;
    bool               mustGetMaterialsAdditionally;
    bool               hasMaterialsInProblem;

    virtual bool       CanHandleMaterials(void) { return false; };

    virtual void       OpenFile(const std::string &) = 0;
    virtual void       WriteHeaders(const avtDatabaseMetaData *, 
                           std::vector<std::string>&,std::vector<std::string>&,
                           std::vector<std::string> &) = 0;
    virtual void       WriteChunk(vtkDataSet *, int) = 0;
    virtual void       CloseFile(void) = 0;
};


#endif


