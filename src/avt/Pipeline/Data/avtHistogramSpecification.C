#include "avtHistogramSpecification.h"

 /** \brief Standard constructor.*/
avtHistogramSpecification::avtHistogramSpecification()
{
    valid = false;
    cm_BoundsSpecified = false;
    cm_Counts = NULL;
}

avtHistogramSpecification::~avtHistogramSpecification()
{
    if (cm_Counts)
    {
        delete[] cm_Counts;
        cm_Counts = NULL;
    }
}

 

/** \brief Specify a N-D histogram with regular binning
 *
 *  The bounds array will be initalized based on the the given numBins and
 *  begin and end parameters.
 *
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variables : List of variables the histogram should be created for
 *  \param numBins   : Number of bins
 *  \param begin     : Lower bound for the binning in each dimension.The order
 *                     of the values in this vector should be the same as in
 *                     the variables parameter.
 *  \param end       : Upper bound for the binning in each dimension.The order
 *                     of the values in this vector should be the same as in
 *                     the variables parameter.
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this.
 *  \return bool     : Is the specification valid. Here we only check for 
 *                     consistency of the specification not if e.g. the
 *                     timestep is valid for the current data or if the given
 *                     variables actually exist. This function will also return
 *                     false if specifyHistogram has been called previously.
 */
bool
avtHistogramSpecification::SpecifyHistogram(int                     timestep,
                                            const vector<string>   &variables,
                                            const vector<int>      &numBins,
                                            const vector<double>   &begin,
                                            const vector<double>   &end,
                                            const string           &condition ,
                                            bool                   exact )            
{
    // don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = true;
    cm_Timestep    = timestep;
    cm_Variables   = variables;
    cm_NumberOfBins= numBins;
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid          = variables.size()>0                &&
                     variables.size() == begin.size()  &&
                     variables.size() == end.size()    &&
                     variables.size() == numBins.size();
    cm_Bounds.resize(cm_Variables.size());
    for (unsigned int i=0 ; i<cm_NumberOfBins.size() ; ++i )
    {
        cm_Bounds[i].resize(cm_NumberOfBins[i]+1);
        for (int j=0; j<=cm_NumberOfBins[i]; j++)
        {
            cm_Bounds[i][j] = begin[i] +
                (end[i]-begin[i])*double(j)/double(cm_NumberOfBins[i]);
        }
    }
    SetBoundsSpecified();
    return valid;
}


/** \brief Specify a N-D histogram and bounds are unknown
 *
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variables : List of variables the histogram should be created for
 *  \param numBins   : Number of bins
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this
 *  \param regular   : Should the histogram be computed using a regular binning
 *                     or should an adaptive binning be used. Which type of 
 *                     adaptive binning is not specified.
 *  \return bool     : Is the specification valid. Here we only check for 
 *                     consistency of the specification not if e.g. the
 *                     timestep is valid for the current data or if the given
 *                     variables actually exist. This function will also return
 *                     false if specifyHistogram has been called previously.
 */
bool 
avtHistogramSpecification:: SpecifyHistogram(int timestep,
                                             const vector<string> &variables,
                                             const vector<int>    &numBins,
                                             const string         &condition,
                                             bool                 exact     ,  
                                             bool                 regular)
{
// don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = regular;
    cm_Timestep    = timestep;
    cm_Variables   = variables;
    cm_NumberOfBins= numBins;
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid          = variables.size()>0                &&
                     variables.size() == numBins.size();
    cm_Bounds.resize(cm_Variables.size());
    cm_BoundsSpecified = false;
    return valid;      
}

/** \brief Specify a 1-D histogram (convenience)
 *   
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variable0  : The name of the variable
 *  \param numBins0   : Number of bins
 *  \param begin0     : Lower bound for the binning
 *  \param end0       : Upper bound for the binning
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this
 *  \param regular   : Should the histogram be computed using a regular binning
 *                     or should an adaptive binning be used. Which type of 
 *                     adaptive binning is not specified.
 *  \return bool     : Is the specification valid. Here we only check for 
 *                     consistency of the specification not if e.g. the
 *                     timestep is valid for the current data or if the given
 *                     variables actually exist. This function will also return
 *                     false if specifyHistogram has been called previously.
 */
