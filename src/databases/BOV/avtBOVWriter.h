// ************************************************************************* //
//                              avtBOVWriter.h                               //
// ************************************************************************* //

#ifndef AVT_BOV_WRITER_H
#define AVT_BOV_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>


// ****************************************************************************
//  Class: avtBOVWriter
//
//  Purpose:
//      A module that writes out BOV files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
// ****************************************************************************

class
avtBOVWriter : public avtDatabaseWriter
{
  public:
                   avtBOVWriter() {;};
    virtual       ~avtBOVWriter() {;};

  protected:
    std::string    stem;

    virtual void   OpenFile(const std::string &);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                std::vector<std::string> &, 
                                std::vector<std::string> &,
                                std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    virtual bool   SupportsTargetChunks(void) { return true; };
    virtual bool   SupportsTargetZones(void) { return true; };

    // Doing material interface reconstruction does nothing for us. 
    // So we may as well report that we can already do it, so the MIR
    // doesn't execute unnecessarily.
    virtual bool   CanHandleMaterials(void) { return true; };
};


#endif


