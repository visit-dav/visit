// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_MATERIAL_ENCODER_H
#define AVT_MATERIAL_ENCODER_H

#include <pipeline_exports.h>

#include <string>
#include <vector>

class avtMaterial;

// ****************************************************************************
// Class: avtMaterialEncoder
//
// Purpose:
//   Keeps track of mixed material information.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:35 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 16:15:38 PST 2004
//   Changed the double argument to float on the AddMixed method.
//
// ****************************************************************************

class PIPELINE_API avtMaterialEncoder
{
public:
    avtMaterialEncoder();
    ~avtMaterialEncoder();

    void AddMaterial(const std::string &mat);
    void AddClean(int zoneId, int matNumber);
    void AddMixed(int zoneId, const int *matNumbers, const float *matVf,
                  int nMats);
    void AllocClean(int nZones);
    int  GetMixedSize() const;

    avtMaterial *CreateMaterial(const int *dims, int ndims) const;
private:
    void Resize(int nMats);

    int    have_mixed;
    int    *mix_zone;
    int    *mix_mat;
    float  *mix_vf;
    int    *mix_next;
    int    *matlist;
    int    _array_size;
    int    _array_index;
    int    _array_growth;
    std::vector<std::string> matNames;
};

#endif
