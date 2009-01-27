#ifndef AVT_HISTOGRAM_SPECIFICATION_H
#define AVT_HISTOGRAM_SPECIFICATION_H

#include <pipeline_exports.h>

#include <vector>
#include <string>
#include <visitstream.h>

using std::vector;
using std::string;

// ****************************************************************************
//  Class:  avtHistogramSpecification
//
//  Purpose:
//    Define and store a n-dimensional histogram
//
//  Programmer:  Oliver Rübel and Jeremy Meredith
//  Creation:    February 22, 2008
//
//  Modifications:
//
//    Hank Childs, Mon Mar  3 19:25:04 PST 2008
//    Add Print function.
//
//    Hank Childs, Wed Mar  5 20:24:41 PST 2008
//    Replaced nonportable uint32_t with unsigned int.
//
//    Jeremy Meredith, Fri Mar  7 14:30:46 EST 2008
//    Add some parallel support functions.
//
// ****************************************************************************

// The basic specification of a histogram can be defined using the 
// different specifyHistogram(...) functions. The different specifyHistogram(...)
// functions are used to specify histograms using different types of binning. 
// (see e_Binning for details). In case that one wants to specify everything
// manually, according set functions are provided. In contrast to 
// specifyHistogram(...) no sanity checks are performed when using the set
// functions directly.

// The actual data of the histogram, i.e. the values of the different bins 
// (and also the bounds of the bins in case that this was not specified in
// specifyHistogram) will be defined later on using  the setCounts() and/or
// the setBounds() function respectively. If copying of this information
// should be avoided then the getCounts() and getBounds() functions can be
// used to get a reference to the according variables which can then be
// modified directly.

// The binning of the histogram can either be specified by the file-reader
// ( ev_AutoBinning ) or by defining the number of bins for each variable
// ( ev_RegularBinning ). Alternatively a stride (plus according begin and
// end parameters) can be used to define the binning ( ev_StrideBinning )
// or bin boundaries can be defined manually ( ev_CustomBinning ). 

// The following parameters are mandatory: cm_Binning , cm_Timestep , 
// cm_Variables , and cm_Condition. cm_Condition can be an empty string
// in case that all data records should be considered in the histogram.
// Depending on which binning is used the parameters cm_NumberOfBins and
// cm_Begin , cm_End, and cm_Stride are specified in addition. 

// The actual histogram is then specified in cm_Bounds (the bounds of each bin)
// and cm_Counts (the actual histogram value of each bin).

class PIPELINE_API avtHistogramSpecification {
  public:

     avtHistogramSpecification();
    ~avtHistogramSpecification();
    
    void copyInfo( avtHistogramSpecification* input);

       //
    // Functions to define the basic specification of the histogram
    //

    // N-D, regular
    bool SpecifyHistogram(int                           timestep,
                          const vector<string>          &variables,
                          const vector<int>             &numBins,
                          const vector<double>          &begin,
                          const vector<double>          &end,
                          const string                  &condition = string(),
                          bool                          exact = true);

    // N-D, irregular
    bool SpecifyHistogram(int                            timestep,
                          const vector<string>          &variables,
                          const vector<vector<double> > &bounds,
                          const string                  &condition = string(),
                          bool                          exact = true);
    
    // N-D histogram. The number of bins are know but the bounds are unknown.
    bool SpecifyHistogram(int                           timestep,
                          const vector<string>          &variables,
                          const vector<int>             &numBins,
                          const string                  &condition = string(),
                          bool                          exact = true ,
                          bool                          regular = false);
     

    // 1-D, regular (convenience)
    bool SpecifyHistogram(int                            timestep,
                          const string                  &variable0,
                          int                            numBins0,
                          double                         begin0,
                          double                         end0,
                          const string                  &condition = string(),
                          bool                          exact = true);
    
    // 1-D, bounds unknown (convenience)
    bool SpecifyHistogram(int                            timestep,
                          const string                  &variable0,
                          int                            numBins0,
                          const string                  &condition = string(),
                          bool                          exact = true,
                          bool                          regular = false);

    // 2-D, regular (convenience)
    bool SpecifyHistogram(int                            timestep,
                          const string                  &variable0,
                          int                            numBins0,
                          double                         begin0,
                          double                         end0,
                          const string                  &variable1,
                          int                            numBins1,
                          double                         begin1,
                          double                         end1,
                          const string                  &condition = string(),
                          bool                          exact = true);
    
    //2-D, bounds unkown (convenience)
    bool SpecifyHistogram(int                            timestep,
                          const string                  &variable0,
                          int                            numBins0,
                          const string                  &variable1,
                          int                            numBins1,
                          const string                  &condition = string(),
                          bool                          exact = true,
                          bool                          regular=false);

        
    // Functions to ask for the specification of the histogram
    int                       GetTimestep()       const;
    bool                      IsRegularBinning()  const;
    const vector<string>     &GetVariables()      const;
    const vector<int>        &GetNumberOfBins()   const;
    string                    GetCondition()      const;
    bool                      GetConditionExact() const;
    
    // Functions to ask for the actual histogram data
    vector< vector<double> > &GetBounds();
    VISIT_LONG_LONG          *GetCounts();

    // Function to set the actual histogram data
    void                      SetCounts(VISIT_LONG_LONG *counts);
    void                      SetCounts(const vector<unsigned int>& counts );
    
    // Additional functions provided for convenience
    int  GetDimensionality()      const;
    int  GetTotalNumberOfBins()   const;
    bool HasCondition()           const;
    bool IsValid()                const;
    bool BoundsSpecified()        const;
    void SetBoundsSpecified();
    
    void Print(ostream &)         const;

#ifdef PARALLEL
    bool SumAcrossAllProcessors();
    bool GetToRootProcessor(int tag);
#endif

private:
    // True if it has been initialized successfully
    bool  valid;

    // Define the timestep for the histogram
    int   cm_Timestep;
    
    // The dimensionality of the histogram is defined by cm_Variables.size()
    vector<string> cm_Variables;
    
    // Define the condition for the histogram
    string cm_Condition;
    
    // Define whether the conditions are exact of if FastBit can modify it
    // to comply with FastBit bin boundaries
    bool cm_ConditionExact;

    // Define the number of bins for each histogram
    vector<int> cm_NumberOfBins;

    // True if the binning is known to be regular
    bool cm_RegularBinning;
    
    //
    // The actual histogram data
    //

    // Define the bounds of the bins of the histogram
    vector< vector<double> > cm_Bounds;    
    
    //Are the bounds specified
    bool cm_BoundsSpecified;
    
    // Define the actual value of each bin of the histogram
    VISIT_LONG_LONG *cm_Counts;
  
};

#endif
