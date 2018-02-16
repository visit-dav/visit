/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

class avtH5PartWriter : public virtual avtDatabaseWriter
{
  public:
                   avtH5PartWriter(DBOptionsAttributes *);
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

#ifdef HAVE_LIBFASTQUERY
    const std::string getFastBitIndexPathPrefix( int timestep );
  
    bool        addFastBitIndexing;
    std::string fastBitIndexPathPrefix;
    std::string sortedKey;
#endif  
};

#endif
