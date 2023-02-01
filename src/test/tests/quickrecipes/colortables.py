# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  colortables.py
#
#  Programmer: Kathleen Biagas 
#  Date:       March 30, 2022
#
#  Modificatons:
#    Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
#    Made changes to reflect the fact that built-in tables cannot be edited.
#
#    Mark C. Miller, Mon Dec 12 19:02:35 PST 2022
#    Add introspecting block
# ----------------------------------------------------------------------------


# NOTE: Sections of this test file are 'literalinclude'd in quickrecipes.rst.
# After modifying this file, ensure the proper content is still displayed in the doc.

# comments of the form '# sometext {' and '# sometext }' bracket the sections
# that are 'literalinclude'd in quickrecipes.rst.

def introspectingColorTable():

    # introspectingColorTable {
    hotCT = GetColorTable("hot")
    print(hotCT)
    # results of print
    #    GetControlPoints(0).colors = (0, 0, 255, 255)
    #    GetControlPoints(0).position = 0
    #    GetControlPoints(1).colors = (0, 255, 255, 255)
    #    GetControlPoints(1).position = 0.25
    #    GetControlPoints(2).colors = (0, 255, 0, 255)
    #    GetControlPoints(2).position = 0.5
    #    GetControlPoints(3).colors = (255, 255, 0, 255)
    #    GetControlPoints(3).position = 0.75
    #    GetControlPoints(4).colors = (255, 0, 0, 255)
    #    GetControlPoints(4).position = 1
    #    smoothing = Linear  # NONE, Linear, CubicSpline
    #    equalSpacingFlag = 0
    #    discreteFlag = 0
    # introspectingColorTable }

def modifyExistingColorTable():

    # modifyTable1 {
    OpenDatabase(silo_data_path("rect2d.silo"))
    AddPlot("Pseudocolor", "d")

    pc = PseudocolorAttributes()
    pc.centering=pc.Nodal 
    # set color table name
    pc.colorTableName = "hot"
    SetPlotOptions(pc)
    
    DrawPlots()
    # put the plot in full-frame mode
    v = GetView2D()
    v.fullFrameActivationMode= v.On
    SetView2D(v)
    # modifyTable1 } 

    Test("standard_hot_table")

    hotCTorig = GetColorTable("hot")

    # modifyTable2 {
    hotCT = GetColorTable("hot")
  
    # Remove a couple of control points
    hotCT.RemoveControlPoints(4)
    hotCT.RemoveControlPoints(3)

    # We must use a different name, as VisIt will not allow overwriting of built-in color tables
    SetColorTable("hot_edited", hotCT)

    # set color table name so changes to it will be reflected in plot
    pc.colorTableName = "hot_edited"
    SetPlotOptions(pc)
    # modifyTable2 }

    Test("modified_hot_table_1")

    # modifyTable3 {
    # Change colors
    hotCT.GetControlPoints(0).colors = (255,0,0,255)
    hotCT.GetControlPoints(1).colors = (255, 0, 255, 255)
    SetColorTable("hot_edited", hotCT)
    # modifyTable3 }

    Test("modified_hot_table_2")

    # modifyTable4 {
    # Turn on equal spacing
    hotCT.equalSpacingFlag = 1
    # Create a new color table by providing a different name
    SetColorTable("hot2", hotCT)

    # tell the Pseudocolor plot to use the new color table
    pc.colorTableName = "hot2"
    SetPlotOptions(pc)
    # modifyTable4 }

    Test("hot2")
 
    # modifyTable5 {
    # Change positions so that the first and last are at the endpoints
    hotCT.equalSpacingFlag=0
    hotCT.GetControlPoints(0).position = 0
    hotCT.GetControlPoints(1).position =0.5 
    hotCT.GetControlPoints(2).position = 1
    SetColorTable("hot3", hotCT)

    pc.colorTableName = "hot3"
    SetPlotOptions(pc)
    # modifyTable5 }

    Test("hot3")

    # remove the added color tables
    RemoveColorTable("hot_edited")
    RemoveColorTable("hot2")
    RemoveColorTable("hot3")
    DeleteAllPlots()
 
def createContinuous():
    # based on http://visitusers.org/index.php?title=Creating_a_color_table

    # continuous1 {
    # create control points (red, green, blue, position).
    ct = ((1,0,0,0.), (1,0.8,0.,0.166), (1,1,0,0.333), (0,1,0,0.5),
          (0,1,1,0.666), (0,0,1,0.8333), (0.8,0.1,1,1))

    ccpl = ColorControlPointList()

    # add the control points to the list 
    for pt in ct:
        p = ColorControlPoint()
        # colors is RGBA and must be in range 0...255
        p.colors = (pt[0] * 255, pt[1] * 255, pt[2] * 255, 255)
        p.position = pt[3]
        ccpl.AddControlPoints(p)
    AddColorTable("myrainbow", ccpl)

    OpenDatabase(silo_data_path("globe.silo"))
    AddPlot("Pseudocolor", "speed")

    # Make the plot use the new color table
    pc = PseudocolorAttributes(1)
    pc.colorTableName = "myrainbow"
    SetPlotOptions(pc)

    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.693476, 0.212776, 0.688344)
    v. viewUp = (0.161927, 0.976983, -0.138864)
    SetView3D(v)
    # continuous1 }

    Test("rainbow_continuous")

    RemoveColorTable("myrainbow")
    DeleteAllPlots()


def createDiscreteUsingVTKNamedColors():
    # discrete1 {
    try:
        import vtk # for vtk.vtkNamedColors
    except:
        return

    # to see list of all color names available: 
    # print(vtk.vtkNamedColors.GetColorNames())

    # choose some colors from vtk.vtkNamedColors
    colorNames = ["tomato", "turquoise", "van_dyke_brown", "carrot",
                  "royalblue", "naples_yellow_deep", "cerulean", "warm_grey",
                  "venetian_red", "seagreen", "sky_blue", "pink"]
    # Create a color control point list
    ccpl = ColorControlPointList()
    # Make it discrete
    ccpl.discreteFlag=1
    # Add color control points corresponding to color names
    for name in colorNames:
        p = ColorControlPoint()
        p.colors=vtk.vtkNamedColors().GetColor4ub(name)
        ccpl.AddControlPoints(p)
    # add a color table based on the color control points
    AddColorTable("mylevels", ccpl)

    OpenDatabase(silo_data_path("multi_rect2d.silo"))
    AddPlot("Subset", "domains")
    s = SubsetAttributes()
    s.colorType = s.ColorByColorTable
    s.colorTableName = "mylevels"
    SetPlotOptions(s)
    DrawPlots()
    # discrete1 }

    Test("discrete_using_vtk")

    # remove the added color tables
    RemoveColorTable("mylevels")
    DeleteAllPlots()

def main():
    introspectingColorTable()
    modifyExistingColorTable()
    createContinuous()
    createDiscreteUsingVTKNamedColors()

main()
Exit()
