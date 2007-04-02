/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtPoint3DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_POINT3D_FILE_FORMAT_H
#define AVT_POINT3D_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


#include <vector>
#include <string>


class     vtkFloatArray;
class     vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtPoint3DFileFormat
//
//  Purpose:
//      A file format reader for curves.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//   Brad Whitlock, Mon Jun 5 10:43:55 PDT 2006
//   Added config file support.
//
// ****************************************************************************

class avtPoint3DFileFormat : public avtSTSDFileFormat
{
  public:
                               avtPoint3DFileFormat(const char *);
    virtual                   ~avtPoint3DFileFormat();
    
    virtual const char        *GetType(void) { return "3D points"; };
    
    virtual vtkDataSet        *GetMesh(const char *);
    virtual vtkDataArray      *GetVar(const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int                GetCycle(void) { return 0; };

  protected:
    bool                       haveReadData;
    vtkFloatArray             *column1;
    vtkFloatArray             *column2;
    vtkFloatArray             *column3;
    vtkFloatArray             *column4;
    vtkUnstructuredGrid       *points;
    std::vector<std::string>   varnames;

    static const char         *MESHNAME;

    void                       ReadData(void);
    bool                       ReadConfigFile(int &);
};


#endif


