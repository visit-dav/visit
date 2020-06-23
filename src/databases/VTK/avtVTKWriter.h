// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtVTKWriter.h                               //
// ************************************************************************* //

#ifndef AVT_VTK_WRITER_H
#define AVT_VTK_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>
#include <vector>

#include <ExpressionList.h>

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
//  Modifications:
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Better support for single-block.
//
//    Jeremy Meredith, Tue Mar 27 11:39:24 EDT 2007
//    Added numblocks to the OpenFile method, and save off the actual
//    encountered mesh types, because we cannot trust the metadata.
//
//    Kathleen Biagas, Thu Dec 18 14:19:13 PST 2014
//    Added doXML.
//
//    Kathleen Biagas, Wed Feb 25 13:25:07 PST 2015
//    Added meshName.
//
//    Kathleen Biagas, Tue Sep  1 11:27:23 PDT 2015
//    Add storage for fileNames, used when exporting multi-block XML.
//
//    Mark C. Miller, Tue Apr  9 18:44:50 PDT 2019
//    Add tetrahedralize option.
//
//    Mark C. Miller, Mon Mar  9 19:51:15 PDT 2020
//    Add exprList
// ****************************************************************************

class
avtVTKWriter : public virtual avtDatabaseWriter
{
  public:
                   avtVTKWriter(const DBOptionsAttributes *);
    virtual       ~avtVTKWriter() {;};

  protected:
    static int            INVALID_CYCLE;
    static double         INVALID_TIME;

    std::string    stem;
    std::string    meshName;
    std::string    mbDirName;
    double         time;
    int            cycle;
    ExpressionList exprList;
    bool           doBinary;
    bool           doXML;
    bool           tetrahedralize;
    int            nblocks;
    std::vector<std::string> fileNames;

    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual void   WriteRootFile();
};


#endif


