// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtExplodeFilter.h
// ************************************************************************* //

#ifndef AVT_Explode_FILTER_H
#define AVT_Explode_FILTER_H

#include <avtDatasetToDatasetFilter.h>
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
//      Alister Maguire, Mon Jan 22 16:16:27 PST 2018
//      Added scaleFactor. 
//
//      Alister Maguire, Mon Jan 29 10:12:44 PST 2018
//      Added MergeDomains().
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed MergeDomains to CreateDomainTree, made
//      explosions a double pointer, and added numExplosions. 
//
//      Alister Maguire, Wed Feb 14 14:36:02 PST 2018
//      Added datasetExtents, hasMaterials, onlyCellExp, 
//      ResetMaterialExtents, ComputeScaleFactor,
//      and changed globalMatExtents to materialExtents. 
//
//      Alister Maguire, Tue Sep 25 11:21:25 PDT 2018
//      Added oneSubPerDomain to flag when we have one
//      subset per domain. 
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

    void                           UpdateExtentsAcrossDomains(double *, std::string);
    void                           UpdateExtentsAcrossProcs();
    int                            GetMaterialIndex(std::string);
    avtDataTree_p                  GetMaterialSubsets(avtDataRepresentation *);
    avtDataTree_p                  ExtractMaterialsFromDomains(avtDataTree_p);
    avtDataTree_p                  CreateDomainTree(vtkDataSet **, 
                                                    int,
                                                    std::vector<int>, 
                                                    stringVector);
    void                           ResetMaterialExtents(bool, int matIdx=-1);

    ExplodeAttributes              atts;
    Explosion                    **explosions;
    double                        *materialExtents;
    double                         scaleFactor;
    int                            numExplosions;
    bool                           hasMaterials;
    bool                           onlyCellExp;
    bool                           oneSubPerDomain;
    
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
//  Modifications:
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Moved variables specific to explosion types from
//      this base class to the children and added 
//      ScaleExplosion(). 
//
//      Alister Maguire, Tue May  1 16:19:18 PDT 2018
//      Added recenter argument to CalcDisplacement. 
//
// ****************************************************************************

class Explosion
{
  public:
                         Explosion();
    virtual             ~Explosion() {};

    virtual void         CalcDisplacement(double *dataCenter, 
                                          double  expFactor, 
                                          double  scaleFactor, 
                                          bool    normalize, 
                                          bool    recenter = false) = 0;

    void                 DisplaceMaterial(vtkUnstructuredGrid *, 
                                          double *, double);
    void                 ExplodeAllCells(vtkDataSet *,
                                         vtkUnstructuredGrid *, 
                                         double);
    void                 ExplodeAndDisplaceMaterial(vtkUnstructuredGrid *, 
                                                    double *, double);
    void                 ScaleExplosion(double, double, bool);

    std::string          materialName;
    bool                 explodeMaterialCells;
    bool                 explodeAllCells;
    int                  explosionPattern;
    double               matExplosionFactor;
    double               cellExplosionFactor;
    double               displaceVec[3];
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
//  Modifications:
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Added explosionPoint. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed pointers to lists. 
//
//      Alister Maguire, Tue May  1 16:19:18 PDT 2018
//      Added recenter argument to CalcDisplacement. 
//
// ****************************************************************************

class PointExplosion : virtual public Explosion
{
  public:
                      PointExplosion();
    virtual          ~PointExplosion() {};
    virtual void      CalcDisplacement(double *dataCenter, 
                                       double  expFactor, 
                                       double  scaleFactor, 
                                       bool    normalize, 
                                       bool    recenter = false);

    double            explosionPoint[3];
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
//  Modifications:
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Added planePoint and planeNorm.
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed pointers to lists. 
//
//      Alister Maguire, Tue May  1 16:19:18 PDT 2018
//      Added recenter argument to CalcDisplacement. 
//
// ****************************************************************************

class PlaneExplosion : virtual public Explosion
{
  public:
                      PlaneExplosion();
    virtual          ~PlaneExplosion() {};
    virtual void      CalcDisplacement(double *dataCenter, 
                                       double  expFactor, 
                                       double  scaleFactor, 
                                       bool    normalize, 
                                       bool    recenter = false);

    double            planePoint[3];
    double            planeNorm[3];
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
//  Modifications:
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Added cylinderPoint1, cylinderPoint2, and cylinderRadius. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed pointers to lists. 
//
//      Alister Maguire, Tue May  1 16:19:18 PDT 2018
//      Added recenter argument to CalcDisplacement. 
//
// ****************************************************************************

class CylinderExplosion : virtual public Explosion
{
  public:
                      CylinderExplosion();
    virtual          ~CylinderExplosion() {};
    virtual void      CalcDisplacement(double *dataCenter, 
                                       double  expFactor, 
                                       double  scaleFactor, 
                                       bool    normalize,
                                       bool    recenter = false);

    double            cylinderPoint1[3];
    double            cylinderPoint2[3];
    double            cylinderRadius;
};


#endif
