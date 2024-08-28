# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  renderpoints.py
#
#  Tests:      mesh      - 3D point
#              plots     - pseudocolor, subset, mesh
#
#  Defect ID:  none
#
#  Programmer: Kathleen Biagas
#  Date:       August 28, 2024 
#
#  Modificatons:
#
# ----------------------------------------------------------------------------


def TestVertexOnlyFiles():

    # These test files were designed to test vertex-only,
    # poly-vertex only and mixed vertex and poly-vertex datasets.
    # The polyData and ugrid versions should be identical.
    vtk_vertex_files=["polyData_vertex.vtk",
                      "polyData_polyVertex.vtk",
                      "polyData_mixedVertex.vtk",
                      "ugrid_vertex.vtk",
                      "ugrid_polyVertex.vtk",
                      "ugrid_mixedVertex.vtk"]

    glyphNames=["Box", "Axis", "Icosahedron", "Octahedron", "Tetrahedron", "SphereGeometry", "Point", "Sphere"]

    plots={'Pseudocolor':'foo_cells','Mesh':'mesh','Subset':'mesh'}
    
    for f in vtk_vertex_files:
        OpenDatabase(data_path("vtk_test_data",f))

        for p,v in plots.items():
            AddPlot(p,v)
            DrawPlots()
            v3d = GetView3D()
            v3d.viewNormal = (-0.396043, 0.723303, 0.56567)
            v3d.viewUp = (0.246238, 0.677138, -0.693434)
            SetView3D(v3d);

            if p == 'Pseudocolor':
                atts=PseudocolorAttributes()
            elif p == 'Mesh':
                atts=MeshAttributes()
            else:
                atts=SubsetAttributes()

            atts.pointSize = 0.5
            atts.pointSizePixels = 10

            for i,n in enumerate(glyphNames):
                atts.pointType = i
                SetPlotOptions(atts)
                Test(p+"_"+n+"_"+f)

            DeleteAllPlots()

        CloseDatabase(data_path("vtk_test_data",f))

TestVertexOnlyFiles()
Exit()

