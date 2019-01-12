Developer: Olivier Cessenat

A plugin to read and plot 3D Far Field Patterns, also called Radiation
Patterns or Antenna Diagrams for electromagnetism such as in:
https://en.wikipedia.org/wiki/Radiation_pattern
This is also suitable for RCS (Radar Cross Section) files.

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

Provided example data files are sphereMie.ffp and nasa_almond_pec.ffp.gz,
python files are sphereMie.py that produces sphereMie0000.png and
nasa_almond_pec.py that produces nasa_almond_pec0000.png.

sphereMie.py uses the "mffp" plot,
nasa_almond_pec.py uses the "cffp" plot.


Generated archive using :
tar cvfz /tmp/pluffp.tgz *.* Makefile

