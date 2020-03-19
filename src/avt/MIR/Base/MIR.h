// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MIR_H
#define MIR_H

#include <mir_exports.h>

#include <MIROptions.h>

#include <vtkSystemIncludes.h>  // for vtkIdType

#include <vector>
#include <deque>

class vtkDataArray;
class vtkDataSet;
class vtkPoints;
class avtMixedVariable;
class avtMaterial;
class avtSpecies;

// ****************************************************************************
//  Class:  MIR
//
//  Purpose:
//    Encapsulation of material interface reconstruction
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 19, 2003
//
//  Note:  refactored base out of what has now become TetMIR.h
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 15 09:48:43 PDT 2003
//    Changed the way some functions were refactored from TetMIR.
//
//    Jeremy Meredith, Wed Oct 15 16:47:49 PDT 2003
//    Added space for a material to get passed in to GetDataset.
//
//    Jeremy Meredith, Thu Aug 18 16:35:05 PDT 2005
//    Added algorithm selector, and added VF for isovolume algorithm.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 11:22:39 EST 2009
//    Added MIR iteration capability.
//
// ****************************************************************************
class MIR_API MIR
{
  public:
    MIR();
    virtual ~MIR();
    
    static void Destruct(void *);

    // set the options
    void SetAlgorithm(int);
    void SetSubdivisionLevel(MIROptions::SubdivisionLevel);
    void SetNumIterations(int);
    void SetIterationDamping(float);
    void SetSmoothing(bool);
    void SetLeaveCleanZonesWhole(bool);
    void SetCleanZonesOnly(bool);
    void SetIsovolumeVF(float);
    void SetAnnealingTime(int);

    // do the processing
    virtual bool Reconstruct3DMesh(vtkDataSet *, avtMaterial *) = 0;
    virtual bool Reconstruct2DMesh(vtkDataSet *, avtMaterial *) = 0;

    // material select everything -- all variables, the mesh, and the material
    // if requested.
    virtual vtkDataSet *GetDataset(std::vector<int>, vtkDataSet *, 
                                   std::vector<avtMixedVariable *>, bool,
                                   avtMaterial * = NULL) = 0;

    // for species selection
    static void SpeciesSelect(const std::vector<bool>&,
                              avtMaterial *, avtSpecies *,
                              vtkDataArray*, avtMixedVariable*,
                              vtkDataArray*&, avtMixedVariable*&);

    // get some result flags
    virtual bool SubdivisionOccurred() = 0;
    virtual bool NotAllCellsSubdivided() = 0;

  protected:

    MIROptions     options;
};

#endif