bool
avtHistogramSpecification::SpecifyHistogram(int           timestep,
                                            const string &variable0,
                                            int           numBins0,
                                            double        begin0,
                                            double        end0,
                                            const string &condition,
                                            bool          exact )
{
    // don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = true;
    cm_Timestep    = timestep;
    cm_Variables.push_back(variable0);
    cm_NumberOfBins.push_back(numBins0);
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid          = numBins0 > 0;
    cm_Bounds.resize(1);
    cm_Bounds[0].resize(numBins0 + 1);
    for (int j=0; j<=numBins0; j++)
    {
        cm_Bounds[0][j] = begin0 +
            (end0-begin0)*double(j)/double(numBins0);
    }
    SetBoundsSpecified();
    return valid;
}

/** \brief Specify a 1-D histogram with bounds unknown (convenience)
 *   
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variable0  : The name of the variable
 *  \param numBins0   : Number of bins
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this
 *  \param regular   : Should the histogram be computed using a regular binning
 *                     or should an adaptive binning be used. Which type of 
 *                     adaptive binning is not specified.
 *  \return bool     : Is the specification valid. Here we only check for 
 *                     consistency of the specification not if e.g. the
 *                     timestep is valid for the current data or if the given
 *                     variables actually exist. This function will also return
 *                     false if specifyHistogram has been called previously.
 */
bool 
avtHistogramSpecification::SpecifyHistogram(int           timestep,
                                            const string &variable0,
                                            int           numBins0,
                                            const string  &condition,
                                            bool          exact,
                                            bool          regular)
{
      // don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = regular;
    cm_Timestep    = timestep;
    cm_Variables.push_back(variable0);
    cm_NumberOfBins.push_back(numBins0);
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid          = numBins0 > 0;
    cm_Bounds.resize(cm_Variables.size());
    cm_BoundsSpecified = false;
    return valid;
}


/** \brief Specify a 2-D histogram (convenience)
 *   
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variable0  : The name of the first variable 
 *  \param numBins0   : Number of bins for the first variable
 *  \param begin0     : Lower bound for the binning of the first variable
 *  \param end0       : Upper bound for the binning of the first variable
 *  \param variable1  : The name of the second variable 
 *  \param numBins1   : Number of bins for the second variable
 *  \param begin1     : Lower bound for the binning of the second variable
 *  \param end1       : Upper bound for the binning of the second variable
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this
 *  \param regular   : Should the histogram be computed using a regular binning
 *                     or should an adaptive binning be used. Which type of 
 *                     adaptive binning is not specified.
 *  \return bool     : Is the specification valid. Here we only check for 
 *                     consistency of the specification not if e.g. the
 *                     timestep is valid for the current data or if the given
 *                     variables actually exist. This function will also return
 *                     false if specifyHistogram has been called previously.
 */
bool
avtHistogramSpecification::SpecifyHistogram(int           timestep,
                                            const string &variable0,
                                            int           numBins0,
                                            double        begin0,
                                            double        end0,
                                            const string &variable1,
                                            int           numBins1,
                                            double        begin1,
                                            double        end1,
                                            const string &condition,
                                            bool          exact )
{
    // don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = true;
    cm_Timestep    = timestep;
    cm_Variables.push_back(variable0);
    cm_Variables.push_back(variable1);
    cm_NumberOfBins.push_back(numBins0);
    cm_NumberOfBins.push_back(numBins1);
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid          = numBins0 > 0 &&
                     numBins1 > 0;
    cm_Bounds.resize(2);
    cm_Bounds[0].resize(numBins0 + 1);
    cm_Bounds[1].resize(numBins1 + 1);
    for (int j=0; j<=numBins0; j++)
    {
        cm_Bounds[0][j] = begin0 +
            (end0-begin0)*double(j)/double(numBins0);
    }
    for (int j=0; j<=numBins1; j++)
    {
        cm_Bounds[1][j] = begin1 +
            (end1-begin1)*double(j)/double(numBins1);
    }
    SetBoundsSpecified();
    return valid;
}

