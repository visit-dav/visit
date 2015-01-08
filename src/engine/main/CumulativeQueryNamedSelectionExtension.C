/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <CumulativeQueryNamedSelectionExtension.h>

#include <avtThresholdFilter.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>
#include <SelectionVariableSummary.h>

#include <DebugStream.h>
#include <StackTimer.h>

#include <algorithm>
#include <map>
#include <avtDataTreeIterator.h>
#include <avtExtents.h>
#include <avtDatasetExaminer.h>
#include <avtHistogramSpecification.h>
#include <avtMetaData.h>
#include <avtNamedSelectionManager.h>
#include <avtParallel.h>
#include <avtTimeLoopCollectorFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedIntArray.h>

//
// Brad Whitlock, Fri May 20 14:48:44 PDT 2011
//
// Implementation notes:
//    In this form, cumulative queries are implemented as 3 extra filters:
//
//    CQHistogramCalculationFilter
//         A filter that calculates histograms for the CQ's input variables.
//
//    avtThresholdFilter
//        Threshold away all of the cells that do not match the limits for 
//        the variables set in the CQ properties. Using this filter may
//        allow the database to use selections to fulfill the threshold
//        operation. That would be good.
//
//    CQFilter
//        This is a time loop filter that gets the thresholded output for all 
//        time steps. The data that we get for each time step is fed into a
//        CumulativeQuery object that does the real work of creating the 
//        selection.
//
// ----------------------------------------------------------------------------
//
//    CumulativeQuery
//        This class examines datasets and counts the number of cells in each
//        time step using the count to then figure out the list of cells that exist
//        in any or all time steps, depending on the summation rule. We then take
//        that set of cells and sort it based on the desired histogramming
//        method and then we select a range of bins from the histogram to
//        contribute cells to our final selection. The final dataset produced
//        by the filter is a dummy dataset that contains the original cell numbers
//        that describe the cells in the selection. That array is used later in
//        named selection to create the selection.
//
//        This filter also computes an average frequency value for the cells in
//        each histogram bin. This is saved and returned to the gui.
//
//    CumulativeQueryNamedSelectionExtension
//        This class implements the GetSelection method to return a vector of
//        ints containing the doms/zones that identify the cells that match the
//        cumulative query selection criteria. The class is smart enough to build
//        a pipeline that executes our 3 filters when it needs to. Alternatively,
//        the class can use intermediate cached data to prevent the time iteration
//        portion of CQ from being done again.
//   

// ****************************************************************************
// Class: CQHistogramCalculationFilter
//
// Purpose:
//   This class is a specialized filter that computes histograms on specific
//   variables. We cache the histograms and make them available to the named
//   selection extension object that creates this filter.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 14:50:16 PST 2010
//
// Modifications:
//     Eric Brugger, Wed Aug 20 16:13:09 PDT 2014
//     Modified the class to work with avtDataRepresentation.
//   
// ****************************************************************************

class CQHistogramCalculationFilter : public avtDataTreeIterator
{
public:
    struct Histogram
    {
        double minimum;
        double maximum;
        double frequency[256];
    };

    CQHistogramCalculationFilter() : avtDataTreeIterator(), histograms()
    {
        cellCount = 0;
        totalCellCount = 0;
        totalNotSet = true;
    }

    virtual const char *GetType() { return "CQHistogramCalculationFilter"; }
    virtual const char *GetDescription() { return "Calculating histograms"; }

    virtual ~CQHistogramCalculationFilter()
    {
        for(std::map<std::string,Histogram*>::iterator it = histograms.begin();
            it != histograms.end(); ++it)
        {
            if(it->second != 0)
                delete it->second;
        }
    }

    void AddVariable(const std::string &var)
    {
        histograms[var] = 0;
    }

    stringVector GetVariables() const
    {
        stringVector names;
        for(std::map<std::string,Histogram*>::const_iterator it = histograms.begin();
            it != histograms.end(); ++it)
        {
            names.push_back(it->first);
        }
        return names;
    }

    const Histogram *GetHistogram(const std::string &var) const
    {
        const Histogram *retval = 0;
        std::map<std::string,Histogram*>::const_iterator it =
          histograms.find(var);
        if(it != histograms.end())
            retval = it->second;
        return retval;
    }

    long GetTotalCellCount() const
    {
        return totalCellCount;
    }

protected:
    virtual void PreExecute(void)
    {
        cellCount = 0;
    }

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *dr)
    {
        //
        // Get the VTK data set.
        //
        vtkDataSet *ds = dr->GetDataVTK();

        cellCount += ds->GetNumberOfCells();

        return dr;
    }

    virtual void PostExecute(void)
    {
        const char *mName = "CQHistogramCalculationFilter::PostExecute: ";
        avtDataObject_p dob = GetInput();
        avtDataset_p ds;
        CopyTo(ds, dob);

        for(std::map<std::string,Histogram*>::iterator it = histograms.begin();
            it != histograms.end(); ++it)
        {
            // If we have not created a histogram yet for this variable then do it now.
            if(it->second == 0)
            {
                double minmax[2] = {0.,1.};

                // Get the extents
                avtDatasetExaminer::GetDataExtents(ds, minmax, it->first.c_str());
                debug5 << mName << "Calculated data extents for " << it->first
                       << " [" << minmax[0] << ", " << minmax[1] << "]" << endl;

                std::vector<VISIT_LONG_LONG> hist(256, 0);
                if(!avtDatasetExaminer::CalculateHistogram(ds, it->first, minmax[0], minmax[1], hist))
                {
                    debug1 << "CalculateHistogram failed" << endl;
                }
                
                it->second = new Histogram;
                for(int i = 0; i < 256; ++i)
                    it->second->frequency[i] = (double)hist[i];

                // Print the histogram.
                debug5 << mName << "Calculated histogram for " << it->first << endl;
                for(int i = 0; i < 256; ++i)
                    debug5 << "\thist[" << i << "] = " << hist[i] << endl;
            
                it->second->minimum = minmax[0];
                it->second->maximum = minmax[1];
            }
        }

        // Sum the cells and send to all procs.
        SumLongAcrossAllProcessors(cellCount);

        if(totalNotSet)
        {
            totalCellCount += cellCount;
            totalNotSet = false;
        }
    }

    virtual avtContract_p ModifyContract(avtContract_p contract)
    {
        const char *mName = "CQHistogramCalculationFilter::ModifyContract: ";
        avtContract_p newContract = new avtContract(contract);
        std::string origvar(newContract->GetDataRequest()->GetOriginalVariable());
 
        for(std::map<std::string,Histogram*>::iterator it = histograms.begin();
            it != histograms.end(); ++it)
        {
            // Request extents
//            newContract->SetCalculateVariableExtents(it->first, true);

            // Try and get a histogram. If we can't then request the data as a
            // secondary variable.
            avtHistogramSpecification hist;
            hist.SpecifyHistogram(contract->GetDataRequest()->GetTimestep(), it->first, 256);
            if(GetMetaData()->GetHistogram(&hist))
            {
                it->second = new Histogram;
                it->second->minimum = hist.GetBounds()[0][0];
                it->second->maximum = hist.GetBounds()[0][1];
                for(int i = 0; i < 256; ++i)
                    it->second->frequency[i] = (double)(hist.GetCounts()[i]);

                // Print the histogram.
                debug5 << mName << "Obtained existing histogram for " << it->first << endl;
                for(int i = 0; i < 256; ++i)
                    debug5 << "\thist[" << i << "] = " << hist.GetCounts()[i] << endl;
            }
            else if(origvar != it->first &&
                    !newContract->GetDataRequest()->HasSecondaryVariable(it->first.c_str()))
            {
                newContract->GetDataRequest()->AddSecondaryVariable(it->first.c_str());
            }
       }

       return newContract;
    }

    std::map<std::string, Histogram *> histograms;
    long                               cellCount;
    long                               totalCellCount;
    bool                               totalNotSet;
};


// *****************************************************************************
// *****************************************************************************
// ***
// *** CumulativeQuery
// ***
// *****************************************************************************
// *****************************************************************************

// ****************************************************************************
// Class: CumulativeQuery
//
// Purpose:
//   This class creates a cumulative query selection based on time step data.
//
// Notes:      The class is templated on CQCellIdentifier and CQSelection so
//             we can use this class' logic to implement CQ that relates cells
//             using different strategies. The cellId-specific coding is moved
//             into the CQSelection implementation. 
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 11:30:52 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 14:46:00 PDT 2011
//   Combine some doms/zones arguments into CQCellIdentifier.
//
//   Brad Whitlock, Sat Nov  5 01:32:16 PDT 2011
//   I templated the class so it can support different selection types that 
//   use different cell id types.
//
// ****************************************************************************

struct CQCellData
{
    int frequency;
    double variable;
};

template <class CQCellIdentifier, class CQSelection>
class CumulativeQuery
{
public:
    CumulativeQuery() : idVariable(), cellFrequency(), cellsPerTimeStep()
    {
    }

    virtual ~CumulativeQuery()
    {
    }

    void SetIdVariable(const std::string &id)
    {
        idVariable = id;
    }

    const std::string &GetIdVariable() const { return idVariable; }

    // The CQFilter class uses this method to supply data objects from which we
    // calculate the selection. We calculate the cellFrequency and then the selection.
    void CreateSelection(const SelectionProperties &props,
                         std::vector<avtDataTree_p> &, 
                         CQSelection &outSel,
                         doubleVector &hist,
                         double &min,
                         double &max);

    // Calculate the selection but use cellFrequency data we already calculated.
    void CreateSelection(const SelectionProperties &props,
                         const CQSelection &cellFrequency,
                         const std::vector<CQSelection> &cellsPerTimeStep,
                         CQSelection &narrowedSelection,
                         doubleVector &hist,
                         double &min,
                         double &max);

    const CQSelection &GetCellFrequency() const { return cellFrequency; }
    const std::vector<CQSelection> &GetCellsPerTimeStep() const { return cellsPerTimeStep; }

private:
    void CalculateFrequency(const SelectionProperties &props,
                            std::vector<avtDataTree_p> &timesteps,
                            CQSelection &cellFrequency,
                            std::vector<CQSelection> &cellsPerTimeStep) const;

