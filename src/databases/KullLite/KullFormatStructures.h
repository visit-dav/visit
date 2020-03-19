// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