/** \brief Specify a 2-D histogram. Bounds unkown (convenience)
 *   
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variable0  : The name of the first variable 
 *  \param numBins0   : Number of bins for the first variable
 *  \param variable1  : The name of the second variable 
 *  \param numBins1   : Number of bins for the second variable
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this
 *  \param regular   : Should the histogram be computed using a regular binning
 *                     or should an adaptive binning be used. Which type of 
 *                     adaptive binning is not specified.
 *  \return bool     : Is the specification valid. Here we only check for 
 *                     consistency of the specification not if e.g. the
 *                     timestep is valid for the current data or if the given
 *                     variables actually exist. This function will also return
 *                     false if specifyHistogram has been called previously.
 */
bool
avtHistogramSpecification::SpecifyHistogram(int timestep,
                                            const string &variable0,
                                            int           numBins0,
                                            const string &variable1,
                                            int           numBins1,
                                            const string  &condition,
                                            bool          exact,
                                            bool          regular)
{
    // don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = regular;
    cm_Timestep    = timestep;
    cm_Variables.push_back(variable0);
    cm_Variables.push_back(variable1);
    cm_NumberOfBins.push_back(numBins0);
    cm_NumberOfBins.push_back(numBins1);
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid          = numBins0 > 0 &&
                     numBins1 > 0;
    cm_BoundsSpecified = false;
    cm_Bounds.resize(cm_Variables.size());
    return valid;
}


/** \brief Specify a request with predefined bounds
 *
 *  \param timestep  : Timestep the histogram should be created for.         
 *  \param variables : List of variables the histogram should be created for
 *  \param bounds    : The bounds to be used in the binning for each dimension.
 *                     [varIndex][boundIndex]
 *  \param condition : Condition to be used e.g. "px>0. If an empty string is
 *                     given then the histogram should be created for all data
 *                     records of the given timestep
 *  \param exact     : Is the condition exact or can it be modified if 
 *                     performance can be increased by this
 */
bool
avtHistogramSpecification::SpecifyHistogram(int                      timestep,
                                      const vector<string>          &variables,
                                      const vector<vector<double> > &bounds,
                                      const string                  &condition,
                                      bool                          exact )
{
    // don't initialize more than once
    if (valid)
        return false;

    cm_RegularBinning = false;
    cm_Timestep    = timestep;
    cm_Variables   = variables;
    cm_Bounds      = bounds;    
    cm_Condition   = condition;
    cm_ConditionExact = exact;
    valid     = variables.size()>0              &&
                variables.size()==bounds.size();
    for (size_t i=0 ; i<bounds.size() ; ++i )
    {
        valid = valid && bounds[i].size() > 1;
        cm_NumberOfBins.push_back(bounds[i].size() - 1);        
    }
    SetBoundsSpecified();
    return valid;
}


/****************************************************************************
 * Functions to ask for the specification of the histogram 
 ***************************************************************************/
/** \brief Ask for the timestep.
 * 
 *  \return int64_t& :
 */    
int
avtHistogramSpecification::GetTimestep() const
{
    return cm_Timestep;
}

/** \brief Ask if the specified bounds define a regular binning or if 
 *         no bounds are specified if the histogram should be computed
 *         using a regular binning.
 *  \return bool : Is binning regular
 */
bool
avtHistogramSpecification::IsRegularBinning() const
{
    return cm_RegularBinning;
}

/** \brief Ask for the used variables
 *  \return const vector<string>& : Vector of all variables
 */
const vector<string>&
avtHistogramSpecification::GetVariables() const
{
    return cm_Variables;
}

