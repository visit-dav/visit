import visit_writer

# Node coordinates
pts = (0., 0., 0., 2., 0., 0., 5., 0., 0.,
    3., 3., 0., 5., 3., 0., 0., 5., 0., 
    2., 5., 0., 4., 5., 0., 5., 5., 0.)

# Connectivity
connectivity = (
    (visit_writer.triangle, 1,3,6),
    (visit_writer.triangle, 3,7,6),
    (visit_writer.quad, 0,1,6,5),
    (visit_writer.quad, 1,2,4,3),
    (visit_writer.quad, 3,4,8,7)
)

# Data arrays
nodal = (1,2,3,4,5,6,7,8,9)
zonal = (1,2,3,4,5)

# Pass the data to visit_writer
vars = (("zonal", 1, 0, zonal), ("nodal", 1, 1, nodal))
visit_writer.WriteUnstructuredMesh("vwucd2d.vtk", 1, pts, connectivity, vars)