    void Summation(const SelectionProperties &props,
                   const CQSelection &cellFrequency,
                   const std::vector<CQSelection> &cellsPerTimeStep,
                   CQSelection &selection,
                   std::vector<CQSelection> &selectionPerTimeStep) const;

    bool GlobalizeSelection(const CQSelection &selection,
                            CQCellIdentifier *&allIds,
                            int *&allFrequencies,
                            int &totalCells,

                            const std::vector< CQSelection > &selectionPerTimeSteps,
                            CQCellIdentifier **&allIdsPerTimeStep,
                            double **&allVariablesPerTimeStep,
                            std::vector< int >& totalCellsPerTimeStep) const;

    void SelectAndHistogram( const SelectionProperties &props,
                             const std::vector< CQSelection > &selectionPerTimeStep,
                             CQSelection &narrowedSelection,
                             doubleVector &histogram) const;

    void SelectAndHistogram(const SelectionProperties &props,
                            const CQCellIdentifier *allIds,
                            const int *allFrequencies,
                            int totalCells,
                            CQSelection &narrowedSelection,
                            doubleVector &histogram,
                            double &min,
                            double &max) const;

    void SelectAndHistogram(const SelectionProperties &props,
                            CQCellIdentifier **allIdsPerTimeStep,
                            double **allVariablesPerTimeStep,
                            std::vector< int > totalCellsPerTimeStep,
                            CQSelection &narrowedSelection,
                            doubleVector &histogram,
                            double &min,
                            double &max) const;

    void ComputeGlobalSizeAndOffset(const CQSelection &selection,
                                    int &globalSize, int &myOffset) const;

    void GlobalizeData(const CQSelection &selection,
                       int globalSize, int myOffset,
                       int *&allFrequencies) const;

    void GlobalizeData(const CQSelection &selection,
                       int globalSize, int myOffset,
                       double *&allVariables) const;

    // Members that we calculate as stage 1 of the selection calculation during
    // frequency calculation.
    std::string idVariable;
    CQSelection cellFrequency;
    std::vector< CQSelection > cellsPerTimeStep;
};

// ****************************************************************************
// Method: CumulativeQuery::CreateSelection
//
// Purpose: 
//   This method creates a selection based on datasets over time.
//
// Arguments:
//   props             : The selection properties.
//   timesteps         : The data trees for each timestep.
//   narrowedSelection : The returned selection that we created from CQ.
//   hist              : The returned histogram for the selection.
//
// Returns:    We return a data tree with a dummy dataset that has the number
//             of cells in our final selection and we place the original zones
//             array on it since that is what is used later to define the
//             named selection.
//
// Note:       As a side-effect, we store the cell freqencies and cells per
//             timestep so we can get that information later for caching
//             purposes.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:28:53 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 01:32:16 PDT 2011
//   I templated the class so it can support different cell identifier types.
//   
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::CreateSelection(
    const SelectionProperties &props,
    std::vector<avtDataTree_p> &timesteps, 
    CQSelection &narrowedSelection,
    doubleVector &hist,
    double &min,
    double &max)
{
    debug5 << "CreateSelection: phase 1" << std::endl;
    StackTimer t0("CreateSelection: phase 1");
    cellFrequency.clear();
    cellsPerTimeStep.clear();

    //
    // Iterate through the data for all time steps and come up with a 
    // "cell frequency". We may find different cell ids over time so we
    // create a set that includes any cell that was present in any dataset
    // over time. Each time we find that cell in a different time step,
    // we increment its "cell frequency". The cell frequency thus becomes
    // a count of how many time steps contained the cell.
    //
    CalculateFrequency(props, timesteps, cellFrequency, cellsPerTimeStep);

    // Now that we know the cell frequency and the cells per time step, we
    // can pass them into the next phase of the selection creation.
    CreateSelection(props, cellFrequency, cellsPerTimeStep, 
                    narrowedSelection, hist, min, max);
}

// ****************************************************************************
// Method: CumulativeQuery::CreateSelection
//
// Purpose: 
//   This method creates a selection based on previously gathered cell 
//   frequencies.
//
// Arguments:
//   props             : The selection properties.
//   cellFrequency     : The number of times each cell appears over time.
//   cellsPerTimeStep  : Cells in each timestep.
//   narrowedSelection : The returned selection that we created from CQ.
//   hist              : The returned histogram for the selection.
//
// Returns:    We return a data tree with a dummy dataset that has the number
//             of cells in our final selection and we place the original zones
//             array on it since that is what is used later to define the
//             named selection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:28:53 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 14:32:35 PDT 2011
//   Combine the doms/zones ids to be more general.
//
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier,
                CQSelection>::CreateSelection(const SelectionProperties &props,
                                              const CQSelection &cellFrequency,
                                              const std::vector<CQSelection> &cellsPerTimeStep,
                                              CQSelection &narrowedSelection,
                                              doubleVector &hist,
                                              double &min,
                                              double &max)
{
    debug5 << "CreateSelection: phase 2" << std::endl;

    StackTimer t0("CreateSelection: phase 2");

    //
    // Apply the summation rule so we get a subset of the cellFrequency
    // map that will contribute to our selection.
    //
    CQSelection selection;
    std::vector<CQSelection> selectionPerTimeStep;

    Summation(props,
              cellFrequency, cellsPerTimeStep,
              selection, selectionPerTimeStep);
//    SaveSelection("selection1.txt", selection);

    //
    // Let's combine the selections from all processors and then send that
    // global selection to all processors, in the process transforming it into
    // a set of arrays.
    //
    CQCellIdentifier *allIds     = NULL;
    int      *allFrequencies     = NULL;

    CQCellIdentifier **allIdsPerTimeStep = NULL;
    double   **allVariablesPerTimeStep   = NULL;

    int totalCells = 0;
    std::vector< int > totalCellsPerTimeStep;

    if( !GlobalizeSelection(selection, allIds, allFrequencies, totalCells,
                            selectionPerTimeStep, allIdsPerTimeStep,
                            allVariablesPerTimeStep, totalCellsPerTimeStep) )
    {
        EXCEPTION1(VisItException, "You have selected too many zones in your "
                   "named selection.  Disallowing ... no selection created");
    }

    //
    // Sort the entire selection according to the selected sorting mode and
    // further narrow the selection. This is the "histogramming" part.
    //
    hist.clear();
    if(props.GetHistogramType() == SelectionProperties::HistogramTime)
    {
        SelectAndHistogram( props, selectionPerTimeStep, 
                            narrowedSelection, hist);
    }
    else if(props.GetHistogramType() == SelectionProperties::HistogramID ||
            props.GetHistogramType() == SelectionProperties::HistogramMatches)

    {
        SelectAndHistogram(props, allIds, allFrequencies, totalCells,
                           narrowedSelection, hist, min, max);
    }

    else if(props.GetHistogramType() == SelectionProperties::HistogramVariable)

    {
        SelectAndHistogram(props,
                           allIdsPerTimeStep, allVariablesPerTimeStep,
                           totalCellsPerTimeStep,
                           narrowedSelection, hist, min, max);
    }

    selection.clear();    

    delete [] allIds;
    delete [] allFrequencies;

    for( size_t ts=0; ts<selectionPerTimeStep.size(); ++ts )
    {
      delete [] allIdsPerTimeStep[ts];
      delete [] allVariablesPerTimeStep[ts];
    }

    delete [] allIdsPerTimeStep;
    delete [] allVariablesPerTimeStep;
}

