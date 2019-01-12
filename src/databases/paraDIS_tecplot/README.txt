The paraDIS_tecplot file is to read paraDIS "cylinder files," so named because they describe the enclosing mesh around a paraDIS data set, and when these files are used, it's generally a cylindrical enclosure.  

A cylinder file is in the same format as a Tecplot data file.  It may optionally include an ORIENTATION X Y Z keyword that reorients the cylinder along the given axis, assuming it starts out with orientation 0 0 1. This keyword must come early in the file.  

