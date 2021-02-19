# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  surface_area_over_time.py
#  Tests:      queries     - surface area over time
#
#
#  Programmer: Cyrus Harrison
#  Date:       Fri May 19 10:04:46 PDT 2017
#
#  Modifications:
#    Kathleen Biagas, Tue Sep 22 13:24:42 PDT 2020
#    Use custom color for curves, as the color-cycling is non-deterministic
#    across runs.
#
# ----------------------------------------------------------------------------

from visit_utils import *


def query_sa_stepwise(s, nts, qname):
    c_vals = []
    s_vals = []
    for ts in range(s, nts):
        TimeSliderSetState(ts)
        c_vals.append(query("Cycle"))
        s_vals.append(query(qname))
    f = open("_out_query_sa_stepwise.ult","w")
    f.write("# sa vs cycle\n")
    for i in range(len(c_vals)):
        f.write("%s %s\n" % (c_vals[i],s_vals[i]))
    f.close()
    
def query_sa_q_over_time(s, nts, qname):
    TimeSliderSetState(0)
    QueryOverTime(qname, start_time=s, end_time=nts-1, stride=1)


def setup_PC_Iso_plot():
    SetActiveWindow(1)
    DeleteAllPlots()
    AddPlot("Pseudocolor", "pressure")
    AddOperator("Isosurface")
    DrawPlots()

def setup_boundary_plot():
    SetActiveWindow(1)
    DeleteAllPlots()
    AddPlot("Boundary", "Material")
    DrawPlots()

def setup_data_binning():
    SetActiveWindow(1)
    DeleteAllPlots()

    # We *Should* be using data binning like this, but there is 
    # a bug with query-over-time, so we add the operator separately.
    #AddPlot("Pseudocolor", "operators/DataBinning/2D/quadmesh")

    AddPlot("Pseudocolor", "pressure")
    AddOperator("DataBinning")
    DataBinningAtts = DataBinningAttributes()
    DataBinningAtts.numDimensions = DataBinningAtts.Two
    DataBinningAtts.dim1BinBasedOn = DataBinningAtts.X
    DataBinningAtts.dim2BinBasedOn = DataBinningAtts.Y
    DataBinningAtts.reductionOperator = DataBinningAtts.Maximum
    DataBinningAtts.varForReduction = "pressure"
    SetOperatorOptions(DataBinningAtts, 1)
 
    DrawPlots()


def run_queries(s, numSteps, qname = "3D Surface area"):
    query_sa_stepwise(s, numSteps, qname)
    query_sa_q_over_time(s, numSteps, qname)
    SetActiveWindow(2)
    ca1 = CurveAttributes(1)
    ca1.curveColorSource = ca1.Custom
    ca1.curveColor=(255, 0, 0)
    SetPlotOptions(ca1)
    OpenDatabase("_out_query_sa_stepwise.ult")
    AddPlot("Curve","sa vs cycle")
    DrawPlots()
    ca2 = CurveAttributes(1)
    ca2.curveColorSource = ca2.Custom
    ca2.curveColor=(0, 255, 0)
    SetPlotOptions(ca2)
    
    
def open_db():
    OpenDatabase(silo_data_path("wave.visit"))

def clear_qot():    
    SetActiveWindow(2)
    DeleteAllPlots()
    CloseDatabase("_out_query_sa_stepwise.ult")
    SetActiveWindow(1)
    TurnMaterialsOn()

def main():
    open_db()
    setup_PC_Iso_plot()
    numSteps = TimeSliderGetNStates()
    run_queries(0, numSteps)
   
    Test("SA_OverTime_0000")

    clear_qot()

    setup_boundary_plot()
    run_queries(0, numSteps)
    Test("SA_OverTime_0001")

    clear_qot()

    numSteps = 45
    m2 = "2 water"
    m1 = "1 barrier"
    TurnMaterialsOff(m2)
    run_queries(0, numSteps)
    Test("SA_OverTime_0002")

    clear_qot()

    TimeSliderSetState(1)
    TurnMaterialsOff(m1)
    run_queries(1, numSteps)
    Test("SA_OverTime_0003")


    clear_qot()

    setup_data_binning()
    numSteps = TimeSliderGetNStates()
    run_queries(0, numSteps, "2D area")
    Test("SA_OverTime_0004")



# Call the main function
TurnOnAllAnnotations()
main()
Exit()