// ****************************************************************************
// Method: CumulativeQuery::CalculateFrequency
//
// Purpose: 
//   This method iterates over this processor's datasets for all time steps and
//   builds up a map of cellid to number of times the cell appears in different
//   time steps.
//
// Arguments:
//   props         : The selection properties.
//   timesteps     : The vector of datasets for all time steps.
//   cellFrequency : The cell frequency map we're creating.
//   cellsPerTimeStep : Cells in each time step that match
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:33:41 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Oct 12 12:08:16 PDT 2011
//   I made histogram variable be a string instead of an index.
//
//   Brad Whitlock, Fri Oct 28 13:37:23 PDT 2011
//   I generalized the id variable.
//
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::CalculateFrequency(
    const SelectionProperties &props,
    std::vector<avtDataTree_p> &timesteps,
    CQSelection &cellFrequency,
    std::vector < CQSelection > &cellsPerTimeStep) const
{
    debug5 << "CalculateFrequency" << std::endl;
    const char *mName = "CQFilter::CalculateFrequency: ";
    StackTimer t0("CalculateFrequency");

    // Make sure that the cellsPerTimestep vector is initialized properly.
    cellsPerTimeStep.resize(timesteps.size() );

    std::string histVar;
    if(props.GetHistogramType() == SelectionProperties::HistogramVariable)
    {
        histVar = props.GetHistogramVariable();
        debug5 << "We're doing histogram by variable " << histVar << endl;
    }
  
    for(size_t ts = 0; ts < timesteps.size(); ++ts)
    {
        if(*timesteps[ts] == NULL)
        {
            debug5 << mName << "Time step " << ts << " of input dataset is empty." << endl;
            continue;
        }

        // Iterate over all pieces of the data for the current time step and 
        // figure out the set of cells.
        int nleaves = 0;
        vtkDataSet **leaves = timesteps[ts]->GetAllLeaves(nleaves);

        for (int i = 0 ; i < nleaves ; i++)
        {
            vtkIdType ncells = leaves[i]->GetNumberOfCells();
            if(ncells == 0)
                continue;

            // Get the id variable.
            vtkDataArray *arr = leaves[i]->GetCellData()->GetArray(idVariable.c_str());
            if(arr == NULL && 
               leaves[i]->GetNumberOfCells() == leaves[i]->GetNumberOfPoints())
            {
                arr = leaves[i]->GetPointData()->GetArray(idVariable.c_str());
            }

            if (arr == NULL)
            {
                debug5 << mName << "Could not locate id variable " << idVariable
                       << ". This dataset will not contribute to the selection."
                       << endl;
                continue;
            }
            if (arr->GetVoidPointer(0) == NULL)
            {
                debug5 << mName << "Could not locate id variable " << idVariable
                       << ". This dataset will not contribute to the selection."
                       << endl;
                continue;
            }

            // If we need an extra variable, get it.
            vtkDataArray *var = 0, *cellVar = 0;
            if(!histVar.empty())
            {
                var = leaves[i]->GetCellData()->GetArray(histVar.c_str());
                if(var == 0)
                {
                    // If we find that the variable is point data then recenter it.
                    vtkDataArray *pointVar = leaves[i]->GetPointData()->GetArray(histVar.c_str());
                    if(pointVar != NULL)
                    {
                        debug5 << "Recentering " << histVar << " so we can histogram with it." << endl;
                        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
                        pd2cd->SetInputData(leaves[i]);
                        pd2cd->Update();

                        vtkDataSet *ds2 = pd2cd->GetOutput();
                        cellVar = ds2->GetCellData()->GetArray(histVar.c_str());
                        if(cellVar != NULL)
                        {
                            debug5 << "We have forced " << histVar << " to be zonal" << endl;
                            cellVar->Register(NULL);
                        }
                        var = cellVar;

                        pd2cd->Delete();
                    }
                    else
                    {
                        debug5 << "Could not get requested histogram variable: " << histVar << endl;
                    }
                }
            }

            for(vtkIdType j = 0; j < ncells; ++j)
            {
                CQCellIdentifier cellId(cellsPerTimeStep[ts].CellIdentifierFromTuple(arr, j));
                typename CQSelection::iterator p =
                  cellsPerTimeStep[ts].find(cellId);

                double variable = var ? var->GetTuple1(j) : 0.;

                if(p == cellsPerTimeStep[ts].end())
                {
                    CQCellData c;
                    c.frequency = 1;
                    c.variable = variable;

                    cellsPerTimeStep[ts][cellId] = c;
                }
                else
                {
                    // we have a cell we've seen before so we must have fragments
                    // of that cell so sum the variables.
                    p->second.variable += variable;
                }
            }

            if(cellVar != NULL)
                cellVar->Delete();
        }

        delete [] leaves;

        // Merge the results from the current time step into the overall
        // cell frequency map. We do this so there are no duplicates from
        // cells that may have been split up previously. This way, cell
        // fragments do not impact the frequency.

        typename CQSelection::iterator it;
        for(it = cellsPerTimeStep[ts].begin(); it != cellsPerTimeStep[ts].end(); ++it)
        {
            typename CQSelection::iterator p = cellFrequency.find(it->first);

            if(p == cellFrequency.end())
            {
                CQCellData c;
                c.frequency = 1;
                c.variable = 0;

                // insert new cell
                cellFrequency[it->first] = c; //it->second;
            }
            else
            {
                // we've seen this cell before in another time step so increment.
                p->second.frequency += 1;
            }
        }
    }
}

// ****************************************************************************
// Method: CumulativeQuery::Summation
//
// Purpose: 
//   Examine the cell frequency map and apply the right "summation" to combine
//   the cells over time into a new selection.
//
// Arguments:
//   props         : The selection properties.
//   cellFrequency : The number of times each cell was found in the datasets
//                   over time.
//   cellsPerTimeStep : Cells in each time step that match
//   selection     : The new "summed" selection.
//   selectionPerTimeStep : The new "summed" selection for each time step that match
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 11:04:39 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 01:32:16 PDT 2011
//   I templated the class so it can support different cell identifier types.
//  
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::Summation(
    const SelectionProperties &props,
    const CQSelection &cellFrequency,
    const std::vector< CQSelection > &cellsPerTimeStep,
    CQSelection &selection,
    std::vector< CQSelection > &selectionPerTimeStep) const
{
    debug5 << "Summation" << std::endl;
    StackTimer t0("Summation");

    unsigned int nts = cellsPerTimeStep.size();

    selectionPerTimeStep.resize( nts );

    if(props.GetCombineRule() == SelectionProperties::CombineAnd)
    {
        typename CQSelection::const_iterator it;
        for(it = cellFrequency.begin(); it != cellFrequency.end(); ++it)
        {
            if((unsigned  int)it->second.frequency == nts)
            {
                selection[it->first] = it->second;

                for( unsigned int ts=0; ts<nts; ++ts )
                {
                  // cellsPerTimeStep is a const but the [] operator
                  // is not a const so use find instead which is a
                  // const operation.
                  typename CQSelection::const_iterator p =
                    cellsPerTimeStep[ts].find(it->first);

                  if(p == cellsPerTimeStep[ts].end())
                  {
                    // This should never happen because for an and
                    // operator the cell must be in all tiem time
                    // steps.
                  }
                  else
                    selectionPerTimeStep[ts][it->first] = p->second;
                }
            }
        }
    }
    else
    {
        selection = cellFrequency;

        for( unsigned int ts=0; ts<nts; ++ts )
          selectionPerTimeStep[ts] = cellsPerTimeStep[ts];
    }
}

// ****************************************************************************
// Method: CumulativeQuery::ComputeGlobalSelectionSize
//
// Purpose: 
//   Compute the global number of cells in the selection when all processors
//   are considered. Also provide an offset into a global array where the
//   local data should be stored.
//
// Arguments:
//   selection  : The local selection.
//   globalSize : The size of the global selection.
//   myOffset   : The offset in the global array where local data can be stored.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 14:35:49 PDT 2011
//
// Modifications:
//   
// ****************************************************************************
template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::ComputeGlobalSizeAndOffset(
    const CQSelection &selection, int &globalSize, int &myOffset) const
{
#ifdef PARALLEL
    // Compute the global selection size. 
    int *numPerProcIn = new int[PAR_Size()];
    int *numPerProc   = new int[PAR_Size()];
    for (int i = 0 ; i < PAR_Size() ; i++)
        numPerProcIn[i] = 0;
    numPerProcIn[PAR_Rank()] = selection.size();
    SumIntArrayAcrossAllProcessors(numPerProcIn, numPerProc, PAR_Size());
    globalSize = 0;
    for (int i = 0 ; i < PAR_Size() ; i++)
        globalSize += numPerProc[i];

    myOffset = 0;
    for (int i = 0 ; i < PAR_Rank()-1 ; i++)
        myOffset += numPerProc[i];

    delete [] numPerProcIn;
    delete [] numPerProc;
#else
    globalSize = (int)selection.size();
    myOffset = 0;
#endif
}

// ****************************************************************************
// Method: CumulativeQuery::GlobalizeData
//
// Purpose: 
//   Globalize the selection's data.
//
// Arguments:
//   selection  : The local selection.
//   globalSize : The size of the global selection.
//   myOffset   : The local offset into the global array.
//   allFrequencies : The returned array contains the global selection frequencies.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 14:37:39 PDT 2011
//
// Modifications:
//   
// ****************************************************************************
template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::GlobalizeData(
    const CQSelection &selection,
    int globalSize, int myOffset, int *&allFrequencies) const
{
    int i;
    typename CQSelection::const_iterator it;
#ifdef PARALLEL
    // Unpack the data into arrays
    int *sendFrequency = new int[globalSize];
    memset(sendFrequency, 0, sizeof(int) * globalSize);

    for(i = 0, it = selection.begin(); it != selection.end(); ++it, ++i)
    {
        sendFrequency[i] = it->second.frequency;
    }

    // Globalize
    allFrequencies = new int[globalSize];
    SumIntArrayAcrossAllProcessors(sendFrequency, allFrequencies, globalSize);
    delete [] sendFrequency;
#else
    // Unpack the data into arrays
    allFrequencies = new int[globalSize];

    for(i = 0, it = selection.begin(); it != selection.end(); ++it, ++i)
    {
        allFrequencies[i] = it->second.frequency;
    }
#endif
}


// ****************************************************************************
// Method: CumulativeQuery::GlobalizeData
//
// Purpose: 
//   Globalize the selection's data.
//
// Arguments:
//   selection  : The local selection.
//   globalSize : The size of the global selection.
//   myOffset   : The local offset into the global array.
//   allVariables : The returned array contains the global selection variables.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 14:37:39 PDT 2011
//
// Modifications:
//   
// ****************************************************************************
template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::GlobalizeData(
    const CQSelection &selection,
    int globalSize, int myOffset, double *&allVariables) const
{
    int i;
    typename CQSelection::const_iterator it;
#ifdef PARALLEL
    // Unpack the data into arrays
    double *sendVariable = new double[globalSize];
    memset(sendVariable, 0, sizeof(double) * globalSize);

    for(i = 0, it = selection.begin(); it != selection.end(); ++it, ++i)
    {
        sendVariable[i] = it->second.variable;
    }

    // Globalize
    allVariables = new double[globalSize];
    SumDoubleArrayAcrossAllProcessors(sendVariable, allVariables, globalSize);
    delete [] sendVariable;
#else
    // Unpack the data into arrays
    allVariables = new double[globalSize];
      
    for(i = 0, it = selection.begin(); it != selection.end(); ++it, ++i)
    {
        allVariables[i] = it->second.variable;
    }
#endif
}

