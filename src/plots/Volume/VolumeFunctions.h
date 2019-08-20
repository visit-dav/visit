// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VOLUME_FUNCTIONS_H
#define VOLUME_FUNCTIONS_H
class vtkDataSet;
class vtkDataArray;
class vtkRectilinearGrid;
class VolumeAttributes;

//
// These functions are shared between the volume renderer and a filter that
// does work on the engine.
//
// Modifications:
//   Jeremy Meredith, Tue Jan  5 15:48:10 EST 2010
//   Added return value from gradient calculation of maximum gradient value.
//
//   Allen Harvey, Sunday September 4 09:12:00 EST 2011
//   Added function VolumeCalculateGradient_SPH to support unstructured data


vtkDataArray *VolumeGetScalar(vtkDataSet *ds, const char *);

bool VolumeGetScalars(const VolumeAttributes &atts, vtkDataSet *ds,
                      vtkDataArray *&data, vtkDataArray *&opac);

void VolumeGetRange(vtkDataArray *s, float &min, float &max);

void VolumeGetVariableExtents(const VolumeAttributes &atts, 
                              vtkDataArray *data,
                              float varmin, float varmax, 
                              float &vmin, float &vmax, float &vsize);

void VolumeGetOpacityExtents(const VolumeAttributes &atts, 
                             vtkDataArray *opac,
                             float &omin, float &omax, float &osize);

float VolumeCalculateGradient(const VolumeAttributes &atts,
                             vtkRectilinearGrid  *grid,
                             vtkDataArray *opac,
                             float *gx, float *gy, float *gz,
                             float *gm, float *gmn, 
                             float ghostval);

float VolumeCalculateGradient_SPH(
                             vtkDataSet *ds,
                             vtkDataArray *opac,
                             float *gx, float *gy, float *gz,
                             float *gm, float *gmn, 
                             float *hs, bool calcHS,
                             float ghostval);

void VolumeHistograms(const VolumeAttributes &atts, 
                      vtkDataArray *data, vtkDataArray *gm, 
                      float *hist, int hist_size);

void VolumeLogTransform(const VolumeAttributes &atts, 
                        vtkDataArray *linear, vtkDataArray *log);

void VolumeSkewTransform(const VolumeAttributes &atts, 
                         vtkDataArray *linear, vtkDataArray *skew);

#endif
