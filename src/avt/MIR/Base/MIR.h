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