// ****************************************************************************
// Method: CumulativeQuery::GlobalizeSelection
//
// Purpose: 
//   This method takes the selections from each processor and combines them
//   and sends the results to each processor as a set of arrays that describe
//   the selection that was created.
//
// Arguments:
//   selection      : This processor's selection
//
//   allDomains     : The array of domain numbers for each cell in the global selection.
//   allCellIds     : The array of cell ids for each cell in the global selection.
//   allFrequencies : The array of frequencies for each cell in the global selection.
//   allVariables   : The array of variable values for each cell in the global selection.
//   totalCells     : The total number of cells in the global selection.
//
// Returns:    True if the selection was globalized; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 10:58:41 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 13:38:57 PDT 2011
//   Rewrite.
//
// ****************************************************************************
template <class CQCellIdentifier, class CQSelection>
bool
CumulativeQuery<CQCellIdentifier, CQSelection>::GlobalizeSelection(
    const CQSelection &selection,
    CQCellIdentifier *&allIds,
    int *&allFrequencies,
    int &globalSize,
    const std::vector< CQSelection > &selectionPerTimeSteps,
    CQCellIdentifier **&allIdsPerTimeStep,
    double **&allVariablesPerTimeStep,
    std::vector< int >& globalSizePerTimeStep) const
{
    debug5 << "GlobalizeSelection" << std::endl;
    StackTimer t0("GlobalizeSelection");

    int myOffset = 0;
    std::vector< int > myOffsetPerTimeStep;

    int nts = selectionPerTimeSteps.size();

    globalSizePerTimeStep.resize( nts );
    myOffsetPerTimeStep.resize( nts );
    
#ifdef PARALLEL
    globalSize = 0;

    ComputeGlobalSizeAndOffset(selection, globalSize, myOffset);

    if(globalSize > avtNamedSelection::MaximumSelectionSize())
    {
        debug5 << "The number of total cells (" << globalSize
               << ") is above the allowed limit." << endl;
        return false;
    }

    for( int ts=0; ts<nts; ++ts )
    {
      globalSizePerTimeStep[ts] = 0;

      ComputeGlobalSizeAndOffset(selectionPerTimeSteps[ts],
                                 globalSizePerTimeStep[ts],
                                 myOffsetPerTimeStep[ts]);
    }

#else
    globalSize = (int) selection.size();

    for( int ts=0; ts<nts; ++ts )
      globalSizePerTimeStep[ts] = (int) selectionPerTimeSteps[ts].size();
#endif

    // Globalize the selection keys (ids).
    selection.GlobalizeIds(globalSize, myOffset, allIds);

    // Globalize the selection data
    GlobalizeData(selection, globalSize, myOffset, allFrequencies);

    allIdsPerTimeStep = new CQCellIdentifier *[nts];
    allVariablesPerTimeStep = new double *[nts];

    for( int ts=0; ts<nts; ++ts )
    {
      // Globalize the selection keys (ids) across all time steps
      selectionPerTimeSteps[ts].GlobalizeIds(globalSizePerTimeStep[ts],
                                             myOffsetPerTimeStep[ts],
                                             allIdsPerTimeStep[ts]);

      // Globalize the selection data across all time steps
      GlobalizeData(selectionPerTimeSteps[ts], globalSizePerTimeStep[ts],
                    myOffsetPerTimeStep[ts], allVariablesPerTimeStep[ts]);
    }

    return true;
}

// ****************************************************************************
// Class: cq_sort_by_value
//
// Purpose:
//   This class is a functor that lets std::sort sort an array of values using
//   value[i] instead of i.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:04:49 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

template <class T>
class cq_sort_by_value
{
public:
    cq_sort_by_value(const T *v) : value(v) { }
    bool operator()(int A, int B) const
    {
        return value[A] < value[B];
    }
private:
    const T *value;
};

// ****************************************************************************
// Method: CumulativeQuery::SelectAndHistogram
//
// Purpose: 
//   This method takes the globalized selection (the list of matching cells 
//   for all domains for all time) and sorts it in the manner prescribed by
//   the desired histogram operation and then we keep some number of bins
//   from that and create our final "narrowed" selection. We also compute an
//   average frequency for the cells in each histogram bin.
//
// Arguments:
//   props             : The selection properties.
//   selectionPerTimeStep: The array containing the ids for the global selectionper time step.
//   narrowedSelection : The returned selection.
//   histogram         : The returned histogram of avg frequency per bin.
//
// Returns:    narrowedSelection, histogram.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:20:58 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 14:19:35 PDT 2011
//   I combined the doms/zones into CQCellIdentifier. I removed the step to
//   restrict the narrowed selection to this processor's data. We just won't
//   have to globalize again later.
//
//   Brad Whitlock, Sat Nov  5 01:32:16 PDT 2011
//   I templated the class so it can support different cell identifier types.
//
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::SelectAndHistogram(
    const SelectionProperties &props,
    const std::vector< CQSelection > &selectionPerTimeStep,
    CQSelection &narrowedSelection,
    doubleVector &histogram ) const
{
    debug5 << "SelectAndHistogram" << std::endl;
    const char *mName = "CQFilter::SelectAndHistogram: ";
    StackTimer t0("SelectAndHistogram");

    debug5 << mName << "start" << endl;

    int numBins = selectionPerTimeStep.size();
    
    for(int bin = 0; bin < numBins; ++bin)
    {
      // If the bin is one that we're selecting then add its cells to the
      // narrowed selection.
      if(props.GetHistogramStartBin() <= bin && bin <= props.GetHistogramEndBin())
      {
        typename CQSelection::const_iterator it;
        
        for(it = selectionPerTimeStep[bin].begin(); it != selectionPerTimeStep[bin].end(); ++it)
        {
          CQCellIdentifier cellid(it->first);
          
          typename CQSelection::iterator p =
            narrowedSelection.find(cellid);
              
          if( p == narrowedSelection.end() )
          {
            CQCellData c;
            c.frequency = 1;
            c.variable = 0.; // we don't need the variable anymore
          
            // insert new cell
            narrowedSelection[cellid] = c;
          }
          else
          {
            // we've seen this cell before in another time step so increment.
            p->second.frequency += 1;
          }
        }
      }

      histogram.push_back( double(selectionPerTimeStep[bin].size()) );
    }
}


// ****************************************************************************
// Method: CumulativeQuery::SelectAndHistogram
//
// Purpose: 
//   This method takes the globalized selection (the list of matching cells 
//   for all domains for all time) and sorts it in the manner prescribed by
//   the desired histogram operation and then we keep some number of bins
//   from that and create our final "narrowed" selection. We also compute an
//   average frequency for the cells in each histogram bin.
//
// Arguments:
//   props             : The selection properties.
//   allIds            : The array containing the ids for the global selection.
//   allFrequencies    : The array containing the frequencies for the global selection.
//   totalCells        : The number of cells in the selection.
//   narrowedSelection : The returned selection.
//   histogram         : The returned histogram of avg frequency per bin.
//
// Returns:    narrowedSelection, histogram.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:20:58 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 14:19:35 PDT 2011
//   I combined the doms/zones into CQCellIdentifier. I removed the step to
//   restrict the narrowed selection to this processor's data. We just won't
//   have to globalize again later.
//
//   Brad Whitlock, Sat Nov  5 01:32:16 PDT 2011
//   I templated the class so it can support different cell identifier types.
//
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::SelectAndHistogram(
    const SelectionProperties &props,
    const CQCellIdentifier *allIds, 
    const int *allFrequencies,
    int totalCells,
    CQSelection &narrowedSelection,
    doubleVector &histogram,
    double &min,
    double &max) const
{
    debug5 << "SelectAndHistogram" << std::endl;
    const char *mName = "CQFilter::SelectAndHistogram: ";
    StackTimer t0("SelectAndHistogram");

    debug5 << mName << "start" << endl;

    // Create an index variable that we'll sort using another variable.
    int *index = new int[totalCells];

    for(int i = 0; i < totalCells; ++i)
        index[i] = i;

    // We need to bin the index array into some number of bins and select
    // the cells in the selected bins into our new narrowed selection.
    int *binPoints = NULL, numBins = 0;

    // ID Histogram
    if(props.GetHistogramType() == SelectionProperties::HistogramID)
    {
        // Sort the index based on cell id
        debug5 << mName << "Sorting index based on cellid" << endl;
        std::sort(index, index + totalCells,
                  cq_sort_by_value<CQCellIdentifier>(allIds));

        min = 0;
        max = 0;

        // For IDs the number of bins is based on the user defined
        // number of bins.
        numBins = props.GetHistogramNumBins();
        binPoints = new int[numBins + 1];

        for(int i = 0; i < numBins+1; ++i)
        {
            float t = float(i) / float(numBins);
            binPoints[i] = int(t * float(totalCells));
        }

        // Capture the ID of the first cell in the start bin.
        narrowedSelection.startBinID = allIds[index[0]];
        narrowedSelection.endBinID = allIds[index[totalCells-1]];
    }

    // Matches Histogram
    else if(props.GetHistogramType() == SelectionProperties::HistogramMatches)
    {
        // Sort the index based on frequency
        debug5 << mName << "Sorting index based on frequency" << endl;
        std::sort(index, index + totalCells,
                  cq_sort_by_value<int>(allFrequencies));

        // For matches the number of bins is based on the number of matches
        min = allFrequencies[index[0]];
        max = allFrequencies[index[totalCells-1]];

        numBins = max-min+1;
        binPoints = new int[numBins + 1];

        // The starting binPoints will always be the first cell.
        binPoints[0] = 0;

        for(int i=1, j=0; i<numBins; ++i)
        {
            // Get the threshold for this bin. The threshold is based on
            // the frequency of matches.
            float t = min + float(i) * (max-min) / float(numBins);

            while( allFrequencies[index[j]] < t )
              ++j;

            binPoints[i] = j;
        }

        // The ending binPoints will always be the totalCells.
        binPoints[numBins] = totalCells;
    }

    debug5 << "numBins=" << numBins
           << ", startBin=" << props.GetHistogramStartBin()
           << ", endBin=" << props.GetHistogramEndBin() << endl;

    for(int bin = 0; bin < numBins; ++bin)
    {
        // If the bin is one that we're selecting then add its cells to the
        // narrowed selection.
        if( props.GetHistogramStartBin() <= bin &&
            bin <= props.GetHistogramEndBin() )
        {
            debug5 << "Bin " << bin << "contains indices ["
                   << binPoints[bin] << ", " << binPoints[bin+1]
                   << "). Adding cells from this bin to the new selection."
                   << endl;

            for(int i = binPoints[bin]; i < binPoints[bin+1]; ++i)
            {
                int idx = index[i];
                CQCellIdentifier cellid(allIds[idx]);

                CQCellData c;
                c.frequency = allFrequencies[idx];
                c.variable = 0.; // we don't need the variable anymore

                narrowedSelection[cellid] = c;
            }
        }

        if(props.GetHistogramType() == SelectionProperties::HistogramID)
        {
            // Compute an average for the bin.
            double sum = 0.;
            for(int i = binPoints[bin]; i < binPoints[bin+1]; ++i)
                sum += allFrequencies[index[i]];
            double averageForBin;
            if( binPoints[bin+1] > binPoints[bin] )
                averageForBin = sum / double(binPoints[bin+1] - binPoints[bin]);
            else
                averageForBin = 0;
            histogram.push_back(averageForBin);
            debug5 << "Bin " << bin << " average frequency: " << averageForBin << endl;
        }
        else if(props.GetHistogramType() == SelectionProperties::HistogramMatches)
          histogram.push_back(binPoints[bin+1] - binPoints[bin]);

    }

    delete [] binPoints;
    delete [] index;
    debug5 << mName << "end" << endl;
}


