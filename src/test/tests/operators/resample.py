# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  resample.py
#
#  Tests:      resample operator
#
#  Defect ID:  none
#
#  Programmer: Sean Ahern
#  Date:       Thu Jan  3 11:00:41 EST 2008
#
#  Modifications:
#
#    Hank Childs, Wed Dec 31 14:32:34 PST 2008
#    Rename ResamplePluginAtts to ResampleAtts.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Alister Maguire, Fri Jan 29 08:17:27 PST 2021
#    Set useExtents = 0 where appropriate as setting the start/end
#    values no longer overrides useExtents.
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("noise.silo"))

AddPlot("Pseudocolor", "PointVar")
AddOperator("Resample")

resample = ResampleAttributes()
resample.samplesX = 5
resample.samplesY = 6
resample.samplesZ = 7
SetOperatorOptions(resample)

DrawPlots()

bigView = GetView3D()
bigView.SetViewNormal(-0.528889, 0.367702, 0.7649)
bigView.SetViewUp(0.176641, 0.929226, -0.324558)
bigView.SetParallelScale(17.3205)
bigView.SetPerspective(1)
SetView3D(bigView)

Test("ops_resampleop_noise_point_small")

resample = ResampleAttributes()
resample.samplesX = 20
resample.samplesY = 30
resample.samplesZ = 40
SetOperatorOptions(resample)

Test("ops_resampleop_noise_point_large")

resample = ResampleAttributes()
resample.useExtents = 0
resample.startX = 5.25
resample.endX = 9.57
resample.samplesX = 10
resample.startY = -5.9
resample.endY = 0
resample.samplesY = 20
resample.startZ = -6
resample.endZ = 0
resample.samplesZ = 30
SetOperatorOptions(resample)

ResetView()
v = GetView3D()
v.SetViewNormal(-0.41557,-0.758094,0.502588)
v.SetFocus(6.862131,4.91535,-9.08261)
v.SetViewUp(-0.720258,0.611719,0.327153)
v.SetParallelScale(10)
v.SetImagePan(-0.21812, 0.210298)
v.SetImageZoom(1.6527)
v.SetPerspective(1)
SetView3D(v)

Test("ops_resampleop_noise_point_explicit")

resample.useExtents = 1
SetOperatorOptions(resample)
SetView3D(bigView)

Test("ops_resampleop_noise_point_backfull")

DeleteAllPlots()

AddPlot("Pseudocolor", "hardyglobal")
AddOperator("Resample")

resample = ResampleAttributes()
resample.samplesX = 3
resample.samplesY = 3
resample.samplesZ = 3
SetOperatorOptions(resample)

SetView3D(bigView)
DrawPlots()

Test("ops_resampleop_noise_hardy_small")

DeleteAllPlots()

OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "v")
AddOperator("Resample")

resample = ResampleAttributes()
resample.samplesX = 10
resample.samplesY = 20
resample.is3D = 0
SetOperatorOptions(resample)

DrawPlots()

Test("ops_resampleop_curv2d")

Exit()
