import visit_writer, math

NX = 10
NY = 20
NZ = 30

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
for k in range(NZ):
    for j in range(NY):
        for i in range(NX):
            nodal = nodal + [math.sqrt(i*i + j*j + k*k)]

# Use visit_writer to write a regular mesh with data.
dims = (NX, NY, NZ)
vars = (("zonal", 1, 0, zonal), ("nodal", 1, 1, nodal))
visit_writer.WriteRegularMesh("vwregmesh2.vtk", 0, dims, vars)
