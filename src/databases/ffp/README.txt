A plugin to read and plot 3D Far Field Patterns, also called Radiation
Patterns or Antenna Diagrams for electromagnetism such as in:
https://en.wikipedia.org/wiki/Radiation_pattern
This is also suitable for RCS (Radar Cross Section) files.

The plugin can handle 1) cartesian splits of the (theta,phi)
parametrization of the sphere, or, newly added, 2) whatever points on
the sphere when there exists a ".unv" file describing the mesh on
the sphere using triangles and/or quadrangles : both these features do
not need a third party library.
Then, a third feature has been added, where you can have 3) whatever
points and the mesh is made, using triangles only, on the fly by the
plugin using a third party library: stripack from Robert J. Renka.
This requires a license agreement from ACM found in file "license.txt".

Input file format is:
 theta angle in degrees, phi angle in degrees, G Gain in dB, Phase in degrees
Comment lines start with either # or % (as in gnupot or matlab).

Provided examples are the NASA PEC (Perfectly Electric Conductor) Almond
[Alex C. Woo, Helen T. G. Wang, Michael J. Schuh, and Michael L. Sanders,
"Benchmark Radar Targets for the Validation of Computational
Electromagnetic Programs" IEEE Antennas and Propagation Magazine Vol.
35, No. 1 February 1993.]
as well as a Mie Sphere (eps=7.414, mu=1, wave length lambda=50
nm, sphere radius is 100 nm) which has an analytical solution using
spherical harmonic functions.

Compressed (".gz") files are read out of the box without ZipWrapper
(using gzgets instead of gets).

Plots available are:
 - "mffp" : nodes position in (r,th,phi) spherical coordinates are
            R,theta,phi where R=10^(G-Gmax),
            where Gmax is the max of all Gain values,
 - "cffp" : nodes positions are 1+10^(G-Gmax)
 - "sffp" : nodes positions are merely (1,theta,phi).

Provided example data files are
1) Feature : cartesian parametrization of the sphere:
sphereMie.ffp and nasa_almond_pec.ffp.gz,
python files are sphereMie.py that produces sphereMie0000.png and
nasa_almond_pec.py that produces nasa_almond_pec0000.png.
sphereMie.py uses the "mffp" plot,
nasa_almond_pec.py uses the "cffp" plot.
2) Feature : user provides the mesh of the sphere in a ".unv" file :
mksn10.py uses the "sffp" plot and the unv file mksn10.unv
3) Feature : mesh is made on the fly using ACM code 772:
mksn20.py uses the "cffp" plot and that is it

To generate the images:
visit -cli -nowin -s mksn10.py
visit -cli -nowin -s mksn20.py

# Use and compilation of the stripack library
ACM Software License Agreement
https://www.acm.org/publications/policies/software-copyright-notice
http://www.netlib.org/toms/index.html
wget http://www.netlib.org/toms/772.gz
http://www.cse.unt.edu/~renka/
Robert J. Renka
                                  Dept. of Computer Science
                                       Univ. of North Texas
                                           renka@cs.unt.edu
                                                   07/08/99
robert.renka@unt.edu

wget http://www.netlib.org/toms/772.gz
gunzip 772.gz
chmod +x ./772
./772
gfortran -fdefault-real-8 -fdefault-double-8 -fPIC -c Fortran77/Src/Sp/src.f
gfortran -fPIC -shared -o libstripack.so src.o
cp -f libstripack.so /path_to_visit/visit-version/arch/lib/.

# One can alternatively and technically equivalently use the Fortran 90 version
http://people.math.sc.edu/imi/dasiv/People/Person/johnburkardt.html
John Burkardt
206A Sumwalt College
Department of Mathematics
University of South Carolina
Columbia, South Carolina, 29208
803-777-6531
http://people.math.sc.edu/Burkardt/
burkardt@mailbox.sc.edu

wget https://people.sc.fsu.edu/~jburkardt/f_src/stripack/stripack.f90
gfortran -fPIC -c stripack.f90
gfortran -fPIC -shared -o libstripack.so stripack.o
cp -f libstripack.so /path_to_visit/visit-version/arch/lib/.

# Using the void stripack:
gfortran -fPIC -fdefault-real-8 -fdefault-double-8 -c voidstripack.f
gfortran -fPIC -shared -o libstripack.so voidstripack.o
cp libstripack.so /path_to_visit/visit-version/arch/lib/.

