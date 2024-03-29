// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtWavefrontOBJWriter.h                       //
// ************************************************************************* //

#ifndef AVT_WavefrontOBJ_WRITER_H
#define AVT_WavefrontOBJ_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>

class DBOptionsAttributes;
class vtkPolyData;
class vtkImageData;

// ****************************************************************************
//  Class: avtWavefrontOBJWriter
//
//  Purpose:
//      A module that writes out WavefrontOBJ files.
//
//  Programmer: pugmire -- generated by xml2avt
//  Creation:   Tue Apr 16 08:33:21 PDT 2013
//
//  Modifications:
//    Brad Whitlock, Tue Sep  8 17:03:17 PDT 2015
//    Rely on base class for geometry aggregation.
// 
//    Justin Privitera, Fri Nov  3 15:25:32 PDT 2023
//    Added doColor, colorTable, and GetColorTable().
// 
//    Justin Privitera, Tue Nov 28 17:31:40 PST 2023
//    Added invert color table option.
//
// ****************************************************************************

class
avtWavefrontOBJWriter : public avtDatabaseWriter
{
  public:
                   avtWavefrontOBJWriter(const DBOptionsAttributes *);
    virtual       ~avtWavefrontOBJWriter() {}

  protected:
    std::string    stem;

    virtual void   OpenFile(const std::string &, int nb);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    virtual bool          CreateTrianglePolyData() const;
    virtual CombineMode   GetCombineMode(const std::string &plotName) const;

  private:
    vtkImageData   *GetColorTable();
    
    bool                       doColor;
    std::string                colorTable;
    bool                       invertCT;
};


#endif
