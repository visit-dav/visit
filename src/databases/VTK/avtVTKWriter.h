// ************************************************************************* //
//                              avtVTKWriter.h                               //
// ************************************************************************* //

#ifndef AVT_VTK_WRITER_H
#define AVT_VTK_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>


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
                   avtVTKWriter() {;};
    virtual       ~avtVTKWriter() {;};

  protected:
    std::string    stem;

    virtual void   OpenFile(const std::string &);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                std::vector<std::string> &, 
                                std::vector<std::string> &,
                                std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
};


#endif


