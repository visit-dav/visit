import math
from array import array
nx = 10
ny = 10
nz = 10
vals = array('f')
for iz in range(nz):
    z = float(iz)
    for iy in range(ny):
        y = float(iy)
        for ix in range(nx):
            x = float(ix)
            vals.append(math.sqrt(x*x+y*y+z*z))

f = open("density.bof", "wb")
vals.tofile(f)
f.close()
