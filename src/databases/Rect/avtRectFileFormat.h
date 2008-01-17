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

// ************************************************************************* //
//                             avtRectFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_RECT_FILE_FORMAT_H
#define AVT_RECT_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <avtTypes.h>

#include <vector>
#include <string>
#include <visitstream.h>


class     vtkUnstructuredGrid;

typedef  struct origin_t {
    int x0;
    int y0;
    int z0;
} origin_t;


// ****************************************************************************
//  Class: avtRectFileFormat
//
//  Purpose:
//      A file format reader for multi-timestep, multi-domain files.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
//  Modifications:
// 
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to PopulateDatabaseMetaData
//
//    Mark C. Miller, Wed Jan 16 17:25:43 PST 2008
//    Added support for multiple rect blocks
// ****************************************************************************

class avtRectFileFormat : public avtMTMDFileFormat
{
  public:
                          avtRectFileFormat(const char *);
    virtual              ~avtRectFileFormat();
    
    virtual const char   *GetType(void) { return "Rect File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *ReadMesh(int, int, const char *);
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

  protected:
    vtkDataSet                        **cachedMeshes;

    std::string                         filename;
    std::string                         basename;
    std::string                         dirname;
    int                                 ndomains;
    int                                 ntimesteps;
    int                                 nvars;
    std::vector<std::string>            varnames;
    int                                 xsize,  ysize,  zsize;
    std::vector<int>                    dxsize, dysize, dzsize, numpts;
    std::vector<origin_t>               origins;
    avtMeshType                         gridType;

    void                                ReadVizFile(ifstream &);
    void                                SetUpDomainConnectivity();
};


#endif
