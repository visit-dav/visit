//============================================================================
//
//    This work was performed under the auspices of the U.S. Department of
//    Energy by the University of California Lawrence Livermore National
//    Laboratory under contract No. W-7405-Eng-48.
//    Author: Brian McCandless, PMesh Project
//    Copyright 2001 Regents of University of California
//
//============================================================================


#ifndef __KULL_FORMAT_STRUCTURES_H
#define __KULL_FORMAT_STRUCTURES_H

  enum TagType {TAG_BADTYPE = 0, 
        TAG_ZONE = 1, TAG_FACE = 2, TAG_EDGE = 3, TAG_NODE = 4 };

  struct pdb_xyz {
    double x, y, z;
  };
  
  struct pdb_mesh {
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