/** \brief Ask for the number of bins to be used in each dimension
 *
 *  This parameter is only valid if ev_RegularBinning is used.
 *  Use numberOfBinsValid() to check for validity. 
 * \return const vector<unsigned int>& : The number of bins. 
 *              The order of the values in this vector should be the same 
 *              as in the cm_Variables ( getVariables() ) vector.
 */
const vector<int>&
avtHistogramSpecification::GetNumberOfBins() const
{
    return cm_NumberOfBins;
}    

   
/** \brief Ask for the condition
 *  
 *  If an empty string is given then no condition is applied, i.e. all data
 *  should be used. Use the conditionUsed() function to check if a condition
 *  is used. 
 *  \return string : the condition
 */
string
avtHistogramSpecification::GetCondition() const
{
    return cm_Condition;
}

/** \brief Ask whether the condition is exact
 *
 *  If eact is false then the condition can be slightly modified in case
 *  that performance is increased by doing this. 
 *  \return bool : Condition exact?
 */
bool   
avtHistogramSpecification::GetConditionExact() const
{
      return cm_ConditionExact;
}

/****************************************************************************
 * Get methods for counts and bounds that specify the actual histogram 
 ***************************************************************************/

/** \brief Get the bounds of the bins
 *
 *  Since a reference to the vector is given one can use this function also 
 *  to actually specify this information directly when computing the histogram
 *  in order to avoid copying of the data afterward.
 *  \return vector< vector<double> >& : The bounds for each dimension.
 *              The order of the values in this vector should be the same 
 *              as in the cm_Variables ( getVariables() ) vector.
 */
vector< vector<double> >&
avtHistogramSpecification::GetBounds()
{
    return cm_Bounds;
}


/** \brief Ask for the counts, i.e. the actual histogram values of the bins
 *
 * Since a reference to the vector is given one can use this function also 
 *  to actually specify this information directly when computing the histogram
 *  in order to avoid copying of the data afterward.
 *  \return vector<unsigned int>&  : The counts per bin
 */
VISIT_LONG_LONG*
avtHistogramSpecification::GetCounts()
{
    if (cm_Counts == NULL)
    {
        int n = GetTotalNumberOfBins();
        cm_Counts = new VISIT_LONG_LONG[n];
        for (int i=0; i<n; i++)
            cm_Counts[i] = 0;
    }
    return cm_Counts;
}

/** \brief Define the values of the bins of the histogram. 
 * 
 *  Use getCounts() and modify the counts vector directly if the information
 *  should be stored here directly to avoid copying. Use setCountsAndBounds()
 *  instead if also the bounds are specified by, e.g., FastBit and are not 
 *  given as input to the function that computes the histogram.
 *  \param counts : The histogram values
 */
void
avtHistogramSpecification::SetCounts(VISIT_LONG_LONG *counts)
{
    cm_Counts = counts;
}

void                      
avtHistogramSpecification::SetCounts(const vector<unsigned int>& counts )
{
    if( cm_Counts != NULL)
        delete[] cm_Counts;
    cm_Counts = new VISIT_LONG_LONG[ counts.size() ];
    for (size_t i=0; i<counts.size(); i++)
        cm_Counts[i] = counts[i];

    // Prabhat-- take this out after debugging
    /*std::cerr<<"HistSpec::SetCounts called, set cm_Counts.size()="<<counts.size()<<std::endl;
    for (int i=0; i<counts.size(); i++)
        if (cm_Counts[i]>0) {
            std::cerr<<"\t found non-zero entries in cm_Counts array" << std::endl;
            break;
        }*/
}

/****************************************************************************
 * Additional functions provided for convenience 
 ***************************************************************************/

/** \brief Ask for the dimensionality of the histogram
 *  \return unsigned int : Dimensionality
 */
int
avtHistogramSpecification::GetDimensionality() const
{
    return cm_Variables.size();
}

