#
# file: dbreak_animate_pathlines.py
# info: Source in VisIt's Commands window with pathlines
#       from dbreak3d_plot_pathlines.py already created.
#

def animate_pathlines():
    iatts = IntegralCurveAttributes(1)
    iatts.cropValue = iatts.Time
    iatts.cropEndFlag = 1
    for ts in range(161):
        tval = ts * .025
        iatts.cropEnd = tval
        SetOperatorOptions(iatts)

animate_pathlines()

