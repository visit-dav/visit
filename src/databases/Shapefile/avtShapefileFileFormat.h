/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtShapefileFileFormat.h                       //
// ************************************************************************* //

#ifndef AVT_Shapefile_FILE_FORMAT_H
#define AVT_Shapefile_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <esriShapefile.h>
#include <dbfFile.h>
#include <vector>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtShapefileFileFormat
//
//  Purpose:
//      Reads in ESRI Shapefile files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Fri Apr 1 23:41:51 PST 2005
//    Added GetNumRepeats.
//
//    Brad Whitlock, Tue Feb 27 11:45:59 PDT 2007
//    Added CountShapeTypes.
//
//    Brad Whitlock, Wed Mar 7 11:42:07 PDT 2007
//    Added support for tessellation.
//
// ****************************************************************************

class avtShapefileFileFormat : public avtSTSDFileFormat
{
public:
                       avtShapefileFileFormat(const char *filename,
                           const DBOptionsAttributes *rdopts);
    virtual           ~avtShapefileFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      &GetAuxiliaryData(const char *var, const char *type,
    //                                  void *args, DestructorFunction &);
    //

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual bool      ReturnsValidCycle() const { return true; };
    // virtual int       GetCycle(void);
    // virtual bool      ReturnsValidTime() const { return true; };
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "ESRI Shapefile"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    struct esriShape
    {
        esriShapeType_t  shapeType;
        void            *shape;
        int              nRepeats;
    };

    typedef std::vector<esriShape> esriShapeVector;

    void                   Initialize();

    int                    CountMemberPoints(esriShapeType_t) const;
    int                    CountShapes(esriShapeType_t) const;
    int                    CountCellsForShape(esriShapeType_t) const;
    int                    GetNumRepeats(const esriShape &shape, bool) const;
    int                    CountShapeTypes() const;
    vtkDataSet            *GetMesh_TessellatedPolygon();

    bool                   initialized;
    esriShapeVector        shapes;
    int                    numShapeTypes;
    dbfFile_t             *dbfFile;

    // Options.
    bool                   polygonsAsLines;
    bool                   tessellatePolygons;
    bool                   esriLogging;
    bool                   dbfLogging;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
