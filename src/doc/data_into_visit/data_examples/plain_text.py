import math

f = open("curves.csv", "wt")
f.write("angle sine cosine\n")

npts = 73
for i in range(npts):
    angle_deg = float(i) * (360. / float(npts-1))
    angle_rad = angle_deg * (3.1415926535 / 180.)
    sine = math.sin(angle_rad)
    cosine = math.cos(angle_rad)
    f.write("%g %g %g\n" % (angle_deg, sine, cosine))

f.close()

import math

f = open("points.csv", "wt")
f.write("x y z value\n")

n = 100
for i in range(n):
    t = float(i) / float(n-1)
    angle = t * (math.pi * 2.) * 5.
    r = t * 10.
    x = r * math.cos(angle)
    y = r * math.sin(angle)
    z = t * 10.
    value = math.sqrt(x*x + y*y + z*z)
    f.write("%g %g %g %g\n" % (x,y,z,value))

f.close()

import math

f = open("regulargrid2d.csv", "wt")
f.write("density c2 c3 c4 c5 c6 c7 c8\n")

nx = 8
ny = 10
for iy in range(ny):
    y = float(iy)
    for ix in range(nx):
        x = float(ix)
        dist = math.sqrt((x - 3.5) * (x - 3.5) + (y - 4.5) * (y - 4.5))
        if (ix < nx - 1):
           f.write("%g " % dist)
        else:
           f.write("%g\n" % dist)

f.close()