/** \brief Ask for the total number of bins/
 *
 *   This function is useful to quickly determine the length
 *   of the counts array (see GetCounts() ).
 *
 *  \return int : Total number of bins. Also the size of the cm_Counts array
 */
int
avtHistogramSpecification::GetTotalNumberOfBins() const
{
    if (!valid)
        return 0;

    int count = cm_NumberOfBins[0];
    for (size_t i=1; i<cm_NumberOfBins.size(); i++)
        count *= cm_NumberOfBins[i];
    return count;
}

/** \brief Is a condition used?
 *
 *  If no condition is used then all data is used for creation of the histogram
 *  \return bool : condition used 
 */
bool
avtHistogramSpecification::HasCondition() const
{
    return !cm_Condition.empty();
}

/** \brief Have the counts and bounds been set for the histogram
 *
 *  \return bool : data available
 */
bool
avtHistogramSpecification::IsValid() const
{
    return valid;
}

/** \brief Have the bounds been specified
 *  \return bool : Bounds specified
 */
bool
avtHistogramSpecification::BoundsSpecified() const 
{
      return cm_BoundsSpecified;
}

/** \brief Define that the bounds have now been specified
 */
void 
avtHistogramSpecification::SetBoundsSpecified()
{
      cm_BoundsSpecified = true;
      //TODO remove later
      for( size_t i=0 ; i<cm_NumberOfBins.size() ; i++){
            cm_NumberOfBins[i] = cm_Bounds[i].size()-1;
      }
}

/** \brief Copy the information of the given inpute specification to this object
 *
 *  Unlike the standard copy constructor this will make a deep copy of all
 *  members of a histogram specification
 *
 *  \param input : The histogram specification to be copied
 */
void 
avtHistogramSpecification::copyInfo( avtHistogramSpecification* input)
{
      valid = input->IsValid();
      cm_Timestep = input->GetTimestep();
      cm_Variables = input->GetVariables();
      cm_Condition = input->GetCondition();
      cm_ConditionExact = input->GetConditionExact();
      cm_NumberOfBins = input->GetNumberOfBins();
      cm_RegularBinning = input->IsRegularBinning();
      cm_Bounds = input->GetBounds();
      cm_BoundsSpecified=input->BoundsSpecified();
      delete[] cm_Counts;
      cm_Counts = NULL;
      int numBins = input->GetTotalNumberOfBins();
      cm_Counts = new VISIT_LONG_LONG[ numBins  ];
      VISIT_LONG_LONG* inputCounts = input->GetCounts();
      for(int i=0; i<numBins; ++i){
            cm_Counts[i] = inputCounts[i];
      }
}


/** \brief Print data members
 *
 */
void
avtHistogramSpecification::Print(ostream &out) const
{
    out << "Valid = " << (valid ? "true" : "false") << endl;
    out << "Time step = " << cm_Timestep << endl;
    out << "Condition string " << cm_Condition << endl;
    out << "Condition exact " << cm_ConditionExact << endl;
    out << "Regular binning " << cm_RegularBinning << endl;
    out << "Bounds specified " << cm_BoundsSpecified << endl;
    for (size_t i = 0 ; i < cm_Variables.size() ; i++)
    {
        out << "Var = " << cm_Variables[i] << endl;
        out << "\tnumbins = " << cm_NumberOfBins[i] << endl;
        out << "\tLow = " << cm_Bounds[i][0] 
            << ", Hi = " << cm_Bounds[i][cm_Bounds[i].size()-1] << endl;
    }
    if (!valid || !cm_Counts)
    {
        out << "No valid counts\n";
    }
    else
    {
        int n = GetTotalNumberOfBins();
        int first_bin;
        for (first_bin=0; first_bin<n; first_bin++)
        {
            if (cm_Counts[first_bin] > 0)
                break;
        }
        if (first_bin == n)
        {
            out << "all bins were zero\n";
        }
        else
        {
            out << "first nonzero bin was index "<<first_bin
                << " with value "<<cm_Counts[first_bin] << endl;
        }
    }
}

