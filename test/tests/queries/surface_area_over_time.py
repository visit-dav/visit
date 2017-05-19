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
# ----------------------------------------------------------------------------

from visit_utils import *


def query_sa_stepwise():
    nts = TimeSliderGetNStates()
    c_vals = []
    s_vals = []
    for ts in range(nts):
        TimeSliderSetState(ts)
        c_vals.append(query("Cycle"))
        s_vals.append(query("3D Surface Area"))
    f = open("_out_query_sa_stepwise.ult","w")
    f.write("# sa vs cycle\n")
    for i in range(len(c_vals)):
        f.write("%s %s\n" % (c_vals[i],s_vals[i]))
    f.close()
    
def query_sa_q_over_time():
    TimeSliderSetState(0)
    QueryOverTime("3D surface area", start_time=0, end_time=19, stride=1)


def setup_plot():
    SetActiveWindow(1)
    DeleteAllPlots()
    AddPlot("Pseudocolor", "pressure")
    AddOperator("Isosurface")
    DrawPlots()


def run_queries():
    query_sa_stepwise()
    query_sa_q_over_time()
    SetActiveWindow(2)
    OpenDatabase("_out_query_sa_stepwise.ult")
    AddPlot("Curve","sa vs cycle")
    DrawPlots()
    
    
def open_db():
    OpenDatabase(silo_data_path("wave.visit"))


def main():
    open_db()
    setup_plot()
    run_queries()
    Test("SA_OverTime_0000")


# Call the main function
TurnOnAllAnnotations()
main()
Exit()