// ****************************************************************************
// Method: CumulativeQuery::SelectAndHistogram
//
// Purpose: 
//   This method takes the globalized selection (the list of matching cells 
//   for all domains for all time) and sorts it in the manner prescribed by
//   the desired histogram operation and then we keep some number of bins
//   from that and create our final "narrowed" selection. We also compute an
//   average frequency for the cells in each histogram bin.
//
// Arguments:
//   props             : The selection properties.
//   allIdsPerTimeStep       : The array containing the ids for the global selection per time step.
//   allVariablesPerTimeStep : The array containing the variables for the global selection per time step.
//   totalCellsPerTimeStep        : The number of cells in the selection per time step.
//   narrowedSelection : The returned selection.
//   histogram         : The returned histogram of avg frequency per bin.
//
// Returns:    narrowedSelection, histogram.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:20:58 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 14:19:35 PDT 2011
//   I combined the doms/zones into CQCellIdentifier. I removed the step to
//   restrict the narrowed selection to this processor's data. We just won't
//   have to globalize again later.
//
//   Brad Whitlock, Sat Nov  5 01:32:16 PDT 2011
//   I templated the class so it can support different cell identifier types.
//
// ****************************************************************************

template <class CQCellIdentifier, class CQSelection>
void
CumulativeQuery<CQCellIdentifier, CQSelection>::SelectAndHistogram(
    const SelectionProperties &props,
    CQCellIdentifier **allIdsPerTimeStep, 
    double **allVariablesPerTimeStep,
    std::vector <int> totalCellsPerTimeStep,
    CQSelection &narrowedSelection,
    doubleVector &histogram,
    double &min,
    double &max) const
{
    debug5 << "SelectAndHistogram" << std::endl;
    const char *mName = "CQFilter::SelectAndHistogram: ";
    StackTimer t0("SelectAndHistogram");

    debug5 << mName << "start" << endl;

    int nts = totalCellsPerTimeStep.size();

    // Merge all of the data into a set of single arrays.
    int totalCells = 0;

    for( int ts=0; ts<nts; ++ts )
      totalCells += totalCellsPerTimeStep[ts];

    CQCellIdentifier *allIds = new CQCellIdentifier[totalCells];
    double *allVariables = new double[totalCells];
    int *index = new int[totalCells];


    // Create an index variable that we'll sort using another variable
    // while storing the variables and ids in a one dimensional array.
    for( int ts=0, i=0; ts<nts; ++ts )
    {
      for( int j=0; j<totalCellsPerTimeStep[ts]; ++j )
      {
        allIds[i] = allIdsPerTimeStep[ts][j];
        allVariables[i] = allVariablesPerTimeStep[ts][j];
        index[i] = i;
        ++i;
      }
    }


    // We need to bin the index array into some number of bins and select
    // the cells in the selected bins into our new narrowed selection.
    int *binPoints = NULL, numBins = 0;

    // Sort the index based on variable
    debug5 << mName << "Sorting index based on variable" << endl;
    std::sort(index, index + totalCells,
              cq_sort_by_value<double>(allVariables));

    // For variables the number of bins is based on user defined
    // the number of bins.
    min = allVariables[index[0]];
    max = allVariables[index[totalCells-1]];

    numBins = props.GetHistogramNumBins();
    binPoints = new int[numBins + 1];
        
    // The starting binPoints will always be the first cell.
    binPoints[0] = 0;

    for(int i=1, j=0; i<numBins; ++i)
    {
        // Get the threshold for this bin. The threshold is based on
        // the frequency of matches.
        float t = min + float(i) * (max-min) / float(numBins);

        while( allVariables[index[j]] < t )
          ++j;

        binPoints[i] = j;
    }

    // The ending binPoints will always be the totalCells.
    binPoints[numBins] = totalCells;

    debug5 << "numBins=" << numBins
           << ", startBin=" << props.GetHistogramStartBin()
           << ", endBin=" << props.GetHistogramEndBin() << endl;

    for(int bin = 0; bin < numBins; ++bin)
    {
        // If the bin is one that we're selecting then add its cells to the
        // narrowed selection.
        if(bin >= props.GetHistogramStartBin() &&
           bin <= props.GetHistogramEndBin())
        {
            debug5 << "Bin " << bin << "contains indices ["
                   << binPoints[bin] << ", " << binPoints[bin+1]
                   << "). Adding cells from this bin to the new selection."
                   << endl;
            for(int i = binPoints[bin]; i < binPoints[bin+1]; ++i)
            {
                int idx = index[i];
                CQCellIdentifier cellid(allIds[idx]);

                typename CQSelection::iterator p
                  = narrowedSelection.find(cellid);

                if( p == narrowedSelection.end() )
                {
                  CQCellData c;
                  c.frequency = 1;
                  c.variable = 0.; // we don't need the variable anymore

                  narrowedSelection[cellid] = c;
                }
                else
                {
                  // we've seen this cell before in another time step so increment.
                  p->second.frequency += 1;
                }
            }
        }

        histogram.push_back(binPoints[bin+1] - binPoints[bin]);
    }

    delete [] binPoints;
    delete [] index;
    debug5 << mName << "end" << endl;
}

// *****************************************************************************
// *****************************************************************************
// ***
// *** CQDomainZoneIdSelection
// ***
// *****************************************************************************
// *****************************************************************************

typedef std::pair<unsigned int, unsigned int> CQDomainZoneId;

// ****************************************************************************
// Class: CQDomainZoneIdSelection
//
// Purpose:
//   This class implemetns a selection type that lets us index cells using a
//   (domain,zoneid) tuple. 
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:44:07 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class CQDomainZoneIdSelection : public std::map<CQDomainZoneId, CQCellData>
{
public:
    CQDomainZoneIdSelection() : std::map<CQDomainZoneId, CQCellData>(), startBinID(0,0), endBinID(0,0)
    {
    }

    avtNamedSelection *CreateNamedSelection(const std::string &name) const;
    vtkDataSet        *CreateSelectedDataset(const std::string &idVariable) const;
    CQDomainZoneId     CellIdentifierFromTuple(vtkDataArray *arr, vtkIdType cellid) const;
    void               GlobalizeIds(int globalSize, int myOffset, CQDomainZoneId*&allIds) const;
    CQDomainZoneId     startBinID, endBinID;
};

// ****************************************************************************
// Method: CQDomainZoneIdSelection::CreateNamedSelection
//
// Purpose: 
//   Create a new named selection from the selection map keys.
//
// Arguments:
//   name : The name of the new selection.
//
// Returns:    We return a new avtZoneIdNamedSelection object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:45:14 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

avtNamedSelection *
CQDomainZoneIdSelection::CreateNamedSelection(const std::string &name) const
{
    int *doms = new int[size()+1];
    int *zones = new int[size()+1];
    int i = 0;
    for(const_iterator it = begin(); it != end(); ++it, ++i)
    {
        doms[i] = it->first.first;
        zones[i] = it->first.second;
    }
    avtZoneIdNamedSelection *ns = new avtZoneIdNamedSelection(name);
    ns->SetIdentifiers((int)size(), doms, zones);
    delete [] doms;
    delete [] zones;
    return ns;
}

// ****************************************************************************
// Method: CQDomainZoneIdSelection::CreateSelectedDataset
//
// Purpose: 
//   Create a VTK that encodes the selection map keys..
//
// Arguments:
//   idVariable : The name of the array we'll put into the VTK dataset to
//                represent the selection.
//
// Returns:    A new VTK dataset that encodes the selection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:46:08 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
CQDomainZoneIdSelection::CreateSelectedDataset(const std::string &idVariable) const
{
    // Create a dummy dataset.
    int dims[] = {1,1,1};
    dims[0] = (int)size()+1;
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dims);
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(dims[0]);
    for(int i = 0; i < dims[0]; ++i)
        x->SetTuple1(i, i);
    rgrid->SetXCoordinates(x);
    x->Delete();

    // Store the map keys as avtOriginalCellNumbers.
    vtkUnsignedIntArray *ids = vtkUnsignedIntArray::New();
    ids->SetName(idVariable.c_str());
    ids->SetNumberOfComponents(2);
    ids->SetNumberOfTuples(size());
    unsigned int *ptr = (unsigned int *)ids->GetVoidPointer(0);
    for(const_iterator it = begin(); it != end(); ++it)
    {
        *ptr++ = it->first.first;
        *ptr++ = it->first.second;
    }
    rgrid->GetCellData()->AddArray(ids);
    ids->Delete();

    return rgrid;
}

// ****************************************************************************
// Method: CQDomainZoneIdSelection::CellIdentifierFromTuple
//
// Purpose: 
//   Return a cell identifier from a specific tuple in a data array.
//
// Arguments:
//   arr    : The VTK data array that we're using to index the cells.
//   cellid : The index of the tuple to use.
//
// Returns:    A new cell identifier.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:51:47 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

CQDomainZoneId
CQDomainZoneIdSelection::CellIdentifierFromTuple(vtkDataArray *arr, vtkIdType cellid) const
{
    const unsigned int *ptr = (const unsigned int *)arr->GetVoidPointer(0);
    return CQDomainZoneId(ptr[cellid*2], ptr[cellid*2+1]);
}

