# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tuvok.py
#
#  Tests: Tuvok volume rendering.
#
#  Programmer: Tom Fogal
#  Date:       May 30, 2009
#
# Modifications:
#
# ----------------------------------------------------------------------------
from __future__ import with_statement

# A handy container and context manager for datasets.
class Dataset:
  def __init__(self, filename, scalar):
    self._file = filename
    self._variable = scalar
  def filename(self): return self._file
  def variable(self): return self._variable
  def __enter__(self):
    OpenDatabase(self._file)
    return self
  def __exit__(self, tp, value, traceback):
    DeleteAllPlots()
    CloseDatabase(self._file)

def test_id():
  tid=0
  while True:
    tid += 1
    yield "".join(["tuvok_", str(tid)])
tid = test_id()

tex3d = VolumeAttributes()
tuvok = VolumeAttributes()
tex3d.rendererType = tex3d.Texture3D
tuvok.rendererType = tex3d.Tuvok

# We'll render twice: once via tex3d.  We're pretty particular about the OpenGL
# state in Tuvok, and if we're not careful we'll set something that one of the
# other plots isn't expecting.
with Dataset("../data/rect3d.silo", "u") as r3d:
  AddPlot("Volume", r3d.variable())

  SetPlotOptions(tex3d)
  DrawPlots()
  Test(tid.next())

  SetPlotOptions(tuvok)
  DrawPlots()
  Test(tid.next())

with Dataset("../data/noise.silo", "hardyglobal") as noise:
  AddPlot("Volume", noise.variable())
  SetPlotOptions(tex3d)
  DrawPlots()
  Test(tid.next())

  SetPlotOptions(tuvok)
  DrawPlots()
  Test(tid.next())

with Dataset("../data/FLOAT_indices.bov", "myvar") as floatbov:
  AddPlot("Volume", floatbov.variable())
  SetPlotOptions(tex3d)
  DrawPlots()
  Test(tid.next())
  SetPlotOptions(tuvok)
  DrawPlots()
  Test(tid.next())

#import os
#with Dataset(os.getenv("HOME") + "/data/csafe/vr-paper/bricked1/visit_ex.bov",
#             "g.mass/2") as csafe_bov:
#  print "CSAFE test... might take a while."
#  AddPlot("Volume", csafe_bov.variable())
#  SetPlotOptions(tex3d)
#  DrawPlots()
#  Test(tid.next())
#  SetPlotOptions(tuvok)
#  DrawPlots()
#  Test(tid.next())

Exit()
