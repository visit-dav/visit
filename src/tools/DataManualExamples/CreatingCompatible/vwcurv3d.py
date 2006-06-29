import visit_writer

NX = 4
NY = 3
NZ = 2

# Curvilinear mesh points stored x0,y0,z0,x1,y1,z1,...
pts = (0, 0.5, 0, 1, 0, 0, 2, 0, 0,
    3, 0.5, 0, 0, 1, 0, 1, 1, 0,
    2, 1, 0, 3, 1, 0, 0, 1.5, 0,
    1, 2, 0, 2, 2, 0, 3, 1.5, 0, 
    0, 0.5, 1, 1, 0, 1, 2, 0, 1,
    3, 0.5, 1, 0, 1, 1, 1, 1, 1,
    2, 1, 1, 3, 1, 1, 0, 1.5, 1,
    1, 2, 1, 2, 2, 1, 3, 1.5, 1)

# Create a zonal variable
zonal = []
index = 0
for k in range(NZ-1):
    for j in range(NY-1):
        for i in range(NX-1):
            zonal = zonal + [index]
            index = index + 1

# Create a nodal variable
nodal = []
index = 0
for k in range(NZ):
    for j in range(NY):
        for i in range(NX):
            nodal = nodal + [index]
            index = index + 1

# Pass the data to visit_writer to write a binary VTK file.
dims = (NX, NY, NZ)
vars = (("zonal", 1, 0, zonal), ("nodal", 1, 1, nodal))
visit_writer.WriteCurvilinearMesh("vwcurv3d.vtk", 0, dims, pts, vars)
