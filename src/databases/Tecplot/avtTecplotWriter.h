// ************************************************************************* //
//                             avtTecplotWriter.h                              //
// ************************************************************************* //

#ifndef AVT_Tecplot_WRITER_H
#define AVT_Tecplot_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>
#include <vector>
#include <visitstream.h>
class vtkStructuredGrid;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtTecplotWriter
//
//  Purpose:
//      A module that writes out Tecplot files.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 15, 2005
//
// ****************************************************************************

class
avtTecplotWriter : public avtDatabaseWriter
{
  public:
                   avtTecplotWriter();
    virtual       ~avtTecplotWriter();

  protected:
    std::string    stem;
    ofstream       file;

    virtual void   OpenFile(const std::string &);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                std::vector<std::string> &, 
                                std::vector<std::string> &,
                                std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

  private:
    void           WriteCurvilinearMesh(vtkStructuredGrid *, int);
    void           WriteUnstructuredMesh(vtkUnstructuredGrid *, int);
    void           WriteDataArrays(vtkDataSet*);
    std::vector<std::string> variableList;
    bool hadMaterial;
};


#endif
