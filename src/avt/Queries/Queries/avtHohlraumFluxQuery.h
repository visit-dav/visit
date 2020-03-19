// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtHohlraumFluxQuery.h                        //
// ************************************************************************* //

#ifndef AVT_HOHLRAUM_FLUX_QUERY_H
#define AVT_HOHLRAUM_FLUX_QUERY_H

#include <query_exports.h>
#include <avtLineScanQuery.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtHohlraumFluxQuery
//
//  Purpose:
//    This query calculates flux from a hohlraum capsule by integrating 
//    random rays, accumulating radiation bin by bin.  It may be extended
//    to be applied to one or more response functions, to match detectors
//    installed in NIF.  
//
//  Programmer: David Bremer
//  Creation:   November 17, 2006
//
//  Modifications:
//    Brad Whitlock, Fri Apr 17 09:38:35 PDT 2009
//    I added a PreExecute method.
//
//    Eric Brugger, Fri May  8 08:57:33 PDT 2009
//    I added a flag which has the query optionally use the emissivity divided
//    by the absorbtivity in place of the emissivity.
//
//    Kathleen Biagas, Mon Jun 20 09:05:48 PDT 2011
//    Added SetInputParams, changed args to SetRayCenter and SetThetaPhi to
//    reflect how information is stored in the input params MapNode.
//
//    Kathleen Biagas, Fri Jul 15 15:54:27 PDT 2011
//    Added GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtHohlraumFluxQuery : public avtLineScanQuery
{
  public:
                              avtHohlraumFluxQuery();
    virtual                  ~avtHohlraumFluxQuery();

    virtual const char       *GetType(void) 
                                 { return "avtHohlraumFluxQuery"; }
    virtual const char       *GetDescription(void)
                                 { return "Calculating Flux For a Hohlraum"; }

    virtual void              SetInputParams(const MapNode &);
    static  void              GetDefaultInputParams(MapNode &);

    void                      SetVariableNames(const stringVector &names);
    void                      SetRayCenter(const doubleVector &);
    void                      SetRayCenter(const intVector &);
    void                      SetRadius(float r);
    void                      SetTheta(const double &);
    void                      SetPhi(const double &);
    void                      SetDivideEmisByAbsorb(bool flag);

  protected:
    float                     rayCenter[3];
    float                     radius;
    float                     theta, phi;
    bool                      divideEmisByAbsorb;

    std::string               absVarName;  //e.g. "absorbtivity"
    std::string               emisVarName; //e.g. "emissivity"
    double *                  radBins;
    std::vector<double>       binWidths;

    //FYI:  Inherits from avtLineScanQuery
    //int                       numBins;    //Used for radiation bins.
                                            //Number is obtained from the mesh, not set by the user
    //int                       numLines;   //Used just as in avtLineScanQuery
    //double                    minLength;  //Unused
    //double                    maxLength;  //Unused
    //int                       numLinesPerIteration;  //Used just as in avtLineScanQuery
    //std::string               varname;    //Unused
    virtual void              IntegrateLine(int oneSide, int otherSide,
                                            vtkPolyData *output,
                                            vtkIntArray *lineids,
                                            int lineid, double dir[3],
                                            vtkDataArray *absorbtivityBins,
                                            vtkDataArray *emissivityBins,
                                            double *tmpBins);
    virtual avtLineScanFilter *CreateLineScanFilter();
    virtual void              GetSecondaryVars( std::vector<std::string> &outVars );

  private:
    virtual void              ExecuteLineScan(vtkPolyData *pd);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
};


#endif
