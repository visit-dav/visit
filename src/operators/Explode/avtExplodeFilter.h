/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************* //
//  File: avtExplodeFilter.h
// ************************************************************************* //

#ifndef AVT_Explode_FILTER_H
#define AVT_Explode_FILTER_H

#include <avtPluginDataTreeIterator.h>
#include <avtSIMODataTreeIterator.h>
#include <avtPluginFilter.h>

#include <ExplodeAttributes.h>

#include <vtkUnstructuredGrid.h>
#include <vtkDataSet.h>

class Explosion;

// ****************************************************************************
//  Class: avtExplodeFilter
//
//  Purpose:
//      A plugin operator for Explode.
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Wed Jan 17 10:06:58 PST 2018
//      Changed inheritance from avtSIMODataTreeIterator to
//      avtDatasetToDatasetFilter for MPI use. Also added
//      globalMatExtents and GetMaterialIndex(). 
//
// ****************************************************************************

class avtExplodeFilter : public avtDatasetToDatasetFilter,
                         public virtual avtPluginFilter
{
  public:
                                   avtExplodeFilter();
    virtual                       ~avtExplodeFilter();
 
    static avtFilter              *Create();

    virtual const char            *GetType(void)  { return "avtExplodeFilter"; };
    virtual const char            *GetDescription(void) { return "Explode"; };

    virtual void                   SetAtts(const AttributeGroup*);
    virtual bool                   Equivalent(const AttributeGroup*);

    virtual bool                   ThreadSafe(void) { return(false); };


  protected:
    void                           Execute(void);

    virtual void                   PostExecute(void);
    virtual void                   PreExecute(void);

    virtual avtContract_p          ModifyContract(avtContract_p);
    virtual void                   UpdateDataObjectInfo(void);

    void                           UpdateGlobalExtents(double *, std::string);
    int                            GetMaterialIndex(std::string);
    avtDataTree_p                  GetMaterialSubsets(avtDataRepresentation *);
    avtDataTree_p                  ExtractMaterialsFromDomains(avtDataTree_p);

    ExplodeAttributes              atts;
    
    Explosion                     *explosion;
    double                        *globalMatExtents;
};


// ****************************************************************************
//  Class: Explosion
//
//  Purpose:
//      This is a virtual class meant to be used for inheritance when
//      creating different types of explosions. 
//      CalcDisplacement is the method that must be overriden by 
//      children, as it is the method that determines exactly how
//      to explode.  
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

class Explosion
{
  public:
                         Explosion();
    virtual             ~Explosion() {};
    virtual void         CalcDisplacement(double *, double, bool) = 0;
    void                 DisplaceMaterial(vtkUnstructuredGrid *, 
                                          double *);
    void                 ExplodeAllCells(vtkDataSet *,
                                         vtkUnstructuredGrid *);
    void                 ExplodeAndDisplaceMaterial(vtkUnstructuredGrid *, 
                                                    double *);

    //
    // Variables for all types
    //
    std::string          materialName;
    bool                 explodeMaterialCells;
    bool                 explodeAllCells;
    int                  explosionPattern;
    double               matExplosionFactor;
    double               cellExplosionFactor;
    double               displaceVec[3];

    //
    // Point variables
    //
    double              *explosionPoint;

    //
    // Plane variables
    //
    double              *planePoint;
    double              *planeNorm;

    //
    // Cylinder variables
    //
    double              *cylinderPoint1;
    double              *cylinderPoint2;
    double               cylinderRadius;
};


// ****************************************************************************
//  Class: PointExplosion
//
//  Purpose:
//      Class used in exploding from a point in space. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

class PointExplosion : virtual public Explosion
{
  public:
                      PointExplosion();
    virtual          ~PointExplosion() {};
    virtual void      CalcDisplacement(double *, double, bool);
};


// ****************************************************************************
//  Class: PlaneExplosion
//
//  Purpose:
//      Class used in exploding from a plane in space. 
//
//  Programmer: Alister Maguire 
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

class PlaneExplosion : virtual public Explosion
{
  public:
                      PlaneExplosion();
    virtual          ~PlaneExplosion() {};
    virtual void      CalcDisplacement(double *, double, bool);
};


// ****************************************************************************
//  Class: CylinderExplosion
//
//  Purpose:
//      Class used in exploding from a cylinder in space. 
//
//  Programmer: Alister Maguire 
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

class CylinderExplosion : virtual public Explosion
{
  public:
                      CylinderExplosion();
    virtual          ~CylinderExplosion() {};
    virtual void      CalcDisplacement(double *, double, bool);
};


#endif
