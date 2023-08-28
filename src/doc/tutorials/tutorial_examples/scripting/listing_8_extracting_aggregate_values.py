###########################################
# file: VisIt Scripting Tutorial Listing 8
###########################################
#
# 1) Run visit from the CLI as described below.
#



####################
# Example that demonstrates looping over a dataset
# to extract an aggregate value at each timestep.
#
#  visit -nowin -cli -s listing_8_extracting_aggregate_values.py wave.visit pressure wave_pressure_out
####################
 
import sys
from visit_utils import *
 
def setup_plot(dbname,varname, materials = None):
    """
    Create a plot to query.
    """
    OpenDatabase(dbname)
    AddPlot("Pseudocolor",varname)
    if not materials is None:
        TurnMaterialsOff()
        # select the materials (by id )
        # example 
        #  materials = [ "1", "2", "4"]
        for material in materials:
            TurnMaterialsOn(materials)
    DrawPlots()
 
def extract_curve(varname,obase,stride=1):
    """
    Loop over all time steps and extract a value at each one.
    """
    f = open(obase + ".ult","w")
    f.write("# %s vs time\n" % varname)
    nts = TimeSliderGetNStates()
    for ts in range(0,nts,stride):
        print("processing timestate: %d" % ts)
        TimeSliderSetState(ts)
        tval = query("Time")
        # sums plotted variable scaled by 
        #   area (2D mesh), 
        #   revolved_volume (2D RZ mesh, or 
        #   volume (3D mesh)
        rval = query("Weighted Variable Sum")
        # or you can use other queries, such as max:
        # mval = query("Maximum")
        res = "%s %s\n" % (str(tval),str(rval))
        print(res)
        f.write(res)
    f.close()
 
 
def open_engine():
    # to open an parallel engine
    #  inside of an mxterm or batch script use:
    engine.open(method="slurm")
    #  outside of an mxterm or batch script
    # engine.open(nprocs=21)
 
def main():
    nargs = len(sys.argv)
    if nargs < 4:
        usage_msg = "usage: visit -nowin -cli -s visit_extract_curve.py "
        usage_msg += "{databsase_name} {variable_name} {output_file_base}"
        print(usage_msg)
        sys.exit(-1)
    # get our args
    dbname  = sys.argv[1]
    varname = sys.argv[2]
    obase   = sys.argv[3]
    # if you need a parallel engine:
    # open_engine()
    setup_plot(dbname,varname)
    extract_curve(varname,obase)
 
 
if __visit_script_file__ == __visit_source_file__:
    main()
    sys.exit(0)
