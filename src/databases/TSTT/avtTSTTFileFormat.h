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
//                            avtTSTTFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_TSTT_FILE_FORMAT_H
#define AVT_TSTT_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <TSTTM.hh>

// these should really be defined by TSTT but they aren't
typedef void* EntityHandle;
typedef void* EntitySetHandle;
typedef void* TagHandle;

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;


// ****************************************************************************
//  Class: avtTSTTFileFormat
//
//  Purpose:
//      Reads in TSTT files as a plugin to VisIt.
//
//  Programmer: Mark C. Miller 
//  Creation:   Wed Mar 7 17:15:33 PST 2007
//
//  Modifications:
//
//    Mark C. Miller, Thu Mar 22 09:37:55 PDT 2007
//    Added handles for primitiveEntities so we don't attempt to get them
//    more than once.
//    Added stuff to start processing the entity set hierarchy. It is not
//    yet in use though.
//
// ****************************************************************************

class avtTSTTFileFormat : public avtSTMDFileFormat
{
  public:
                       avtTSTTFileFormat(const char *);
    virtual           ~avtTSTTFileFormat() {;};

    virtual const char    *GetType(void)   { return "TSTT"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:
      typedef struct _VisItEntitySetInfo {
          int domainId;
          unsigned int groupId;
          int matId;
          vector<string> varNames;
      } VisItEntitySetInfo_t;

      string               vmeshFileName;
      TSTTM::Mesh          tsttMesh;
      void                *rootSet;
      sidl::array<EntityHandle>  *vertEnts;
      sidl::array<EntityHandle>  *edgeEnts;
      sidl::array<EntityHandle>  *faceEnts;
      sidl::array<EntityHandle>  *regnEnts;
      int                  geomDim;
      int                  topoDim;
      int                  numVerts;
      int                  numEdges;
      int                  numFaces;
      int                  numRegns;
      TSTTM::EntityType    domToEntType[4];
      bool                 haveMixedElementMesh;
      vector<TagHandle>    primitiveTagHandles[4];
      map<EntitySetHandle,VisItEntitySetInfo_t> esMap;

};


#endif
