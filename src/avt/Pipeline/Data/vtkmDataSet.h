// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            vtkmDataSet.h                                  //
// ************************************************************************* //

#ifndef VTKM_DATA_SET_H
#define VTKM_DATA_SET_H

#include <visit-config.h>

#ifdef HAVE_LIBVTKM
#include <vtkm/cont/DataSet.h>

class  vtkmDataSet
{
  public:
                  vtkmDataSet() {};
    virtual      ~vtkmDataSet() {};

    vtkm::cont::DataSet ds;
};
#endif

#endif
