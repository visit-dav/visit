// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtH5PartWriter.h                            //
// ************************************************************************* //

#ifndef AVT_H5PART_WRITER_H
#define AVT_H5PART_WRITER_H

#include <visit-config.h>

#include <avtDatabaseWriter.h>

// STL
#include <vector>
#include <string>

class DBOptionsAttributes;
class vtkUnstructuredGrid;
class vtkPoints;

struct H5PartFile;

// ****************************************************************************
//  Class: avtH5PartWriter
//
//  Purpose:
//      A module that writes out H5Part files.
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
//    Eddie Rusu, Wed Oct 14 15:52:13 PDT 2020
//    Removed FastBit and FastQuery
//
// ****************************************************************************

class avtH5PartWriter : public virtual avtDatabaseWriter
{
  public:
                   avtH5PartWriter(const DBOptionsAttributes *);
    virtual       ~avtH5PartWriter();

  protected:
    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual bool   SequentialOutput() const;

    H5PartFile    *file;

private:
    void   WriteUnstructuredMesh(vtkUnstructuredGrid *, int);
    void   WritePolyData(vtkPolyData *pd, int);
    void   WriteDataArrays(vtkDataSet *ds1);
    void   WritePoints(vtkPoints *pts);

    void   WriteParentFile();

    const std::string getVariablePathPrefix( int timestep );

    static int            INVALID_CYCLE;
    static double         INVALID_TIME;

  
    std::vector<std::string>    variableList;
    std::string    filename;
    std::string    meshname;
    double         time;
    int            cycle;

    std::string variablePathPrefix;
    std::string parentFilename;
    bool        createParentFile;
};

#endif