// ****************************************************************************
// Method: CQDomainZoneIdSelection::GlobalizeIds
//
// Purpose: 
//   Globalize the map keys so all processors will have the same set of keys.
//
// Arguments:
//   globalSize : The global number of map keys.
//   myOffset   : This processor's local offset into the global array.
//   allIds     : The returned globalized ids.
//
// Returns:    
//
// Note:       This implementation is specific to tuples of (domain,zone).
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:53:39 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
CQDomainZoneIdSelection::GlobalizeIds(int globalSize, int myOffset,
                                      CQDomainZoneId*&allIds) const
{
    int i = 0;
    const_iterator it;
#ifdef PARALLEL
    // Unpack the keys into arrays
    int *sendKey = new int[globalSize*2];
    memset(sendKey, 0, sizeof(int) * globalSize * 2);
    for(i = 0, it = begin(); it != end(); ++it, ++i)
    {
        int idx = (myOffset + i) * 2;
        sendKey[idx]     = it->first.first;
        sendKey[idx + 1] = it->first.second;
    }
    // Globalize
    int *allKeys = new int[globalSize * 2];
    SumIntArrayAcrossAllProcessors(sendKey, allKeys, globalSize * 2);
    delete [] sendKey;
    // Turn the arrays back into keys
    allIds = new CQDomainZoneId[globalSize];
    int *ptr = allKeys;
    for(i = 0; i < globalSize; ++i, ptr += 2)
        allIds[i] = CQDomainZoneId(ptr[0], ptr[1]);
    delete [] allKeys;
#else
    allIds = new CQDomainZoneId[size()+1];
    for(i = 0, it = begin(); it != end(); ++it, ++i)
        allIds[i] = it->first;
#endif
}

// *****************************************************************************
// *****************************************************************************
// ***
// *** CQVariableIdSelection
// ***
// *****************************************************************************
// *****************************************************************************
typedef double CQVariableId;

// ****************************************************************************
// Class: CQVariableIdSelection
//
// Purpose:
//   This class implements a selection type that lets us index cells using a
//   user-defined variable.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:44:07 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class CQVariableIdSelection : public std::map<CQVariableId, CQCellData>
{
public:
    CQVariableIdSelection() : std::map<CQVariableId, CQCellData>(), startBinID(0.), endBinID(0.)
    {
    }

    avtNamedSelection *CreateNamedSelection(const std::string &name) const;
    vtkDataSet        *CreateSelectedDataset(const std::string &idVariable) const;
    CQVariableId       CellIdentifierFromTuple(vtkDataArray *arr, vtkIdType cellid) const;
    void               GlobalizeIds(int globalSize, int myOffset, CQVariableId*&allIds) const;
    CQVariableId       startBinID, endBinID;
};

// ****************************************************************************
// Method: CQVariableIdSelection::CreateNamedSelection
//
// Purpose: 
//   Create a new named selection from the selection map keys.
//
// Arguments:
//   name : The name of the new selection.
//
// Returns:    We return a new avtFloatingPointIdNamedSelection object.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:45:14 PDT 2011
//
// Modifications:
//   
// ****************************************************************************


avtNamedSelection *
CQVariableIdSelection::CreateNamedSelection(const std::string &name) const
{
    // Create the floating point named selection
    std::vector<double> ids;
    ids.reserve(size());
    for(const_iterator it = begin(); it != end(); ++it)
        ids.push_back(it->first);
    avtFloatingPointIdNamedSelection *ns = new avtFloatingPointIdNamedSelection(name);
    ns->SetIdentifiers(ids);
    return ns;
}

// ****************************************************************************
// Method: CQVariableIdSelection::CreateSelectedDataset
//
// Purpose: 
//   Create a VTK that encodes the selection map keys..
//
// Arguments:
//   idVariable : The name of the array we'll put into the VTK dataset to
//                represent the selection.
//
// Returns:    A new VTK dataset that encodes the selection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:46:08 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
CQVariableIdSelection::CreateSelectedDataset(const std::string &idVariable) const
{
    // Create a dummy dataset
    int dims[] = {1,1,1};
    dims[0] = (int)size()+1;
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dims);
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(dims[0]);
    for(int i = 0; i < dims[0]; ++i)
        x->SetTuple1(i, i);
    rgrid->SetXCoordinates(x);
    x->Delete();

    // Encode the map keys as an array on the dataset.
    vtkDoubleArray *ids = vtkDoubleArray::New();
    ids->SetName(idVariable.c_str());
    ids->SetNumberOfTuples(size());
    double *ptr = (double *)ids->GetVoidPointer(0);
    for(const_iterator it = begin(); it != end(); ++it)
    {
        *ptr++ = it->first;
    }
    rgrid->GetCellData()->AddArray(ids);
    ids->Delete();

    return rgrid;
}

// ****************************************************************************
// Method: CQVariableIdSelection::CellIdentifierFromTuple
//
// Purpose: 
//   Return a cell identifier from a specific tuple in a data array.
//
// Arguments:
//   arr    : The VTK data array that we're using to index the cells.
//   cellid : The index of the tuple to use.
//
// Returns:    A new cell identifier.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:51:47 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

CQVariableId
CQVariableIdSelection::CellIdentifierFromTuple(vtkDataArray *arr, vtkIdType cellid) const
{
    return arr->GetTuple1(cellid);
}

// ****************************************************************************
// Method: CQVariableIdSelection::GlobalizeIds
//
// Purpose: 
//   Globalize the map keys so all processors will have the same set of keys.
//
// Arguments:
//   globalSize : The global number of map keys.
//   myOffset   : This processor's local offset into the global array.
//   allIds     : The returned globalized ids.
//
// Returns:    
//
// Note:       This implementation is specific to tuples of double.
//
// Programmer: Brad Whitlock
// Creation:   Sat Nov  5 01:53:39 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
CQVariableIdSelection::GlobalizeIds(int globalSize, int myOffset,
     CQVariableId*&allIds) const
{
    int i = 0;
    const_iterator it;
#ifdef PARALLEL
    // Unpack the keys into arrays
    double *sendKey = new double[globalSize];
    memset(sendKey, 0, sizeof(double) * globalSize);
    for(i = 0, it = begin(); it != end(); ++it, ++i)
    {
        int idx = (myOffset + i);
        sendKey[idx] = it->first;
    }
    // Globalize
    double *allKeys = new double[globalSize];
    SumDoubleArrayAcrossAllProcessors(sendKey, allKeys, globalSize);
    delete [] sendKey;
    // Turn the arrays back into keys
    allIds = new CQVariableId[globalSize];
    for(i = 0; i < globalSize; ++i)
        allIds[i] = allKeys[i];
    delete [] allKeys;
#else
    allIds = new CQVariableId[size()+1];
    for(i = 0, it = begin(); it != end(); ++it, ++i)
        allIds[i] = it->first;
#endif
}

// *****************************************************************************
// *****************************************************************************
// ***
// *** CQFilter
// ***
// *****************************************************************************
// *****************************************************************************

// ****************************************************************************
// Class: CQFilter
//
// Purpose:
//   This filter collects selected time steps and comes up with the set of cells
//   that are present in any of the time steps. That set of cells then gets
//   further reduced using different "histogramming" functions. The final,
//   reduced set of cells is returned as a new dataset which is used to construct
//   the CQ selection. 
//
// Notes:      We also extract information about the selection cells and store
//             them so we can use them in the selection summary.
//
// Programmer: Brad Whitlock
// Creation:   Mon May  9 15:24:10 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Sep  7 11:07:18 PDT 2011
//   I moved most of the guts to the CumulativeQuery class.
//
//   Brad Whitlock, Sat Nov  5 01:10:14 PDT 2011
//   I templated the class on a CQ implementation and a CQ selection type so
//   we can do CQ's that use different index variables to relate cells.
//
// ****************************************************************************

template <class CQImpl, class CQSelection>
class CQFilter : public avtTimeLoopCollectorFilter
{
public:
    CQFilter() : avtTimeLoopCollectorFilter(), props(), summary(), CQ()
    {
    }

    virtual ~CQFilter()
    {
    }

    void SetSelectionProperties(const SelectionProperties &p)
    {
        props = p;
    }

    CQImpl &GetCumulativeQuery() { return CQ; }

    const SelectionSummary &GetSummary() const { return summary; }

    virtual const char *GetType() { return "CQFilter"; }
    virtual const char *GetDescription() { return "Creating cumulative query selection"; }
    virtual bool ExecutionSuccessful() { return true; }

    virtual avtDataTree_p ExecuteAllTimesteps(std::vector<avtDataTree_p> &);

protected:
    virtual avtContract_p ModifyContract(avtContract_p contract);

private:
    SelectionProperties  props;
    SelectionSummary     summary;
    CQImpl               CQ;
};

// ****************************************************************************
// Method: CQFilter::ExecuteAllTimesteps
//
// Purpose: 
//   This method implements the filter's time iteration loop. We take in the
//   datasets for all time steps and iterate over them, calculating our
//   histogrammed selection using the CumulativeQuery object.
//
// Arguments:
//   timesteps : The data trees for each timestep.
//
// Returns:    We return a data tree with a dummy dataset that has the number
//             of cells in our final selection and we place the original zones
//             array on it since that is what is used later to define the
//             named selection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 14:28:53 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Sep  7 11:04:34 PDT 2011
//   I moved the guts of the function into the CumulativeQuery class.
//
//   Brad Whitlock, Sat Nov  5 01:10:14 PDT 2011
//   I templated the class on a CQ implementation and a CQ selection type so
//   we can do CQ's that use different index variables to relate cells.
//
// ****************************************************************************

