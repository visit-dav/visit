/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                             avtSiloWriter.h                               //
// ************************************************************************* //

#ifndef AVT_SILO_WRITER_H
#define AVT_SILO_WRITER_H

#include <avtDatabaseWriter.h>

#include <map>
#include <string>
#include <vector>

#include <silo.h>


class vtkCellData;
class vtkDataSetAttributes;
class vtkPointData;
class vtkPolyData;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

class avtMeshMetaData;

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtSiloWriter
//
//  Purpose:
//      A module that writes out Silo files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications
//    Mark C. Miller, Tue Mar  9 07:03:56 PST 2004
//    Added data members to hold args passed in WriteHeaders to facilitate
//    writing extents on multi-objects in CloseFile()
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Jeremy Meredith, Tue Mar 27 11:39:24 EDT 2007
//    Added numblocks to the OpenFile method, and save off the actual
//    encountered mesh types, because we cannot trust the metadata.
//
//    Cyrus Harrison, Thu Aug 16 20:50:24 PDT 2007
//    Added dir to hold output directory.
//
//    Cyrus Harrison, Wed Feb 27 07:57:35 PST 2008
//    Added VTKZoneTypeToSiloZoneType helper method to help with 
//    DBPutZoneList2 migration. 
//
//    Mark C. Miller, Wed Jul 23 17:49:39 PDT 2008
//    Added bool to WriteUcdvars to handle point vars    
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Brad Whitlock, Fri Mar 6 15:30:16 PST 2009
//    I added helper functions so we can share code when writing variables.
//    I also added code to export expressions.
//
//    Brad Whitlock, Tue May 19 12:22:38 PDT 2009
//    I added an argument to WriteUcdvars.
//
//    Eric Brugger, Mon Jun 22 16:35:47 PDT 2009
//    I modified the writer to handle the case where the meshes in a
//    multimesh or multivar were not all of the same type.
//
//    Mark C. Miller, Tue Jun 14 10:35:34 PDT 2016
//    Added arg to ConstrutMulti... methods. Added SingleFile() method to
//    subsume functionality of singleFile write option.
// ****************************************************************************

class
avtSiloWriter : public virtual avtDatabaseWriter
{
  public:
                   avtSiloWriter(DBOptionsAttributes *);
    virtual       ~avtSiloWriter();

  protected:
    std::string         stem;
    std::string         dir;
    std::string         meshname;
    std::string         matname;
    int            nblocks;
    int            driver;
    int            nmeshes;
    int           *meshtypes;
    int           *chunkToFileMap;
    DBoptlist     *optlist;
    bool           singleFile;
    bool           checkSums;
    std::string    compressionParams;

    // places to hold args passed in WriteHeaders
    const avtDatabaseMetaData           *headerDbMd;
    std::vector<std::string>             headerScalars;
    std::vector<std::string>             headerVectors;
    std::vector<std::string>             headerMaterials;

    // storage for per-block info to be written in DBPutMulti... calls 
    std::vector<double>               spatialMins;
    std::vector<double>               spatialMaxs;
    std::vector<int>                  zoneCounts;
    std::map<std::string,std::vector<double> > dataMins;
    std::map<std::string,std::vector<double> > dataMaxs;

    bool           WriteContextIsDefault();
    int            FileID();
    bool           ShouldCreateFile();

    virtual bool   CanHandleMaterials(void) { return true; };


    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual void   WriteRootFile();

    void           ConstructMultimesh(DBfile *dbfile, const avtMeshMetaData *,
                       int const *, int const *);
    void           ConstructMultivar(DBfile *dbfile, const std::string &,
                       const avtMeshMetaData *, int const *, int const *);
    void           ConstructMultimat(DBfile *dbfile, const std::string &,
                       const avtMeshMetaData *, int const *);
    void           ConstructChunkOptlist(const avtDatabaseMetaData *);

    void           WriteCurvilinearMesh(DBfile *, vtkStructuredGrid *, int);
    void           WritePolygonalMesh(DBfile *, vtkPolyData *, int);
    void           WriteRectilinearMesh(DBfile *, vtkRectilinearGrid *, int);
    void           WriteUnstructuredMesh(DBfile *, vtkUnstructuredGrid *, int);

    void           WriteUcdvars(DBfile *, vtkPointData *, vtkCellData *, bool,
                                const unsigned char *);
    void           WriteQuadvars(DBfile *, vtkPointData *, vtkCellData *,
                                    int, int *);
    void           WriteMaterials(DBfile *, vtkCellData *, int);
    int            VTKZoneTypeToSiloZoneType(int);
    void           WriteUcdvarsHelper(DBfile *dbfile, vtkDataSetAttributes *ds, 
                                      bool isPointMesh, int centering,
                                      const unsigned char *gz);
    void           WriteQuadvarsHelper(DBfile *dbfile, vtkDataSetAttributes *ds,
                                       int ndims, int *dims, int centering);

    void           WriteExpressions(DBfile *dbfile);
};


#endif


