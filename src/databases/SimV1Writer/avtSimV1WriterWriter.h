#ifndef AVT_SIMV1_WRITER_H
#define AVT_SIMV1_WRITER_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <avtDatabaseWriter.h>
#include <vectortypes.h>
#include <VisItDataInterface_V1.h>

class vtkDataSet;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;
class vtkPolyData;

// ****************************************************************************
// Class: avtSimV1WriterWriter
//
// Purpose:
//   Writer interface that translates requests to write into data transfers
//   back to the simulation. This lets the simulation provide callbacks that
//   let it accept data back from VisIt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 30 14:22:32 PST 2006
//
// Modifications:
//    Jeremy Meredith, Tue Mar 27 11:39:24 EDT 2007
//    Added numblocks to the OpenFile method, and save off the actual
//    encountered mesh types, because we cannot trust the metadata.
//   
// ****************************************************************************

class avtSimV1WriterWriter : public virtual avtDatabaseWriter
{
public:
                   avtSimV1WriterWriter();
    virtual       ~avtSimV1WriterWriter();

protected:
    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                std::vector<std::string> &,
                                std::vector<std::string> &,
                                std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

private:
    const avtDatabaseMetaData     *metadata;
    std::string                    objectName;
    stringVector                   varList;
    int                            numblocks;
    VisIt_SimulationWriterCallback cb;

    void           WriteCurvilinearMesh(vtkStructuredGrid *,
                                       int, VisIt_MeshMetaData *);
    void           WriteUnstructuredMesh(vtkUnstructuredGrid *,
                                       int, VisIt_MeshMetaData *);
    void           WriteRectilinearMesh(vtkRectilinearGrid *,
                                       int, VisIt_MeshMetaData *);
    void           WritePolyDataMesh(vtkPolyData *,
                                       int, VisIt_MeshMetaData *);

    void           WriteDataArrays(vtkDataSet *ds, int chunk);
    void           WriteOneDataArray(vtkDataArray *, const std::string &, int, int);
    void           WriteDataArraysConditionally(vtkDataSet *, int,
                                                const unsigned char *);
    void           WriteCellDataArrayConditionally(vtkDataArray *,
                                                   const std::string &, int,
                                                   const unsigned char *);
};


#endif
