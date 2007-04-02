// ************************************************************************* //
//                              avtVTKWriter.h                               //
// ************************************************************************* //

#ifndef AVT_VTK_WRITER_H
#define AVT_VTK_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtVTKWriter
//
//  Purpose:
//      A module that writes out VTK files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
// ****************************************************************************

class
avtVTKWriter : public avtDatabaseWriter
{
  public:
                   avtVTKWriter(DBOptionsAttributes *);
    virtual       ~avtVTKWriter() {;};

  protected:
    std::string    stem;
    bool           doBinary;

    virtual void   OpenFile(const std::string &);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                std::vector<std::string> &, 
                                std::vector<std::string> &,
                                std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
};


#endif


