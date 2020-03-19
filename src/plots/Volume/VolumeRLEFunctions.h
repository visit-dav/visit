// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VOLUME_RLE_FUNCTIONS_H
#define VOLUME_RLE_FUNCTIONS_H
#include <vectortypes.h>

void VolumeRLECompress(const unsignedCharVector &in, unsignedCharVector &out);

void VolumeRLEDecompress(const unsignedCharVector &in, unsignedCharVector &out);

#endif
