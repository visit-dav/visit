/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                             avtVTKFileReader.h                            //
// ************************************************************************* //

#ifndef AVT_VTK_FILE_READER_H
#define AVT_VTK_FILE_READER_H

#include <map>
#include <string>
#include <void_ref_ptr.h>

class vtkDataArray;
class vtkDataSet;
class vtkRectilinearGrid;
class vtkStructuredPoints;
class vtkVisItXMLPDataReader;

class DBOptionsAttributes;


// ****************************************************************************
//  Class: avtVTKFileReader
//
//  Purpose:
//      Handles files of the .vtk file format.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Thu Mar 11 12:53:12 PST 2004 
//    Added ConvertStructuredPointsToRGrid. 
//
//    Hank Childs, Tue May 24 12:06:52 PDT 2005
//    Added argument to constructor for DB options.
//
//    Eric Brugger, Fri Aug 12 11:28:43 PDT 2005
//    Added GetCycleFromFilename.
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Added GetAuxiliaryData to support materials
//
//    Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005 
//    Added 'extension' to store file extension. 
//
//    Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006
//    Added GetTime method.
//
//    Hank Childs, Tue Sep 26 14:09:18 PDT 2006
//    Remove class qualification of method definition, which xlC dislikes.
//
//    Kathleen Bonnell, Wed Jul  9 18:13:50 PDT 2008
//    Added GetCycle method.
//
//    Brad Whitlock, Wed Oct 26 11:01:00 PDT 2011
//    I added vtkCurves.
//
//    Eric Brugger, Mon Jun 18 12:26:52 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Brad Whitlock, Mon Oct 22 16:35:15 PDT 2012
//    Make it a helper class for the real readers.
//
//    Eric Brugger, Tue Jul  9 09:33:28 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
// ****************************************************************************

class avtVTKFileReader
{
  public:
    avtVTKFileReader(const char *,DBOptionsAttributes *);
    ~avtVTKFileReader();

    int           GetNumberOfDomains();

    vtkDataSet   *GetMesh(int, const char *);
    vtkDataArray *GetVar(int, const char *);
    vtkDataArray *GetVectorVar(int, const char *);
    void         *GetAuxiliaryData(const char *var, int,
                                   const char *type, void *, DestructorFunction &df);

    void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    void          FreeUpResources(void);

    double        GetTime(void);
    int           GetCycle(void);

  protected:
    static int            INVALID_CYCLE;
    static double         INVALID_TIME;

    char                 *filename;

    bool                  readInDataset;

    int                   nblocks;
    char                **pieceFileNames;
    vtkDataSet          **pieceDatasets;
    int                 **pieceExtents;

    static const char    *MESHNAME;
    static const char    *VARNAME;
    char                 *matvarname;
    std::vector<int>      matnos;
    std::vector<std::string> matnames;
    double                vtk_time;
    int                   vtk_cycle;

    std::string           fileExtension;
    std::string           pieceExtension;

    std::map<std::string, vtkRectilinearGrid *> vtkCurves;

    void                  ReadInFile(int _domain=-1);
    void                  ReadInDataset(int domain);
    vtkDataSet           *ConvertStructuredPointsToRGrid(vtkStructuredPoints *,
                                                         int *);
    void                  CreateCurves(vtkRectilinearGrid *rgrid);

    // Borrowed from avtFileFormat.
    void       AddScalarVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering,
                                      const double * = NULL,
                                      const bool = false);
    void       AddVectorVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering, int = 3,
                                      const double * = NULL);
    void       AddTensorVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering, int = 3);
    void       AddArrayVarToMetaData(avtDatabaseMetaData *, std::string, int,
                                     std::string, avtCentering);
};


#endif


