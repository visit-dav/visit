
# GK uses this to make sure that there are no filename clashes between
# different libraries, a constraint introduced with the Windows port.
# Currently this is very simple, perhaps a more sophisticated thing
# will be written later.  Doubtful.
#
# Usage is:
#
# cd src
# ls -1 {air,hest,biff,ell,nrrd,unrrdu,dye,moss,gage,bane,limn,hoover,mite,alan,ten,echo,coil,push}/*.c | perl names.pl | sort | wc -l
# ls -1 {air,hest,biff,ell,nrrd,unrrdu,dye,moss,gage,bane,limn,hoover,mite,alan,ten,echo,coil,push}/*.c | perl names.pl | sort | uniq | wc -l

while (<>) {
    chomp;
    ($dir, $name) = split '/', $_, 2;
    print "$name\n";
}
