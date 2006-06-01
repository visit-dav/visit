/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <PlotList.h>
#include <Observer.h>
#include <BufferConnection.h>

#include <TestUtil.h>

#define VERBOSE
#define N_TESTS 2

// Prototypes
bool Run_Test1(bool verbose, int *subtest, int *nsubtests);
bool Run_Test2(bool verbose, int *subtest, int *nsubtests);

// *******************************************************************
// Function: main
//
// Purpose:
//   The main function for this test program. It executes the test
//   cases.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 15:34:38 PST 2000
//
// Modifications:
//   
// *******************************************************************

int
main(int argc, char *argv[])
{
    int      subtest, nsubtests;
    bool     test[N_TESTS];
    TestUtil util(argc, argv, "Tests Plot and PlotList classes");

    // Test the Plot class.
    test[0] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(1, subtest, nsubtests, test[0]);

    // Test the Plot class.
    test[1] = Run_Test2(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(2, subtest, nsubtests, test[1]);
    
    return util.PassFail(test, N_TESTS);
}

///////////////////////////////////////////////////////////////////////////
/// Test 1 - Test the Plot class.
///
/// Notes:
///    This test is designed to test various methods and operators in the
///    Plot class.
///
///////////////////////////////////////////////////////////////////////////
#ifdef VERBOSE
ostream &
operator << (ostream &os, const Plot &p)
{
    const char *names[] = {
       "BLOCK=0", "BOUNDARY=1", "CONTOUR=2", "CURVE=3", "FILLED_BOUNDARY=4",
       "LABEL=5", "MESH=6", "PICK=7", "PSEUDOCOLOR=8", "REFLINE=9", "SURFACE=10",
       "VECTOR=11"
    };

    os << "{plotType=" << names[p.GetPlotType()];
    os << ", activeFlag=" << p.GetActiveFlag();
    os << ", hiddenFlag=" << p.GetHiddenFlag();
    os << ", plotVar=" << p.GetPlotVar();
    os << "}";

    return os;
}
#endif

// *******************************************************************
// Function: Run_Test1
//
// Purpose:
//   Tests operations for the Plot class.
//
// Arguments:
//   verbose   : Whether or not to do lots of output.
//   subtest   : A return code to indicate the last subtest to be
//               executed.
//   nsubtests : The number of subtests.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 10 16:09:30 PST 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test1(bool verbose, int *subtest, int *nsubtests)
{
    // We have 7 subtests
    *nsubtests = 7;

#ifdef VERBOSE
    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 1" << endl;
        cout << "=================================================" << endl;
    }
#endif

    // Create a plot
    Plot p1;

    // Test set/get PlotType
    *subtest = 1;
    p1.SetPlotType(Plot::BLOCK);
    if(p1.GetPlotType() != Plot::BLOCK)
        return false;

    // Test set/get ActiveFlag
    *subtest = 2;
    p1.SetActiveFlag(false);
    if(p1.GetActiveFlag() != false)
        return false;

    // Test set/get HiddenFlag
    *subtest = 3;
    p1.SetHiddenFlag(true);
    if(p1.GetHiddenFlag() != true)
        return false;

    // Test set/get PlotVar
    *subtest = 4;
    p1.SetPlotVar("density");
    if(p1.GetPlotVar() != std::string("density"))
        return false;

    // Create another plot
    Plot p2;

    // Test the assignment operator
    *subtest = 5;
    p2 = p1;
    if(p1 != p2)
        return false;

    // Test the copy constructor
    *subtest = 6;
    Plot p3(p1);
    if(p1 != p3)
        return false;

    // Try writing and reading a Plot object
    *subtest = 7;
    BufferConnection buf;
    bool retval = true;
    for(int i = 0; (i < 4) && retval; ++i)
    {
        Plot writer, reader;

        if(i >= 0)
            writer.SetPlotType(Plot::PSEUDOCOLOR);
        if(i >= 1)
            writer.SetActiveFlag(true);
        if(i >= 2)
            writer.SetHiddenFlag(true);
        if(i >= 3)
            writer.SetPlotVar(std::string("p"));

        // Write the Plot to the connection
        writer.Write(buf);

        // Read the Plot from the connection
        reader.Read(buf);

        // See if the Plots are equal
        retval = (reader == writer);
#ifdef VERBOSE
        // Write the plot to stdout
        if(verbose)
            cout << endl << "Writer:" << writer << endl << "Reader:"
                 << reader << endl << endl;
#endif
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////
/// Test 2 - Test the PlotList class.
///
/// Notes:
///    This test is designed to test various methods and operators in the
///    PlotList class.
///
///////////////////////////////////////////////////////////////////////////

// *******************************************************************
// Class: PlotListPrinter
//
// Purpose:
//   This observer class prints out the fields that have changed in a
//   PlotList.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 11 13:31:16 PST 2000
//
// Modifications:
//   
// *******************************************************************

class PlotListPrinter : public Observer
{
public:
    PlotListPrinter(PlotList *plots) : Observer((Subject *)plots)
    {
        // nothing much
    }

    ~PlotListPrinter()
    {
        // nothing here
    }

    virtual void Update(Subject *)
    {
        PlotList *plots = (PlotList *)subject;
        const char *names[] = {
            "BLOCK=0", "BOUNDARY=1", "CONTOUR=2", "CURVE=3", "FILLED_BOUNDARY=4",
            "LABEL=5", "MESH=6", "PICK=7", "PSEUDOCOLOR=8", "REFLINE=9", "SURFACE=10",
            "VECTOR=11"
        };

        // There is only one attribute in the PlotList class and it is
        // the plot list.

        cout << "{";
        for(int i = 0; i < plots->GetNumPlots(); ++i)
        {
            Plot &p = plots->GetPlot(i);
            cout << "{type=" << names[(int)p.GetPlotType()] <<
                    ", active=" << p.GetActiveFlag() <<
                    ", hidden=" << p.GetHiddenFlag() <<
                    ", var=" << p.GetPlotVar() << "}, ";
        }
        cout << "}" << endl;
    }
};

// *******************************************************************
// Function: operator << (ostream &os, const PlotList &p)
//
// Purpose:
//   Writes a PlotList to an ostream.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 11 13:30:43 PST 2000
//
// Modifications:
//   
// *******************************************************************

ostream &
operator << (ostream &os, const PlotList &p)
{
    os << "{";
    for(int i = 0; i < p.GetNumPlots(); ++i)
        os << "plot[" << i << "]=" << p[i] << ", ";
    os << "}";

    return os;
}

// *******************************************************************
// Function: Run_Test2
//
// Purpose:
//   This function runs this program's test case #2 which tests the 
//   PlotList class.
//
// Arguments:
//   verbose   : Whether or not to do lots of output.
//   subtest   : A return code to indicate the last subtest to be
//               executed.
//   nsubtests : The number of subtests.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 11 13:30:06 PST 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test2(bool verbose, int *subtest, int *nsubtests)
{
    // We have 11 subtests
    *nsubtests = 11;

#ifdef VERBOSE
    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 2" << endl;
        cout << "=================================================" << endl;
    }
#endif

    // Create a plot
    Plot plot1;
    plot1.SetPlotType(Plot::FILLED_BOUNDARY);
    plot1.SetActiveFlag(true);
    plot1.SetHiddenFlag(true);
    plot1.SetPlotVar("rho");

    // Create another plot
    Plot plot2;
    plot2.SetPlotType(Plot::CONTOUR);
    plot2.SetActiveFlag(true);
    plot2.SetHiddenFlag(false);
    plot2.SetPlotVar("density");

    // Create another plot
    Plot plot3;
    plot3.SetPlotType(Plot::PSEUDOCOLOR);
    plot3.SetActiveFlag(false);
    plot3.SetHiddenFlag(true);
    plot3.SetPlotVar("pressure");

    // Create a plot list
    PlotList p1;

    // Test adding plots and the [] operator.
    *subtest = 1;
    p1.AddPlot(plot1);
    if(p1[0] != plot1)
        return false;

    // Test GetNumPlots
    *subtest = 2;
    if(p1.GetNumPlots() != 1)
        return false;

    // Test the copy constructor and the == operator
    *subtest = 3;
    PlotList p2(p1);
#ifdef VERBOSE
    if(verbose)
        cout << "p1=" << p1 << endl << "p2=" << p2 << endl;
#endif
    if(p1 != p2)
        return false;

    // Test ClearPlots
    *subtest = 4;
    p1.AddPlot(plot2);
    p1.ClearPlots();
    if(p1.GetNumPlots() != 0)
        return false;

    // Test RemovePlot
    *subtest = 5;
    PlotList p3;
    p3.AddPlot(plot1);
    p3.AddPlot(plot2);
    p3.AddPlot(plot3);
    PlotList p4(p3);
#ifdef VERBOSE
    if(verbose)
        cout << endl << "p3=" << p3 << endl << "p4=" << p4 << endl;
#endif
    p3.RemovePlot(1); // Remove the 2nd plot from both
    p4.RemovePlot(1);
#ifdef VERBOSE
    if(verbose)
        cout << endl << "p3=" << p3 << endl << "p4=" << p4 << endl;
#endif
    if(p3 != p4)
        return false;

    // Test GetPlot
    *subtest = 6;
    PlotList p5;
    p5.AddPlot(plot1);
    // Set the plot to plot3's attributes.
    p5.GetPlot(0).SetPlotType(Plot::PSEUDOCOLOR);
    p5.GetPlot(0).SetActiveFlag(false);
    p5.GetPlot(0).SetHiddenFlag(true);
    p5.GetPlot(0).SetPlotVar("pressure");
#ifdef VERBOSE
    if(verbose)
        cout << endl << "p5[0]=" << p5.GetPlot(0) << ", plot3="
             << plot3 << endl;
#endif
    if(p5.GetPlot(0) != plot3)
        return false;

    // Test the [] operator
    *subtest = 7;
    PlotList p6;
    p6.AddPlot(plot1);
    // Set the plot to plot3's attributes.
    p6[0].SetPlotType(Plot::PSEUDOCOLOR);
    p6[0].SetActiveFlag(false);
    p6[0].SetHiddenFlag(true);
    p6[0].SetPlotVar("pressure");
#ifdef VERBOSE
    if(verbose)
        cout << endl << "p6[0]=" << p6.GetPlot(0) << ", plot3="
             << plot3 << endl;
#endif
    if(p6[0] != plot3)
        return false;

    // Test the assignment operator
    *subtest = 8;
    PlotList p7, p8;
    p7.AddPlot(plot1);
    p7.AddPlot(plot2);
    p8 = p7;
    if(verbose)
        cout << endl << "p7=" << p7 << endl << "p8=" << p8 << endl;
    if(p7 != p8)
       return false;

    // Test writing and reading a PlotList
    *subtest = 9;
    bool retval = true;
    BufferConnection buf;
#ifdef VERBOSE
    if(verbose)
        cout << endl << "Testing read/write of PlotList" << endl;
#endif
    for(int i = 0; (i < 3) && retval; ++i)
    {
        PlotList writer;

        plot1.UnSelectAll();
        plot2.UnSelectAll();
        plot3.UnSelectAll();

        if(i >= 0)
           writer.AddPlot(plot1);
        if(i >= 1)
           writer.AddPlot(plot2);
        if(i >= 2)
           writer.AddPlot(plot3);

        // Write to the vuffer
        writer.Write(buf);

        // Read from the buffer.
        PlotList reader;
        reader.Read(buf);

#ifdef VERBOSE
        if(verbose)
        {
            PlotListPrinter *TObserver = new PlotListPrinter(&reader);
            reader.Notify();
            delete TObserver;
        }
#endif

        // See if they were equal.
        retval = (writer == reader);
    }
    if(!retval)
        return false;

    // Test sending a PlotList that has had a plot removed.
    *subtest = 10;
#ifdef VERBOSE
    if(verbose)
        cout << endl << "Testing read/write of PlotList" << endl;
#endif
    PlotList p9;
    p9.AddPlot(plot1);
    p9.AddPlot(plot2);
    PlotList p10(p9);
    // Remove plot 0 from p9.
    p9.RemovePlot(0);

    p9.Write(buf);
    p10.Read(buf);
#ifdef VERBOSE
    if(verbose)
    {
        PlotListPrinter *TObserver = new PlotListPrinter(&p10);
        p10.Notify();
        delete TObserver;
    }
#endif
    if(p9 != p10)
        return false;

    // Test sending a PlotList that has been emptied.
    *subtest = 11;
#ifdef VERBOSE
    if(verbose)
        cout << endl << "Testing read/write of PlotList" << endl;
#endif
    PlotList p11;
    p11.AddPlot(plot1);
    p11.AddPlot(plot2);
    PlotList p12(p11);
    // Remove all plots from p11 and write it
    p11.ClearPlots();
    p11.Write(buf);

    // Read an empty PlotList into p12.
    p12.Read(buf);
#ifdef VERBOSE
    if(verbose)
    {
        PlotListPrinter *TObserver = new PlotListPrinter(&p12);
        p12.Notify();
        delete TObserver;
    }
#endif
    if(p12.GetNumPlots() != 0)
        return false;

    return true;
}
