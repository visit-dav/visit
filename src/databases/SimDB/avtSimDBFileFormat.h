// ************************************************************************* //
//                            avtSimDBFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SimDB_FILE_FORMAT_H
#define AVT_SimDB_FILE_FORMAT_H

#include <database_exports.h>

#include <avtSTMDFileFormat.h>

#include <vector>


// ****************************************************************************
//  Class: avtSimDBFileFormat
//
//  Purpose:
//      Reads in a Simulation file as a plugin to VisIt.
//      Note: This is the Metadata Server -only portion of the plugin.
//      Each simulation creates its own specific Engine plugin.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 11, 2004
//
// ****************************************************************************

class avtSimDBFileFormat : public avtSTMDFileFormat
{
  public:
                       avtSimDBFileFormat(const char *);
    virtual           ~avtSimDBFileFormat() {;};

    virtual const char    *GetType(void)   { return "SimDB"; };
    virtual void           FreeUpResources(void); 
    virtual int            GetCycle() { return -1; }

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    std::string            host;
    int                    port;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