#ifdef PARALLEL
#include <avtParallel.h>
#include <mpi.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Method:  avtHistogramSpecification::GetToRootProcessor
//
//  Purpose:
//    Get this object to the root processor.  I.e. if we're the
//    root processor, receive it.  If not, then send it.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 26, 2008
//
//  Modifications:
//    Mark C. Miller, Tue Jan 27 18:34:41 PST 2009
//    MPI_Type_get_extent is only in MPI-2. Likewise for
//    MPI_UNSIGNED_LONG_LONG. So, I made the first check conditional
//    on MPI_UNSIGNED_LONG_LONG being defined.
//
//    Gunther H. Weber, Mon Apr  6 20:50:50 PDT 2009
//    Check whether MPI_INTEGER8 defined even if MPI_UNSIGNED_LONG_LONG
//    is defined.
//    
// ****************************************************************************
bool
avtHistogramSpecification::GetToRootProcessor(int tag)
{
    // Find a long-long data type for the counts array
    MPI_Datatype datatype = MPI_LONG_LONG;
    MPI_Aint lb,e;
    // On at least one mpi implementation (mpich2-1.0.5, Linux-x86-64),
    // MPI_LONG_LONG blatantly fails.  But for some reason INTEGER8 works.
    // Luckily we can tell this by checking the datatype size of the type.
    // We'll try a few different ones, and if none work, just do it slowly
    // using a single-precision int.
#if defined(MPI_UNSIGNED_LONG_LONG)
    MPI_Type_get_extent(datatype, &lb, &e);
    if (e != sizeof(VISIT_LONG_LONG))
    {
        datatype = MPI_UNSIGNED_LONG_LONG;
        MPI_Type_get_extent(datatype, &lb, &e);
    }
#endif
#if defined(MPI_INTEGER8)  // ... may only be MPI-2.
    if (e != sizeof(VISIT_LONG_LONG))
    {
        datatype = MPI_INTEGER8;
        MPI_Type_get_extent(datatype, &lb, &e);
    }
#endif

    // Do the matching send/receives
    int rank = PAR_Rank();
    MPI_Status stat;
    if (rank==0)
    {
        //cout << "---  BEFORE ---\n";
        //this->Print(cout);

        int len;
        char buff[10000];

        int tmp_valid;
        MPI_Recv(&tmp_valid, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        valid = tmp_valid;

        if (!valid)
            return false;

        int dim;
        MPI_Recv(&dim, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);

        MPI_Recv(&cm_Timestep, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);

        cm_Variables.resize(dim);
        for (int i=0; i<dim; i++)
        {
            MPI_Recv(&len, 1, MPI_INT,
                     MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
            MPI_Recv(buff, len, MPI_CHAR,
                     MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
            buff[len] = '\0';
            cm_Variables[i] = buff;
        }

        MPI_Recv(&len, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        MPI_Recv(buff, len, MPI_CHAR,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        buff[len] = '\0';
        cm_Condition = buff;

        int tmp_ce;
        MPI_Recv(&tmp_ce, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        cm_ConditionExact = tmp_ce;


        cm_NumberOfBins.resize(dim);
        MPI_Recv(&(cm_NumberOfBins[0]), dim, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);



        int tmp_rb;
        MPI_Recv(&tmp_rb, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        cm_RegularBinning = tmp_rb;

        cm_Bounds.resize(dim);
        for (int i=0; i<dim; i++)
        {
            len = cm_NumberOfBins[i]+1;
            cm_Bounds[i].resize(len);
            MPI_Recv(&(cm_Bounds[i][0]), len, MPI_DOUBLE,
                     MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        }


        int tmp_bs;
        MPI_Recv(&tmp_bs, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
        cm_BoundsSpecified = tmp_bs;


        if (cm_Counts)
            delete[] cm_Counts;
        cm_Counts = NULL;

        int counts_valid;
        MPI_Recv(&counts_valid, 1, MPI_INT,
                 MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);


        if (counts_valid)
        {
            len = GetTotalNumberOfBins();
            cm_Counts = new VISIT_LONG_LONG[len];
            if (e == sizeof(VISIT_LONG_LONG))
            {
                MPI_Recv(cm_Counts, len, datatype,
                         MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
            }
            else
            {
                // This is pathetic, but I don't have a better idea.
                int *tmpArray = new int[len];
                MPI_Recv(tmpArray, len, MPI_INT,
                         MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &stat);
                for (int i=0; i<len; i++)
                    cm_Counts[i] = tmpArray[i];
                delete[] tmpArray;
            }
        }

        //cout << "---  AFTER ---\n";
        //this->Print(cout);
    }
    else
    {
        //cout << "---  RANK "<<rank<<" SENDING ---\n";
        //this->Print(cout);

        int len;
        int tmp_valid = valid;
        MPI_Send(&tmp_valid, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        if (!valid)
            return false;

        int dim = cm_Variables.size();
        MPI_Send(&dim, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        MPI_Send(&cm_Timestep, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        for (int i=0; i<dim; i++)
        {
            len = cm_Variables[i].length();
            MPI_Send(&len, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);
            MPI_Send((void*)(cm_Variables[i].c_str()),
                     len, MPI_CHAR, 0, tag, VISIT_MPI_COMM);
        }

        len = cm_Condition.length();
        MPI_Send(&len, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);
        MPI_Send((void*)(cm_Condition.c_str()),
                 len, MPI_CHAR, 0, tag, VISIT_MPI_COMM);

        int tmp_ce = cm_ConditionExact;
        MPI_Send(&tmp_ce, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        MPI_Send(&(cm_NumberOfBins[0]), dim, MPI_INT, 0, tag, VISIT_MPI_COMM);

        int tmp_rb = cm_RegularBinning;
        MPI_Send(&tmp_rb, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        for (int i=0; i<dim; i++)
        {
            len = cm_NumberOfBins[i]+1;
            MPI_Send(&(cm_Bounds[i][0]), len, MPI_DOUBLE,
                     0, tag, VISIT_MPI_COMM);
        }
        
        int tmp_bs = cm_BoundsSpecified;
        MPI_Send(&tmp_bs, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        int counts_valid = (cm_Counts != NULL);
        MPI_Send(&counts_valid, 1, MPI_INT, 0, tag, VISIT_MPI_COMM);

        if (counts_valid)
        {
            len = GetTotalNumberOfBins();
            if (e == sizeof(VISIT_LONG_LONG))
            {
                MPI_Send(cm_Counts, len, datatype, 0, tag, VISIT_MPI_COMM);
            }
            else
            {
                int *tmpArray = new int[len];
                for (int i=0; i<len; i++)
                    tmpArray[i] = cm_Counts[i];
                MPI_Send(tmpArray, len, MPI_INT, 0, tag, VISIT_MPI_COMM);
                delete[] tmpArray;
            }
        }        
    }


    return true;
}

// ****************************************************************************
//  Method:  avtHistogramSpecification::SumAcrossAllProcessors
//
//  Purpose:
//    Sum the counts array across all processors.  Return failure
//    if the lengths don't all match.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  7, 2008
//
// ****************************************************************************
bool
avtHistogramSpecification::SumAcrossAllProcessors()
{
    // Make sure all processors agree on the size
    int length = GetTotalNumberOfBins();
    double minMaxLength[2] = {length, length};
    UnifyMinMax(minMaxLength, 2);
    if (minMaxLength[0] != minMaxLength[1])
        return false;
    
    VISIT_LONG_LONG *oldcounts = GetCounts();
    VISIT_LONG_LONG *newcounts = new VISIT_LONG_LONG[length];
    SumLongLongArrayAcrossAllProcessors(oldcounts, newcounts, length);
    cm_Counts = newcounts;
    delete[] oldcounts;

    return true;
}

#endif