template <class CQImpl, class CQSelection>
avtDataTree_p
CQFilter<CQImpl,CQSelection>::ExecuteAllTimesteps(std::vector<avtDataTree_p> &timesteps)
{
    CQSelection selection;

    summary = SelectionSummary();

    doubleVector hist;
    double min, max;

    CQ.CreateSelection(this->props, timesteps, selection, hist, min, max);

    // Save the histogram values in the summary.
    summary.SetHistogramValues(hist);

//     if(this->props.GetHistogramType() == SelectionProperties::HistogramTime)
//     {
//       outSummary.SetHistogramMinBin( props.GetMinTimeState() );

//       if(this->props.GetMaxTimeState() < 0)
//      summary.SetHistogramMaxBin( this->dob->GetInfo().GetAttributes().GetNumStates() - 1 );
//       else
//      summary.SetHistogramMaxBin( this->props.GetMaxTimeState() );
//     }
//    else
      if(this->props.GetHistogramType() == SelectionProperties::HistogramMatches ||
         this->props.GetHistogramType() == SelectionProperties::HistogramVariable )
    {
      summary.SetHistogramMinBin(min);
      summary.SetHistogramMaxBin(max);
    }

    //
    // Now that we have the final set of local cells derived from the selection,
    // we create an output dataset that identifies this processor's portion
    // of the cells in the selection.
    //
    vtkDataSet *ds = selection.CreateSelectedDataset(CQ.GetIdVariable());
    avtDataTree_p outputTree = new avtDataTree(ds, -1);
    ds->Delete();

    return outputTree;
}

// ****************************************************************************
// Method: CQFilter::ModifyContract
//
// Purpose: 
//   We use this method to make sure that the histogram variable that we want
//   to use has been requested in the contract.
//
// Arguments:
//   contract : The input contract.
//
// Returns:    The output contract.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 10:50:51 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Oct 12 12:09:52 PDT 2011
//   I changed histogram variable from an index to a string.
//
//   Brad Whitlock, Sat Nov  5 01:10:14 PDT 2011
//   I templated the class on a CQ implementation and a CQ selection type so
//   we can do CQ's that use different index variables to relate cells.
//
// ****************************************************************************

template <class CQImpl, class CQSelection>
avtContract_p
CQFilter<CQImpl,CQSelection>::ModifyContract(avtContract_p contract)
{
    avtContract_p newContract = avtTimeLoopCollectorFilter::ModifyContract(contract);

    // We may need to extract another variable.
    if(props.GetHistogramType() == SelectionProperties::HistogramVariable)
    {
        const std::string &histVar = props.GetHistogramVariable();
        debug5 << "We're doing histogram by variable " << histVar << endl;

        std::string origvar(newContract->GetDataRequest()->GetOriginalVariable());
        if(origvar != histVar &&
           !newContract->GetDataRequest()->HasSecondaryVariable(histVar.c_str()))
        {
            debug5 << "Making sure we add " << histVar
                   << " to the contract so we'll have it" << endl;
            newContract->GetDataRequest()->AddSecondaryVariable(histVar.c_str());
        }
    }

    return newContract;
}

// *****************************************************************************
// *****************************************************************************
// ***
// *** CumulativeQueryCacheItem 
// ***
// *****************************************************************************
// *****************************************************************************

// ****************************************************************************
// Class: CumulativeQueryCacheItem
//
// Purpose:
//   This class contains intermediate results for CumulativeQuery that we can
//   store in the avtNamedSelectionManager's cache.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 13:47:24 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 01:11:50 PDT 2011
//   I templated the class on the CQ selection type so we can store different
//   types of selections in the cache.
//
// ****************************************************************************

template <class CQSelection>
class CumulativeQueryCacheItem : public avtNamedSelectionCacheItem
{
public:
    CumulativeQueryCacheItem() : avtNamedSelectionCacheItem(), 
        cellFrequency(), cellsPerTimeStep(), summary()
    {
    }

    virtual ~CumulativeQueryCacheItem()
    {
        debug5 << "Deleting CumulativeQueryCacheItem." << endl;
    }

    CQSelection      cellFrequency;
    std::vector<CQSelection> cellsPerTimeStep;
    SelectionSummary summary;
};

// *****************************************************************************
// *****************************************************************************
// ***
// *** CumulativeQueryNamedSelectionExtension 
// ***
// *****************************************************************************
// *****************************************************************************

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::CumulativeQueryNamedSelectionExtension
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 13:48:25 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

