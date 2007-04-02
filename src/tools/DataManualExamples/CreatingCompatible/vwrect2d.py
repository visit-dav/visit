import visit_writer

NX = 4
NY = 5
x = (0., 1., 2.5, 5.)
y = (0., 2., 2.25, 2.55,  5.)
z = 0.

# Create a zonal variable
zonal = []
index = 0
for j in range(NY-1):
    for i in range(NX-1):
        zonal = zonal + [index]
        index = index + 1

# Create a nodal variable
nodal = []
index = 0
for j in range(NY):
    for i in range(NX):
        nodal = nodal + [index]
        index = index + 1

vars = (("zonal", 1, 0, zonal), ("nodal", 1, 1, nodal))
visit_writer.WriteRectilinearMesh("vwrect2d.vtk", 0, x, y, z, vars)
