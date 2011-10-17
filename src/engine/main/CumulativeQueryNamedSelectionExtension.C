/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
        std::map<std::string,Histogram*>::const_iterator it = histograms.find(var);
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

    virtual vtkDataSet *ExecuteData(vtkDataSet *ds, int dom, std::string)
    {
        cellCount += ds->GetNumberOfCells();

        return ds;
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
            double minmax[2] = {0.,1.};
#if 0
            avtExtents *ext = dob->GetInfo().GetAttributes().
                GetThisProcsOriginalDataExtents(it->first.c_str());
            if(ext != 0)
                ext->CopyTo(minmax);
            else
#endif
            // If we have not created a histogram yet for this variable then do it now.
            if(it->second == 0)
            {
                // Get the extents
                avtDatasetExaminer::GetDataExtents(ds, minmax, it->first.c_str());
                debug5 << mName << "Calculated data extents for " << it->first
                       << " [" << minmax[0] << ", " << minmax[1] << "]" << endl;

                std::vector<VISIT_LONG_LONG> hist(256, 0);
                if(!avtDatasetExaminer::CalculateHistogram(ds, it->first, minmax[0], minmax[1], hist))
                    debug1 << "CalculateHistogram failed" << endl;
                
                it->second = new Histogram;
                for(int i = 0; i < 256; ++i)
                    it->second->frequency[i] = (double)hist[i];

                // Print the histogram.
                debug5 << mName << "Calculated histogram for " << it->first << endl;
                for(int i = 0; i < 256; ++i)
                    debug5 << "\thist[" << i << "] = " << hist[i] << endl;
            }
            it->second->minimum = minmax[0];
            it->second->maximum = minmax[1];
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
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 11:30:52 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class CumulativeQuery
{
public:
    struct CQCellData
    {
        int    frequency;
        double variable;
    };

    typedef std::pair<unsigned int, unsigned int>  CQCellIdentifier;
    typedef std::map<CQCellIdentifier, CQCellData> CQCellIdentifierCQCellDataMap;

    CumulativeQuery() : cellFrequency(), cellsPerTimestep()
    {
    }

    virtual ~CumulativeQuery()
    {
    }

    // The CQFilter class uses this method to supply data objects from which we
    // calculate the selection. We calculate the cellFrequency and then the selection.
    void CreateSelection(const SelectionProperties &props,
                         std::vector<avtDataTree_p> &, 
                         CQCellIdentifierCQCellDataMap &outSel,
                         doubleVector &hist);

    // Calculate the selection but use cellFrequency data we already calculated.
    void CreateSelection(const SelectionProperties &props,
                         const CQCellIdentifierCQCellDataMap &cellFrequency,
                         const intVector &cellsPerTimestep,
                         CQCellIdentifierCQCellDataMap &narrowedSelection,
                         doubleVector &hist);

    const CQCellIdentifierCQCellDataMap &GetCellFrequency() const { return cellFrequency; }
    const intVector                     &GetCellsPerTimestep() const { return cellsPerTimestep; }

private:
    void CalculateFrequency(const SelectionProperties &props,
                            std::vector<avtDataTree_p> &timesteps,
                            CQCellIdentifierCQCellDataMap &cellFrequency,
                            intVector &cellsPerTimestep) const;

    void Summation(const SelectionProperties &props,
                   unsigned int nts, 
                   const CQCellIdentifierCQCellDataMap &cellFrequency,
                   CQCellIdentifierCQCellDataMap &selection) const;

    bool GlobalizeSelection(const CQCellIdentifierCQCellDataMap &selection,
                            int *&allDomains, int *&allCellIds, int *&allFrequencies, double *&allVariables,
                            int &totalCells) const;

    void SelectAndHistogram(const SelectionProperties &props,
                            const int *allDomains, const int *allCellIds, 
                            const int *allFrequencies, const double *allVariables,
                            int totalCells,
                            const CQCellIdentifierCQCellDataMap &selection,
                            CQCellIdentifierCQCellDataMap &narrowedSelection,
                            doubleVector &histogram) const;

    // Members that we calculate as stage 1 of the selection calculation during
    // frequency calculation.
    CQCellIdentifierCQCellDataMap cellFrequency;
    intVector                     cellsPerTimestep;
};

#if 0
// Debugging function to save out selections to a text file.
void
SaveSelection(const char *filename, const CumulativeQuery::CQCellIdentifierCQCellDataMap &sel)
{
    FILE *f = fopen(filename, "wt");
    if(f != NULL)
    {
        CumulativeQuery::CQCellIdentifierCQCellDataMap::const_iterator it;
        for(it = sel.begin(); it != sel.end(); ++it)
            fprintf(f, "%d %d %d %lg\n", it->first.first, it->first.second, it->second.frequency, it->second.variable);

        fclose(f);
    }
}
#endif

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
//   
// ****************************************************************************

void
CumulativeQuery::CreateSelection(const SelectionProperties &props,
    std::vector<avtDataTree_p> &timesteps, 
    CQCellIdentifierCQCellDataMap &narrowedSelection,
    doubleVector &hist)
{
    StackTimer t0("CreateSelection: phase 1");
    cellFrequency.clear();
    cellsPerTimestep.clear();

    //
    // Iterate through the data for all time steps and come up with a 
    // "cell frequency". We may find different cell ids over time so we
    // create a set that includes any cell that was present in any dataset
    // over time. Each time we find that cell in a different time step,
    // we increment its "cell frequency". The cell frequency thus becomes
    // a count of how many time steps contained the cell.
    //
    CalculateFrequency(props, timesteps, cellFrequency, cellsPerTimestep);
//    SaveSelection("selection0.txt", cellFrequency);

    // Now that we know the cell frequency and the cells per time step, we
    // can pass them into the next phase of the selection creation.
    CreateSelection(props, cellFrequency, cellsPerTimestep, 
                    narrowedSelection, hist);
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
//   cellsPerTimestep  : The number of cells in each timestep.
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
//   
// ****************************************************************************

void
CumulativeQuery::CreateSelection(const SelectionProperties &props,
    const CQCellIdentifierCQCellDataMap &cellFrequency,
    const intVector &cellsPerTimestep,
    CQCellIdentifierCQCellDataMap &narrowedSelection,
    doubleVector &hist)
{
    StackTimer t0("CreateSelection: phase 2");

    //
    // Apply the summation rule so we get a subset of the cellFrequency
    // map that will contribute to our selection.
    //
    CQCellIdentifierCQCellDataMap selection;
    Summation(props, cellsPerTimestep.size(), cellFrequency, selection);
//    SaveSelection("selection1.txt", selection);

#ifdef PARALLEL
    //
    // We have a number of cells per timestep for this processor. We need to
    // sum those values across processors so we have the whole picture.
    //
    int *cpt = new int[cellsPerTimestep.size()];
    memcpy(cpt, &cellsPerTimestep[0], sizeof(int) * cellsPerTimestep.size());
    intVector cellsPerTimestepTotal(cellsPerTimestep.size(), 0);
    SumIntArrayAcrossAllProcessors(cpt, 
        &cellsPerTimestepTotal[0], cellsPerTimestep.size());
    delete [] cpt;
#else
    intVector cellsPerTimestepTotal(cellsPerTimestep);
#endif

    //
    // Let's combine the selections from all processors and then send that
    // global selection to all processors, in the process transforming it into
    // a set of arrays.
    //
    int *allDomains = 0, *allCellIds = 0, *allFrequencies = 0;
    double *allVariables = 0;
    int totalCells = 0;
    if(!GlobalizeSelection(selection, 
                           allDomains, allCellIds, allFrequencies, allVariables,
                           totalCells))
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
        narrowedSelection = selection;

        for(size_t i = 0; i < cellsPerTimestepTotal.size(); ++i)
            hist.push_back(double(cellsPerTimestepTotal[i]));
    }
    else
    {
        SelectAndHistogram(props,
                           allDomains, allCellIds, allFrequencies, allVariables, 
                           totalCells,
                           selection, narrowedSelection, hist);
    }
    selection.clear();

    delete [] allDomains;
    delete [] allCellIds;
    delete [] allFrequencies;
    delete [] allVariables;
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
//   cellsPerTimestep : The number of cells in each time step that match
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
// ****************************************************************************

void
CumulativeQuery::CalculateFrequency(const SelectionProperties &props,
    std::vector<avtDataTree_p> &timesteps,
    CumulativeQuery::CQCellIdentifierCQCellDataMap &cellFrequency, 
    intVector &cellsPerTimestep) const
{
    const char *mName = "CQFilter::CalculateFrequency: ";
    StackTimer t0("CalculateFrequency");

    CQCellIdentifierCQCellDataMap::const_iterator it;

    // Make sure that the cellsPerTimestep vector is initialized properly.
    cellsPerTimestep = intVector(timesteps.size(), 0);   

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
        CQCellIdentifierCQCellDataMap thisTS;
        for (int i = 0 ; i < nleaves ; i++)
        {
            vtkIdType ncells = leaves[i]->GetNumberOfCells();
            if(ncells == 0)
                continue;

            // Get the original zone numbers
            vtkDataArray *ocn = leaves[i]->GetCellData()->
                                    GetArray("avtOriginalCellNumbers");
            if (ocn == NULL)
            {
                debug5 << mName << "Could not locate original cells. This "
                    "dataset will not contribute to the selection." << endl;
                continue;
            }
            unsigned int *ptr = (unsigned int *) ocn->GetVoidPointer(0);
            if (ptr == NULL)
            {
                debug5 << mName << "Could not locate original cells. This "
                    "dataset will not contribute to the selection." << endl;
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
                        pd2cd->SetInput(leaves[i]);
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

            for(vtkIdType j = 0; j < ncells; ++j, ptr += 2)
            {
                CQCellIdentifier cellId(ptr[0], ptr[1]);
                CQCellData c;
                c.frequency = 1;
                c.variable = var ? var->GetTuple1(j) : 0.;

                CQCellIdentifierCQCellDataMap::iterator p = thisTS.find(cellId);
                if(p == thisTS.end())
                    thisTS[cellId] = c;
                else
                {
                    // we have a cell we've seen before so we must have fragments
                    // of that cell so sum the variables.
                    p->second.variable += c.variable;
                }
            }

            if(cellVar != NULL)
                cellVar->Delete();
        }

        delete [] leaves;

        // Merge the results from the current time step into the overall
        // cell frequency map. We do this so there are no duplicates from
        // cells that may have been split up previously. This way, cell fragments
        // do not impact the frequency.
        for(it = thisTS.begin(); it != thisTS.end(); ++it)
        {
            CQCellIdentifierCQCellDataMap::iterator p;
            p = cellFrequency.find(it->first);
            if(p == cellFrequency.end())
            {
                // insert new cell
                cellFrequency[it->first] = it->second;
            }
            else
            {
                // we've seen this cell before in another time step so increment.
                p->second.frequency += 1;
            }
        }

        // Record the number of cells for this time step
        cellsPerTimestep[ts] = thisTS.size();
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
//   nts           : The number of time steps used in the selection.
//   cellFrequency : The number of times each cell was found in the datasets
//                   over time.
//   selection     : The new "summed" selection.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 11:04:39 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
CumulativeQuery::Summation(const SelectionProperties &props,
    unsigned int nts, 
    const CumulativeQuery::CQCellIdentifierCQCellDataMap &cellFrequency,
    CumulativeQuery::CQCellIdentifierCQCellDataMap &selection) const
{
    StackTimer t0("Summation");

    if(props.GetCombineRule() == SelectionProperties::CombineAnd)
    {
        CQCellIdentifierCQCellDataMap::const_iterator it;
        for(it = cellFrequency.begin(); it != cellFrequency.end(); ++it)
        {
            if(it->second.frequency == nts)
                selection[it->first] = it->second;
        }
    }
    else
        selection = cellFrequency;
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
//   
// ****************************************************************************

bool
CumulativeQuery::GlobalizeSelection(
    const CQCellIdentifierCQCellDataMap &selection,
    int *&allDomains, int *&allCellIds, int *&allFrequencies, double *&allVariables,
    int &totalCells) const
{
    StackTimer t0("GlobalizeSelection");

    int i = 0;
    CQCellIdentifierCQCellDataMap::const_iterator it;
#ifdef PARALLEL
    // Figure out how many cells are in the summed selection by adding the selection
    // sizes over all processors.
    int nCellsTotal = (int)selection.size();
    SumIntAcrossAllProcessors(nCellsTotal);
    if(nCellsTotal > avtNamedSelectionManager::MaximumSelectionSize())
    {
        debug5 << "The number of total cells (" << nCellsTotal
               << ") is above the allowed limit." << endl;
        return false;
    }

    //
    // Let's get all of the selected cells from all processors to rank 0.
    //
    int *receiveCounts = 0;
    int *sendDomains = new int[selection.size()+1];
    int *sendCellIds = new int[selection.size()+1];
    int *sendFrequencies = new int[selection.size()+1];
    double *sendVariables = new double[selection.size() + 1];
    for(it = selection.begin(); it != selection.end(); ++it, ++i)
    {
        sendDomains[i] = it->first.first;
        sendCellIds[i] = it->first.second;
        sendFrequencies[i] = it->second.frequency;
        sendVariables[i] = it->second.variable;
    }
    int *recvCounts = 0;
    CollectIntArraysOnRootProc(allDomains, recvCounts, sendDomains, selection.size());
    delete [] recvCounts;
    CollectIntArraysOnRootProc(allCellIds, recvCounts, sendCellIds, selection.size());
    delete [] recvCounts;
    CollectIntArraysOnRootProc(allFrequencies, recvCounts, sendFrequencies, selection.size());
    delete [] recvCounts;
    CollectDoubleArraysOnRootProc(allVariables, recvCounts, sendVariables, selection.size());
    delete [] sendDomains;
    delete [] sendCellIds;
    delete [] sendFrequencies;
    delete [] sendVariables;

    // Count the number of cells in the whole selection.
    totalCells = 0;
    if(PAR_Rank() == 0)
    {
        for(i = 0; i < PAR_Size(); ++i)
            totalCells += recvCounts[i];
    }
    delete [] recvCounts;
    SumIntAcrossAllProcessors(totalCells);

    // Send the entire selection back to all processors
    if(PAR_Rank() != 0)
    {
        allDomains = new int[totalCells];
        allCellIds = new int[totalCells];
        allFrequencies = new int[totalCells];
        allVariables = new double[totalCells];
    }
    BroadcastIntArray(allDomains, totalCells);
    BroadcastIntArray(allCellIds, totalCells);
    BroadcastIntArray(allFrequencies, totalCells);
    BroadcastDoubleArray(allVariables, totalCells);
#else
    // Convert the selection into "all" arrays.
    totalCells = selection.size();
    allDomains = new int[totalCells];
    allCellIds = new int[totalCells];
    allFrequencies = new int[totalCells];
    allVariables = new double[totalCells];
    for(it = selection.begin(); it != selection.end(); ++it, ++i)
    {
        allDomains[i] = it->first.first;
        allCellIds[i] = it->first.second;
        allFrequencies[i] = it->second.frequency;
        allVariables[i] = it->second.variable;
    }
#endif

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
//   allDomains        : The array containing the domains for the global selection.
//   allCellIds        : The array containing the cellids for the global selection.
//   allFrequencies    : the array containing the frequencies for the global selection.
//   allVariables      : The array containing the variables for the global selection.
//   totalCells        : The number of cells in the selection.
//   selection         : The local selection.
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
//   
// ****************************************************************************

void
CumulativeQuery::SelectAndHistogram(const SelectionProperties &props,
    const int *allDomains, const int *allCellIds, 
    const int *allFrequencies, const double *allVariables, int totalCells,
    const CumulativeQuery::CQCellIdentifierCQCellDataMap &selection,
    CumulativeQuery::CQCellIdentifierCQCellDataMap &narrowedSelection,
    doubleVector &histogram) const
{
    const char *mName = "CQFilter::SelectAndHistogram: ";
    StackTimer t0("SelectAndHistogram");

    debug5 << mName << "start" << endl;

    // Create an index variable that we'll sort using another variable.
    int *index = new int[totalCells];
    for(int i = 0; i < totalCells; ++i)
        index[i] = i;

    //
    // Sort the index based on different histogram criteria
    //
    if(props.GetHistogramType() == SelectionProperties::HistogramMatches)
    {
        // Sort the index based on frequency
        debug5 << mName << "Sorting index based on frequency" << endl;
        std::sort(index, index + totalCells, cq_sort_by_value<int>(allFrequencies));
    }
    else if(props.GetHistogramType() == SelectionProperties::HistogramID)
    {
        // Sort the index based on cellid
        debug5 << mName << "Sorting index based on cellid" << endl;
        std::sort(index, index + totalCells, cq_sort_by_value<int>(allCellIds));
    }
    else if(props.GetHistogramType() == SelectionProperties::HistogramVariable)
    {
        // Sort the index based on variable
        debug5 << mName << "Sorting index based on variable" << endl;
        std::sort(index, index + totalCells, cq_sort_by_value<double>(allVariables));
    }

    // We need to bin the index array into some number of bins and select
    // the cells in the selected bins into our new narrowed selection.
    int *binPoints = NULL, numBins = 0;

    // For IDs the bins are based on the number of cells
    if(props.GetHistogramType() == SelectionProperties::HistogramID)
    {
        numBins = props.GetHistogramNumBins();
        binPoints = new int[numBins + 1];

        for(int i = 0; i < numBins+1; ++i)
        {
            float t = float(i) / float(numBins);
            binPoints[i] = int(t * float(totalCells));
        }
    }

    else if(props.GetHistogramType() == SelectionProperties::HistogramMatches ||
            props.GetHistogramType() == SelectionProperties::HistogramVariable)
    {

        double min, max;

        if(props.GetHistogramType() == SelectionProperties::HistogramMatches)
        {
            min = allFrequencies[index[0]];
            max = allFrequencies[index[totalCells-1]];

            numBins = max-min+1;
            binPoints = new int[numBins + 1];
        }
        else if( props.GetHistogramType() == SelectionProperties::HistogramVariable)
        {
            numBins = props.GetHistogramNumBins();
            binPoints = new int[numBins + 1];

            min = allVariables[index[0]];
            max = allVariables[index[totalCells-1]];
        }

        // The starting binPoints will always be the first cell.
        binPoints[0] = 0;

        int j = 0;
        for(int i = 1; i < numBins; ++i)
        {
            // Get the threshold for this bin. The threshold is based on
            // the frequency of matches.
            float t = min + float(i) * (max-min) / float(numBins);
            if(props.GetHistogramType() == SelectionProperties::HistogramMatches)
            {
                while( allFrequencies[index[j]] < t )
                    ++j;
            }
            else if(props.GetHistogramType() == SelectionProperties::HistogramVariable)
            {
                while( allVariables[index[j]] < t )
                    ++j;
            }

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
                CQCellIdentifier cellid(allDomains[idx],allCellIds[idx]);

#ifdef PARALLEL
                // Only keep the piece local to this processor since there is 
                // another combination step later that will reunite the pieces.
                if(selection.find(cellid) != selection.end())
#endif
                {
                    CQCellData c;
                    c.frequency = allFrequencies[idx];
                    c.variable = 0.; // we don't need the variable anymore

                    narrowedSelection[cellid] = c;
                }
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
        else
            histogram.push_back(binPoints[bin+1] - binPoints[bin]);
    }

    delete [] binPoints;
    delete [] index;
    debug5 << mName << "end" << endl;
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
// ****************************************************************************

class CQFilter : public avtTimeLoopCollectorFilter
{
public:
    CQFilter() : avtTimeLoopCollectorFilter(), props(), summary()
    {
        CQ = NULL;
    }

    virtual ~CQFilter()
    {
    }

    void SetSelectionProperties(const SelectionProperties &p)
    {
        props = p;
    }

    void SetCumulativeQuery(CumulativeQuery *obj) { CQ = obj; }

    const SelectionSummary &GetSummary() const { return summary; }

    virtual const char *GetType() { return "CQFilter"; }
    virtual const char *GetDescription() { return "Creating cumulative query selection"; }
    virtual bool ExecutionSuccessful() { return true; }

    virtual avtDataTree_p ExecuteAllTimesteps(std::vector<avtDataTree_p> &);

protected:
    virtual avtContract_p ModifyContract(avtContract_p contract);

private:
    avtDataTree_p CreateSelectedDataset(const CumulativeQuery::CQCellIdentifierCQCellDataMap &selection) const;

    SelectionProperties  props;
    SelectionSummary     summary;
    CumulativeQuery     *CQ;
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
// ****************************************************************************

avtDataTree_p
CQFilter::ExecuteAllTimesteps(std::vector<avtDataTree_p> &timesteps)
{
    CumulativeQuery::CQCellIdentifierCQCellDataMap selection;

    summary = SelectionSummary();

    if(CQ != NULL)
    {
        doubleVector hist;
        CQ->CreateSelection(this->props, timesteps, selection, hist);

        // Save the histogram values in the summary.
        summary.SetHistogramValues(hist);
    }

    //
    // Now that we have the final set of local cells derived from the selection,
    // we create an output dataset that identifies this processor's portion
    // of the cells in the selection.
    //
    avtDataTree_p outputTree = CreateSelectedDataset(selection);

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
// ****************************************************************************

avtContract_p
CQFilter::ModifyContract(avtContract_p contract)
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

// ****************************************************************************
// Method: CQFilter::CreateSelectedDataset
//
// Purpose: 
//   Create a bogus 1D rectilinear grid that we'll pass back as the results
//   of the CQ selection creation.
//
// Arguments:
//   selection : The selection that we're translating into a VTK dataset.
//
// Returns:    A data tree with the bogus mesh.
//
// Note:       We pass back a bogus grid because the grid does not matter. Only
//             the original zones array on it matters because the named selection
//             manager gets that array and uses it to determine which cells are
//             in the named selection. This dataset tells it which cells.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 11:02:13 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

avtDataTree_p
CQFilter::CreateSelectedDataset(const CumulativeQuery::CQCellIdentifierCQCellDataMap &selection) const
{
    // We can translate this list into a new mesh with avtOriginalCellNumbers 
    // that contains the contents of the map.
    int dims[] = {1,1,1};
    dims[0] = selection.size()+1;
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    rgrid->SetDimensions(dims);
    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfTuples(dims[0]);
    for(int i = 0; i < dims[0]; ++i)
        x->SetTuple1(i, i);
    rgrid->SetXCoordinates(x);
    x->Delete();

    vtkUnsignedIntArray *ocn = vtkUnsignedIntArray::New();
    ocn->SetName("avtOriginalCellNumbers");
    ocn->SetNumberOfTuples(2 * selection.size());
    unsigned int *ptr = (unsigned int *)ocn->GetVoidPointer(0);

    CumulativeQuery::CQCellIdentifierCQCellDataMap::const_iterator it;
    for(it = selection.begin(); it != selection.end(); ++it)
    {
        *ptr++ = it->first.first;
        *ptr++ = it->first.second;
    }
    rgrid->GetCellData()->AddArray(ocn);
    ocn->Delete();

    avtDataTree_p outputTree = new avtDataTree(rgrid, -1);
    rgrid->Delete();

    return outputTree;
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
//   
// ****************************************************************************

class CumulativeQueryCacheItem : public avtNamedSelectionCacheItem
{
public:
    CumulativeQueryCacheItem() : avtNamedSelectionCacheItem(), 
        cellFrequency(), cellsPerTimestep(), summary()
    {
    }

    virtual ~CumulativeQueryCacheItem()
    {
        debug5 << "Deleting CumulativeQueryCacheItem." << endl;
    }

    CumulativeQuery::CQCellIdentifierCQCellDataMap cellFrequency;
    intVector                                      cellsPerTimestep;
    SelectionSummary                               summary;
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
    cqFilter = NULL;
    hist = NULL;
    threshold = NULL;
    nts = 1;
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
    if(cqFilter != NULL)
        delete cqFilter;

    if(hist != NULL)
        delete hist;

    if(threshold != NULL)
        delete threshold;
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::CreateSelectionKey
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
//   
// ****************************************************************************

std::string
CumulativeQueryNamedSelectionExtension::CreateSelectionKey(const SelectionProperties &props) const
{
    // implement me... For now return the name. We could work more information into the name
    // to allow us to cache multiple intermediate results for a single selection name.
    return props.GetName();
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::CheckProperties
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
// ****************************************************************************

bool
CumulativeQueryNamedSelectionExtension::CheckProperties(const SelectionProperties &newProps, 
    const SelectionProperties &oldProps) const
{
    bool basicsSame = (newProps.GetName() == oldProps.GetName()) &&
                      (newProps.GetSource() == oldProps.GetSource()) &&
                      (newProps.GetSelectionType() == oldProps.GetSelectionType());

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

    return basicsSame && varsSame && timeSame && typeCloseEnough;
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::GetSelection
//
// Purpose: 
//   Compute a cumulative query selection based on the input data object and
//   return the selection as a vector of doms, zones.
//
// Arguments:
//   dob   : The input data object (We don't always use it).
//   props : The selection properties.
//   cache : The NSM's cache.
//   doms  : The returned selection domains.
//   zones : The returned selection cells.
//
// Returns:    doms, zones.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 12:37:13 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
CumulativeQueryNamedSelectionExtension::GetSelection(avtDataObject_p dob,
    const SelectionProperties &props,
    avtNamedSelectionCache &cache,
    std::vector<int> &doms, std::vector<int> &zones)
{
    const char *mName = "CumulativeQueryNamedSelectionExtension::GetSelection: ";
    CumulativeQuery CQ;

    // Check the selection cache for intermediate data we can use so we do not
    // have to execute the CQ over time.
    std::string selectionKey = CreateSelectionKey(props);
    CumulativeQueryCacheItem *intermediateResults = NULL;
    if(cache.exists(selectionKey))
    {
        debug1 << mName << "Found intermediate selection data for "
               << props.GetName() << endl;
        intermediateResults = (CumulativeQueryCacheItem *)cache[selectionKey];

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
        CumulativeQuery::CQCellIdentifierCQCellDataMap selection;
        doubleVector hist;
        CQ.CreateSelection(props, intermediateResults->cellFrequency,
            intermediateResults->cellsPerTimestep, selection, hist);

        // Convert the selection to doms, zones.
        doms.reserve(selection.size());
        zones.reserve(selection.size());
        CumulativeQuery::CQCellIdentifierCQCellDataMap::const_iterator it = selection.begin();
        for(; it != selection.end(); ++it)
        {
            doms.push_back(it->first.first);
            zones.push_back(it->first.second);
        }

        // Save the histogram values in the summary.
        summary = intermediateResults->summary;
        summary.SetHistogramValues(hist);
    }
    else
    {
        StackTimer t1("Calculating CQ selection");

        debug1 << mName << "Calculating selection " << props.GetName()
               << " from scratch." << endl;

        bool needsUpdate = false;
        avtContract_p contract = GetContract(dob, needsUpdate);

        // Add in our custom filters.
        avtDataObject_p newdob = AddFilters(dob, props);
        if(cqFilter != NULL)
            cqFilter->SetCumulativeQuery(&CQ);

        // Execute the full pipeline.
        debug1 << mName << "Must re-execute pipeline to create named selection" << endl;
        TimedCodeBlock("CQ Pipeline update",
            newdob->Update(contract);
        );
        debug1 << mName << "Done re-executing pipeline to create named selection" << endl;
    
        // Extract the selection from the data tree.
        avtDataset_p ds;
        CopyTo(ds, newdob);
        GetSelectionFromDataset(ds, doms, zones);

        // Extract data from the filters and stick it in the summary. 
        summary = BuildSummary();

        // Delete our filters.
        if(hist != NULL)
            delete hist;
        hist = NULL;
        if(cqFilter != NULL)
            delete cqFilter;
        cqFilter = NULL;
        if(threshold != NULL)
            delete threshold;
        threshold = NULL;

        // Cache the intermediate selection & summary.
        debug1 << mName << "Caching intermediate results for selection" << endl;
        CumulativeQueryCacheItem *item = new CumulativeQueryCacheItem;
        item->properties = props;
        item->cellFrequency = CQ.GetCellFrequency();
        item->cellsPerTimestep = CQ.GetCellsPerTimestep();
        item->summary = summary;
        cache[selectionKey] = item;
    }
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::AddFilters
//
// Purpose: 
//   Inject additional filters that we'll use to set up the pipeline that we'll
//   execute to arrive at the datasets that give us our data selection.
//
// Arguments:
//   dob      : the input data object.
//   props    : The selection properties for the selection we're creating.
//
// Returns:   The output of any filters that we've added to the pipeline. 
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 14:48:47 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Sep  6 15:28:26 PDT 2011
//   I renamed the method and dropped the contract argument.
//
// ****************************************************************************

avtDataObject_p
CumulativeQueryNamedSelectionExtension::AddFilters(avtDataObject_p dob, 
    const SelectionProperties &props)
{
    avtDataObject_p retval(dob);

    if(!props.GetVariables().empty())
    {
        // Set up the variable that we're going to use to threshold.
        SelectionProperties prop2(props);
        if(prop2.GetMaxTimeState() < 0)
            prop2.SetMaxTimeState(dob->GetInfo().GetAttributes().GetNumStates()-1);

        // Save the number of time steps.
        nts = prop2.GetMaxTimeState() - prop2.GetMinTimeState() + 1;

        //
        // Add a CQHistogramCalculationFilter so we can calculate histograms for
        // the variables that we care about.
        //
        if(hist != NULL)
            delete hist;
        hist = new CQHistogramCalculationFilter;
        hist->SetInput(dob);
        for(int i = 0; i < prop2.GetVariables().size(); ++i)
            hist->AddVariable(prop2.GetVariables()[i]);
        retval = hist->GetOutput(); 

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

        if(threshold != NULL)
            delete threshold;
        threshold = new avtThresholdFilter;
        threshold->SetInput(retval);
        threshold->SetAtts(&thresholdAtts);
        retval = threshold->GetOutput();

        //
        // Add our time iteration CQ filter.
        //
        cqFilter = new CQFilter;
        cqFilter->SetInput(retval);
        cqFilter->SetTimeLoop(prop2.GetMinTimeState(), prop2.GetMaxTimeState(),
            prop2.GetTimeStateStride());
        cqFilter->SetSelectionProperties(prop2);
        retval = cqFilter->GetOutput();
    }

    return retval;
}

// ****************************************************************************
// Method: CumulativeQueryNamedSelectionExtension::BuildSummary
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
// ****************************************************************************

SelectionSummary
CumulativeQueryNamedSelectionExtension::BuildSummary()
{
    const char *mName = "CumulativeQueryNamedSelectionExtension::FreeUpResources: ";

    // Stash hist's histograms into this class so we can return them from the engine.
    SelectionSummary s;
    stringVector vars = hist->GetVariables();
    for(size_t i = 0; i < vars.size(); ++i)
    {
        const CQHistogramCalculationFilter::Histogram *h = hist->GetHistogram(vars[i]);
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
debug5 << "CQnamedSelectionExtension: total cells = " << hist->GetTotalCellCount() << endl;
    s.SetTotalCellCount((int)hist->GetTotalCellCount());

    // Get the histogram computed by the CQ filter.
    if(cqFilter != NULL)
    {
        const SelectionSummary &histSummary = cqFilter->GetSummary();
        s.SetHistogramValues(histSummary.GetHistogramValues());
    }

    return s;
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
