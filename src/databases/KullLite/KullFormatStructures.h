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

//============================================================================
//
//  Programmer: Brian McCandless, PMesh Project
//
//  Modifications:
//
//    Hank Childs, Mon Jul 26 09:03:27 PDT 2004
//    Make a split between 2D and 3D meshes.
//
//============================================================================


#ifndef __KULL_FORMAT_STRUCTURES_H
#define __KULL_FORMAT_STRUCTURES_H

  enum TagType {TAG_BADTYPE = 0, 
        TAG_ZONE = 1, TAG_FACE = 2, TAG_EDGE = 3, TAG_NODE = 4 };

  struct pdb_xyz {
    double x, y, z;
  };
  
  struct pdb_mesh3d {
    int       ndimensions;
    int       npnts;
    pdb_xyz * positions;
    int       nfaces;
    int *     faceToNodesIndex;
    int *     nodeIndices;
    int       nzones;
    int *     zoneToFacesIndex;
    int *     faceIndices;
  };
  
  struct pdb_xy {
    double x, y;
  };

  struct pdb_mesh2d {
    int       ndimensions;
    int       npnts;
    pdb_xy  * positions;
    int       nedges;
    int *     nodeIndices;
    int       nzones;
    int *     zoneToEdgesIndex;
    int *     edgeIndices;
  };
  
  struct pdb_comm {
    int   neighborID;
    int   listSize;
    int * list;
  };
  
  struct pdb_tag {
    char * tagname;
    int    type;
    int    size;
  };

  struct pdb_taglist {
    int       num_tags;
    pdb_tag * tags; 
  };

#endif 
