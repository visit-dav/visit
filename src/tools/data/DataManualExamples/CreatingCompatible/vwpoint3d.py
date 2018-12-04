import visit_writer, math

NPTS = 100

pts = []
data = []
for i in range(NPTS):
    # Make a point
    t = float(i) / float(NPTS-1)
    angle = 3.14159 * 10. * t
    pts = pts + [t * math.cos(angle), t * math.sin(angle), t]
    # Make a scalar
    data = data + [t * math.cos(angle)]

# Pass the data to visit_writer
vars = (("data", 1, 1, pts), ("ptsvec", 3, 1, pts))
visit_writer.WritePointMesh("vwpoint3d.vtk", 1, pts, vars)
