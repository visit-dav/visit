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
//
// ****************************************************************************
class MIR_API MIR
{
  public:
    MIR();
    virtual ~MIR();
    
    static void Destruct(void *);

    // set the options
    void SetSubdivisionLevel(MIROptions::SubdivisionLevel);
    void SetNumIterations(int);
    void SetSmoothing(bool);
    void SetLeaveCleanZonesWhole(bool);
    void SetCleanZonesOnly(bool);

    // do the processing
    virtual bool Reconstruct3DMesh(vtkDataSet *, avtMaterial *) = 0;
    virtual bool Reconstruct2DMesh(vtkDataSet *, avtMaterial *) = 0;

    // material select everything -- all variables, the mesh, and the material
    // if requested.
    virtual vtkDataSet *GetDataset(std::vector<int>, vtkDataSet *, 
                                   std::vector<avtMixedVariable *>, bool) = 0;

    // for species selection
    static void SpeciesSelect(const std::vector<bool>&,
                              avtMaterial *, avtSpecies *,
                              vtkDataArray*, avtMixedVariable*,
                              vtkDataArray*&, avtMixedVariable*&);

    // get some result flags
    bool SubdivisionOccurred()   { return !allClean; }
    bool NotAllCellsSubdivided() { return someClean; }

  protected:
    bool           allClean;
    bool           someClean;

    MIROptions     options;
};

#endif