CumulativeQueryNamedSelectionExtension::CumulativeQueryNamedSelectionExtension() : 
    avtNamedSelectionExtension()
{
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::~CumulativeQueryNamedSelectionExtension
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 13:48:39 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

CumulativeQueryNamedSelectionExtension::~CumulativeQueryNamedSelectionExtension()
{
}

// ****************************************************************************
// Function: CreateSelectionKey
//
// Purpose: 
//   Determine a key to use for caching, given the selection properties.
//
// Arguments:
//   props : The selection properties.
//
// Returns:    A string that can be used for a cache key.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 13:48:54 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 01:13:39 PDT 2011
//   I turned it into a static function.
//
// ****************************************************************************

static std::string
CreateSelectionKey(const SelectionProperties &props)
{
    // implement me... For now return the name. We could work more information into the name
    // to allow us to cache multiple intermediate results for a single selection name.
    return props.GetName();
}

// ****************************************************************************
// Function: CheckProperties
//
// Purpose: 
//   Check the old properties vs the new properties to see if we can use the
//   intermediate results that the old properties represent.
//
// Arguments:
//   newProps : The new selection properties.
//   oldProps : The old selection properties.
//
// Returns:    True if the new and old selection properties are sufficiently
//             the same such that any intermediate selection results can be
//             reused to compute the selection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 15:06:55 PDT 2011
//
// Modifications:
//   Brad Whitlock, Wed Oct 12 12:11:01 PDT 2011
//   I changed histogram variable to a string from an index.
//
//   Brad Whitlock, Fri Oct 28 11:33:05 PDT 2011
//   Account for id variable.
//
//   Brad Whitlock, Sat Nov  5 01:13:39 PDT 2011
//   I turned it into a static function.
//
// ****************************************************************************

static bool
CheckProperties(const SelectionProperties &newProps, const SelectionProperties &oldProps)
{
    bool basicsSame = (newProps.GetName() == oldProps.GetName()) &&
                      (newProps.GetSource() == oldProps.GetSource()) &&
                      (newProps.GetSelectionType() == oldProps.GetSelectionType());

    bool idvarSame = newProps.GetIdVariableType() == oldProps.GetIdVariableType();
    if(idvarSame && newProps.GetIdVariableType() == SelectionProperties::UseVariableForID)
        idvarSame = newProps.GetIdVariable() == oldProps.GetIdVariable();

    bool varsSame = (newProps.GetVariables() == oldProps.GetVariables()) &&
                    (newProps.GetVariableMins() == oldProps.GetVariableMins()) &&
                    (newProps.GetVariableMaxs() == oldProps.GetVariableMaxs());

    bool timeSame = (newProps.GetMinTimeState() == oldProps.GetMinTimeState()) &&
                    (newProps.GetMaxTimeState() == oldProps.GetMaxTimeState()) &&
                    (newProps.GetTimeStateStride() == oldProps.GetTimeStateStride());

    bool typeCloseEnough = true;
    if(newProps.GetHistogramType() == oldProps.GetHistogramType())
    {
        if(newProps.GetHistogramType() == SelectionProperties::HistogramVariable &&
           newProps.GetHistogramVariable() != oldProps.GetHistogramVariable())
        {
            typeCloseEnough = false;
        }
    }

    return basicsSame && idvarSame && varsSame && timeSame && typeCloseEnough;
}

// ****************************************************************************
// Function: BuildSummary
//
// Purpose: 
//   Retrieve the histograms we've calculated in filters and store them into a
//   summary object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 14:47:17 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Sep  6 15:41:23 PDT 2011
//   I renamed the method to BuildSummary and removed code to delete filters
//   or set the internal summary object.
//
//   Brad Whitlock, Sat Nov  5 01:13:39 PDT 2011
//   I turned it into a static function.
//
// ****************************************************************************

static SelectionSummary
BuildSummary(const CQHistogramCalculationFilter &hist)
{
    const char *mName = "CumulativeQueryNamedSelectionExtension::BuildSummary: ";

    // Stash hist's histograms into this class so we can return them from the engine.
    SelectionSummary s;
    stringVector vars = hist.GetVariables();
    for(size_t i = 0; i < vars.size(); ++i)
    {
        const CQHistogramCalculationFilter::Histogram *h = hist.GetHistogram(vars[i]);
        if(h != 0)
        {
            SelectionVariableSummary vs;
            vs.SetName(vars[i]);
            vs.SetMinimum(h->minimum);    
            vs.SetMaximum(h->maximum);    
            vs.SetHistogram(h->frequency);    
            s.AddVariables(vs);
        }
        else
        {
            debug5 << mName << "Could not locate histogram for " << vars[i]
                   << " so create a default histogram." << endl;
            double frequency[256];
            memset(frequency, 0, 256 * sizeof(double));

            SelectionVariableSummary vs;
            vs.SetName(vars[i]);
            vs.SetMinimum(0.);
            vs.SetMaximum(1.);    
            vs.SetHistogram(frequency);    
            s.AddVariables(vs);
        }
    }

    // Set the total cell count. Note that we now divide that number by the number
    // of time steps that were processed because the time loop filter causes our
    // local histogram filter that counts the number of original cells to sum the
    // total number of cells for all time steps. This gives us the per time step avg.
    debug5 << mName << "total cells = " << hist.GetTotalCellCount() << endl;
    s.SetTotalCellCount((int)hist.GetTotalCellCount());

    return s;
}

// ****************************************************************************
// Function: GetSelectionEx
//
// Purpose: 
//   Compute a cumulative query selection based on the input data object and
//   return the selection.
//
// Arguments:
//   dob        : The input data object (We don't always use it).
//   contract   : The contract that we'll use if we need to reexecute the pipeline.  
//   props      : The selection properties.
//   cache      : The NSM's cache.
//   cqFilter   : The CQ filter implementation we want to use.
//   selection  : The CQ selection that we want to use.
//   outSummary : Return the selection summary in this object.
//
// Returns:    A named selection containing the pieces we care about.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 12:37:13 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

template <class CQFilterType, class CQSelection>
static avtNamedSelection *
GetSelectionEx(avtDataObject_p dob, avtContract_p contract, const SelectionProperties &props, 
    avtNamedSelectionCache &cache, CQFilterType &cqFilter, CQSelection &selection, 
    SelectionSummary &outSummary)
{
    const char *mName = "CumulativeQueryNamedSelectionExtension::GetSelection: ";
    avtNamedSelection *ns = NULL;

    // Check the selection cache for intermediate data we can use so we do not
    // have to execute the CQ over time.
    std::string selectionKey = CreateSelectionKey(props);
    CumulativeQueryCacheItem<CQSelection> *intermediateResults = NULL;
    if(cache.exists(selectionKey))
    {
        debug1 << mName << "Found intermediate selection data for "
               << props.GetName() << endl;
        intermediateResults = (CumulativeQueryCacheItem<CQSelection> *)cache[selectionKey];

        // Check to see if the intermediate results are okay to use.
        if(!CheckProperties(props, intermediateResults->properties))
        {
            debug1 << mName << "Intermediate selection data was found but "
                               "it could not be used." << endl;
            intermediateResults = NULL;
        }
    }

    if(intermediateResults != NULL)
    {
        StackTimer t1("Calculating CQ selection with intermediate data");

        debug1 << mName << "Calculating selection " << props.GetName()
               << " using intermediate results." << endl;

        // Intermediate data exists. Let's just redo the histogramming part 
        // of CQ so we don't have to do time iteration again.
        doubleVector hist;
        double min, max;

        cqFilter.GetCumulativeQuery().CreateSelection(props, 
            intermediateResults->cellFrequency,
            intermediateResults->cellsPerTimeStep, selection, hist, min, max);

        // Now that we have a selection, convert it into a named selection object.
        ns = selection.CreateNamedSelection(props.GetName());

        // Save the histogram values in the summary.
        outSummary = intermediateResults->summary;
        outSummary.SetHistogramValues(hist);

        if(props.GetHistogramType() == SelectionProperties::HistogramTime)
        {
          outSummary.SetHistogramMinBin( props.GetMinTimeState() );

          if(props.GetMaxTimeState() < 0)
            outSummary.SetHistogramMaxBin( dob->GetInfo().GetAttributes().GetNumStates() - 1 );
          else
            outSummary.SetHistogramMaxBin( props.GetMaxTimeState() );
        }
        else if(props.GetHistogramType() == SelectionProperties::HistogramMatches ||
                props.GetHistogramType() == SelectionProperties::HistogramVariable )
        {
          outSummary.SetHistogramMinBin(min);
          outSummary.SetHistogramMaxBin(max);
        }
    }
    else
    {
        StackTimer t1("Calculating CQ selection");

        debug1 << mName << "Calculating selection " << props.GetName()
               << " from scratch." << endl;

        // Add in our custom filters.
        avtDataObject_p newdob(dob);
        CQHistogramCalculationFilter hist;
        avtThresholdFilter           threshold;
        if(!props.GetVariables().empty())
        {
            // Set up the variable that we're going to use to threshold.
            SelectionProperties prop2(props);
            if(prop2.GetMaxTimeState() < 0)
                prop2.SetMaxTimeState(dob->GetInfo().GetAttributes().GetNumStates()-1);

            //
            // Add a CQHistogramCalculationFilter so we can calculate histograms for
            // the variables that we care about.
            //
            hist.SetInput(newdob);
            for(size_t i = 0; i < prop2.GetVariables().size(); ++i)
                hist.AddVariable(prop2.GetVariables()[i]);
            newdob = hist.GetOutput(); 

            //
            // Add a threshold operator to the pipeline, thresholding out the variables
            // in our CQ selection.
            //
            ThresholdOpAttributes thresholdAtts;
            if(!props.GetVariables().empty())
                thresholdAtts.SetDefaultVarName(props.GetVariables()[0]);
            thresholdAtts.SetListedVarNames(props.GetVariables());
            thresholdAtts.SetLowerBounds(props.GetVariableMins());
            thresholdAtts.SetUpperBounds(props.GetVariableMaxs());

            threshold.SetInput(newdob);
            threshold.SetAtts(&thresholdAtts);
            newdob = threshold.GetOutput();

            //
            // Add our time iteration CQ filter.
            //
            cqFilter.SetInput(newdob);
            cqFilter.SetTimeLoop(prop2.GetMinTimeState(),
                                 prop2.GetMaxTimeState(),
                                 prop2.GetTimeStateStride());
            cqFilter.SetSelectionProperties(prop2);
            newdob = cqFilter.GetOutput();
        }

        // Execute the full pipeline.
        debug1 << mName << "Must re-execute pipeline to create named selection" << endl;
        TimedCodeBlock("CQ Pipeline update",
            newdob->Update(contract);
        );
        debug1 << mName << "Done re-executing pipeline to create named selection" << endl;
    
        // Extract the selection from the data tree.
        avtDataset_p ds;
        CopyTo(ds, newdob);
        ns = avtNamedSelectionExtension::GetSelectionFromDataset(ds, props);

        // Extract data from the filters and stick it in the summary. 
        outSummary = BuildSummary(hist);
        outSummary.SetHistogramValues(cqFilter.GetSummary().GetHistogramValues());

        if(props.GetHistogramType() == SelectionProperties::HistogramTime)
        {
          outSummary.SetHistogramMinBin( props.GetMinTimeState() );

          if(props.GetMaxTimeState() < 0)
            outSummary.SetHistogramMaxBin( dob->GetInfo().GetAttributes().GetNumStates() - 1 );
          else
            outSummary.SetHistogramMaxBin( props.GetMaxTimeState() );
        }
        else if(props.GetHistogramType() == SelectionProperties::HistogramMatches ||
                props.GetHistogramType() == SelectionProperties::HistogramVariable )
        {
          outSummary.SetHistogramMinBin(cqFilter.GetSummary().GetHistogramMinBin());
          outSummary.SetHistogramMaxBin(cqFilter.GetSummary().GetHistogramMaxBin());
        }

        // Cache the intermediate selection & summary.
        debug1 << mName << "Caching intermediate results for selection" << endl;
        CumulativeQueryCacheItem<CQSelection> *item = new CumulativeQueryCacheItem<CQSelection>;
        item->properties = props;
        item->cellFrequency = cqFilter.GetCumulativeQuery().GetCellFrequency();
        item->cellsPerTimeStep = cqFilter.GetCumulativeQuery().GetCellsPerTimeStep();
        item->summary = outSummary;
        cache[selectionKey] = item;
    }

    return ns;
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::GetSelection
//
// Purpose: 
//   Compute a cumulative query selection based on the input data object and
//   return the selection.
//
// Arguments:
//   dob   : The input data object (We don't always use it).
//   props : The selection properties.
//   cache : The NSM's cache.
//
// Returns:    A named selection containing the pieces we care about.
//
// Note:       This function calls GetSelectionEx with different CQ 
//             implementations so we can do CQ with different cell indexing
//             schemes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 15:41:41 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

avtNamedSelection *
CumulativeQueryNamedSelectionExtension::GetSelection(avtDataObject_p dob,
    const SelectionProperties &props, avtNamedSelectionCache &cache)
{
    avtNamedSelection *ns = NULL;

    avtContract_p c0 = dob->GetContractFromPreviousExecution();
    bool needsUpdate = false;
    avtContract_p contract = ModifyContract(c0, props, needsUpdate);

    if(props.GetIdVariableType() == SelectionProperties::UseZoneIDForID)
    {
        CQFilter<CumulativeQuery<CQDomainZoneId, CQDomainZoneIdSelection>,
                 CQDomainZoneIdSelection
                > cqFilter;
        cqFilter.GetCumulativeQuery().SetIdVariable(GetIdVariable(props));
        CQDomainZoneIdSelection selection;
        ns = GetSelectionEx(dob, contract, props, cache, cqFilter, selection, summary);

        if(props.GetHistogramType() == SelectionProperties::HistogramID)
        {
          summary.SetHistogramMinBin(selection.startBinID.second);
          summary.SetHistogramMaxBin(selection.endBinID.second);
        }
    }
    else if(props.GetIdVariableType() == SelectionProperties::UseGlobalZoneIDForID)
    {
// This one might be VISIT_LONG_LONG for the index type. Check it...
        CQFilter<CumulativeQuery<CQVariableId, CQVariableIdSelection>,
                 CQVariableIdSelection
                > cqFilter;
        cqFilter.GetCumulativeQuery().SetIdVariable(GetIdVariable(props));
        CQVariableIdSelection selection;
        ns = GetSelectionEx(dob, contract, props, cache, cqFilter, selection, summary);

        if(props.GetHistogramType() == SelectionProperties::HistogramID)
        {
          summary.SetHistogramMinBin(selection.startBinID);
          summary.SetHistogramMaxBin(selection.endBinID);
        }
    }
    else if(props.GetIdVariableType() == SelectionProperties::UseVariableForID)
    {
        CQFilter<CumulativeQuery<CQVariableId, CQVariableIdSelection>,
                 CQVariableIdSelection
                > cqFilter;
        cqFilter.GetCumulativeQuery().SetIdVariable(GetIdVariable(props));
        CQVariableIdSelection selection;
        ns = GetSelectionEx(dob, contract, props, cache, cqFilter, selection, summary);

        if(props.GetHistogramType() == SelectionProperties::HistogramID)
        {
          summary.SetHistogramMinBin(selection.startBinID);
          summary.SetHistogramMaxBin(selection.endBinID);
        }
    }

    if(ns != NULL)
        ns->SetIdVariable(GetIdVariable(props));

    return ns;
}


// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::GetSelectionSummary
//
// Purpose: 
//   Get the selection summary.
//
// Returns:    Get the selection summary.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 14:55:13 PST 2010
//
// Modifications:
//   
// ****************************************************************************

const SelectionSummary &
CumulativeQueryNamedSelectionExtension::GetSelectionSummary() const
{
    return summary;
}
