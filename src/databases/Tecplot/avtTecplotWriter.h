// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTecplotWriter.h                            //
// ************************************************************************* //

#ifndef AVT_TECPLOT_WRITER_H
#define AVT_TECPLOT_WRITER_H

#include <avtDatabaseWriter.h>
#include <DBOptionsAttributes.h>

#include <string>
#include <vector>
#include <visitstream.h>
#include <visit_gzstream.h>

class vtkPoints;
class vtkPolyData;
class vtkRectilinearGrid;
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
//  Modifications:
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Jeremy Meredith, Tue Mar 27 17:03:47 EDT 2007
//    Added numblocks (currently ignored) to the OpenFile interface.
//
//    Brad Whitlock, Wed Sep  2 14:16:43 PDT 2009
//    I added methods for writing rectilinear and polydata datasets.
//
// ****************************************************************************

class avtTecplotWriter : public virtual avtDatabaseWriter
{
  public:
                   avtTecplotWriter(DBOptionsAttributes *);
    virtual       ~avtTecplotWriter();

  protected:
    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual bool   SequentialOutput() const;

  private:
    void           WritePolyData(vtkPolyData *, int);
    void           WriteRectilinearMesh(vtkRectilinearGrid *, int);
    void           WriteCurvilinearMesh(vtkStructuredGrid *, int);
    void           WriteUnstructuredMesh(vtkUnstructuredGrid *, int);

    void           WritePoints(vtkPoints *pts, int dim);
    void           WriteDataArrays(vtkDataSet*);
    void           WriteVariables(const std::vector<std::string> &);

    bool           ReallyHasMaterials();

    std::string              stem;
    visit_ofstream           file;
    std::vector<std::string> variableList;
    std::vector<std::string> materialList;
    bool                     variablesWritten;
    int                      gzipLevel;
};


#endif
