#!/usr/bin/perl -w

print "\n\#include \"nrrd.h\"\n\n";

print " /* ************************************************* */\n";
print " /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */\n";
print " /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */\n";
print " /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */\n";
print " /*                                                   */\n";
print " /*                                                   */\n";
print " /*       THIS FILE AUTOMATICALLY GENERATED FROM      */\n";
print " /*        PERL SCRIPTS IN THE tmf SUBDIRECTORY       */\n";
print " /*         EDIT THOSE SCRIPTS, NOT THIS FILE!        */\n";
print " /*                                                   */\n";
print " /*                                                   */\n";
print " /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */\n";
print " /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */\n";
print " /* !! WARNING !!! WARNING !!! WARNING !!! WARNING !! */\n";
print " /* ************************************************* */\n\n";

# generate a stub kernel for when the user incorrectly indexes
# into ef == 0, which is undefined; ef must be >= 1
print "double\n";
print "_nrrd_TMFBAD_Int(const double *parm) {\n";
print "  AIR_UNUSED(parm);\n";
print "  fprintf\(stderr, \"_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\\n\"\);\n";
print "  return 0.0;\n";
print "}\n\n";
print "double\n";
print "_nrrd_TMFBAD_Sup(const double *parm) {\n";
print "  AIR_UNUSED(parm);\n";
print "  fprintf\(stderr, \"_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\\n\"\);\n";
print "  return 0.0;\n";
print "}\n\n";
print "double\n";
print "_nrrd_TMFBAD_1_d(double x, const double *parm) {\n";
print "  AIR_UNUSED(x);\n";
print "  AIR_UNUSED(parm);\n";
print "  fprintf\(stderr, \"_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\\n\"\);\n";
print "  return 0.0;\n";
print "}\n\n";
print "float\n";
print "_nrrd_TMFBAD_1_f(float x, const double *parm) {\n";
print "  AIR_UNUSED(x);\n";
print "  AIR_UNUSED(parm);\n";
print "  fprintf\(stderr, \"_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\\n\"\);\n";
print "  return 0.0;\n";
print "}\n\n";
print "void\n";
print "_nrrd_TMFBAD_N_d(double *f, const double *x, size_t len, const double *parm) {\n";
print "  AIR_UNUSED(f);\n";
print "  AIR_UNUSED(x);\n";
print "  AIR_UNUSED(len);\n";
print "  AIR_UNUSED(parm);\n";
print "  fprintf\(stderr, \"_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\\n\"\);\n";
print "}\n\n";
print "void\n";
print "_nrrd_TMFBAD_N_f(float *f, const float *x, size_t len, const double *parm) {\n";
print "  AIR_UNUSED(f);\n";
print "  AIR_UNUSED(x);\n";
print "  AIR_UNUSED(len);\n";
print "  AIR_UNUSED(parm);\n";
print "  fprintf\(stderr, \"_nrrd_TMFBAD: Invalid TMF indexing: ef == 0\\n\"\);\n";
print "}\n\n";
print "NrrdKernel\n";
print "_nrrdKernel_TMFBAD = {\n";
print "  \"TMFBAD\",\n";
print "  1, _nrrd_TMFBAD_Sup, _nrrd_TMFBAD_Int,\n";
print "  _nrrd_TMFBAD_1_f, _nrrd_TMFBAD_N_f,\n";
print "  _nrrd_TMFBAD_1_d, _nrrd_TMFBAD_N_d\n";
print "};\n";

%needk = ();
%support = ();

$maxD = 0;
$maxC = 0;
$maxA = 0;
# process body of file
while (<>) {
    if (/\#define/) {
	print;
	next;
    }
    if (/float (d[n012]_c[n0123]_[1234]ef)\(float a/) {
	$kern = "TMF_$1";
	if ($_ =~ m/d([012])_c([0123])_([1234])ef/) {
	    $maxD = $1 > $maxD ? $1 : $maxD;
	    $maxC = $2 > $maxC ? $2 : $maxC;
	    $maxA = $3 > $maxA ? $3 : $maxA;
	}
	print "\n/* ------------------------ $kern --------------------- */\n\n";
	print "\#define ${kern}(a, i, t) ( \\\n";
	while (<>) {
	    # for when $kern is just a wrapper around $_needk
	    if (/return (d[n012]_c[n0123]_[1234]ef)\((.+), *t\)/) {
		$_needk = "TMF_$1";
		# finish the #define, and pass in the correct first arg
		print "  ${_needk}((double)($2), i, t))\n\n";

		# remember that this kernel needs another in order to know the support
		$needk{$kern} = $_needk;
		last;
	    }

	    # process the switch cases
	    s/case ([0-9]): +result *= *(.+); +break;$/\(i == $1 ? $2 : \\/g;
	    $n = $1;
	    $sup = ($n + 1)/2;
	    
	    # when we've reached the end of the switch cases
	    if (/default: result = 0;/) {
		# the default case
		print "  0";

		# print one end paren for each of the cases
		for ($i=0; $i <= $n; $i++) {
		    print ")";
		}
		
		# and one more end paren to finish the #define
		print ")\n\n";
		
		# remember what the support is
		$support{$kern} = $sup;
		last;
	    }

	    print;
	}
    }
}

# generate 3-D array of all TMFs

if (2 != $maxD) {
    print "teem/src/nrrd/tmf/fix2.pl error: maxD = $maxD, not 2\n";
    exit;
}
if (3 != $maxC) {
    print "teem/src/nrrd/tmf/fix2.pl error: maxC = $maxC, not 3\n";
    exit;
}
if (4 != $maxA) {
    print "teem/src/nrrd/tmf/fix2.pl error: maxA = $maxA, not 4\n";
    exit;
}

# print out code for all kernels
for ($_d=0; $_d<=3; $_d++) {
    $d = (($_d > 0) ? $_d-1 : "n");
    for ($_c=0; $_c<=4; $_c++) {
	$c = (($_c > 0) ? $_c-1 : "n");
	for ($ef=1; $ef<=4; $ef++) {
	    $kern = "TMF_d${d}_c${c}_${ef}ef";
	    print "\n/* ------------------------ $kern --------------------- */\n\n";
	    print blah($kern);
	}
    }
}

# create master array of all kernels
print "\nNrrdKernel *const\n";
print "nrrdKernelTMF[4][5][5] = {\n";
for ($_d=0; $_d<=3; $_d++) {
    $d = (($_d > 0) ? $_d-1 : "n");
    print "  {            /* d = $d */ \n";
    for ($_c=0; $_c<=4; $_c++) {
	$c = (($_c > 0) ? $_c-1 : "n");
	print "    {\n";
	print "       &_nrrdKernel_TMFBAD,\n";
	for ($ef=1; $ef<=4; $ef++) {
	    $kern = "TMF_d${d}_c${c}_${ef}ef";
	    print "       &_nrrdKernel_${kern},\n";
	}
	print "    },\n";
    }
    print "  },\n";
}
print "};\n\n";

print "const unsigned int nrrdKernelTMF_maxD = $maxD;\n";
print "const unsigned int nrrdKernelTMF_maxC = $maxC;\n";
print "const unsigned int nrrdKernelTMF_maxA = $maxA;\n";

sub blah {
    $kern = $_[0];
    # there seems to be at most two levels of indirection
    $sup = (exists $support{$kern}
	    ? $support{$kern}
	    : (exists $support{$needk{$kern}}
	       ? $support{$needk{$kern}}
	       : $support{$needk{$needk{$kern}}}));
    $integral = ("double "
		 . "_nrrd_${kern}_Int\(const double *parm\) {\n"
                 . "  AIR_UNUSED(parm);\n"
		 . (($kern =~ m/_d0_/ || $kern =~ m/_dn_/)
		    ? "  return 1.0;\n"
		    : "  return 0.0;\n")
		 . "}\n\n");
    $support = ("double "
		. "_nrrd_${kern}_Sup\(const double *parm\) {\n"
                . "  AIR_UNUSED(parm);\n"
		. "  return ${sup};\n"
		. "}\n\n");
    $_1_d = ("double\n"
	     . "_nrrd_${kern}_1_d\(double x, const double *parm\) {\n"
	     . "  int i;\n\n"
             . "  AIR_UNUSED(parm); /* ${kern} */\n"
	     . "  x += $sup;\n"
	     . "  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */\n"
	     . "  x -= i;\n"
	     . "  return ${kern}\(parm[0], i, x\);\n"
	     . "}\n\n");
    $_1_f = ("float\n"
	     . "_nrrd_${kern}_1_f\(float x, const double *parm\) {\n"
	     . "  int i;\n\n"
             . "  AIR_UNUSED(parm); /* ${kern} */\n"
	     . "  x += $sup;\n"
	     . "  i = (int)((x<0) ? x-1 : x); /* HEY scrutinize cast */\n"
	     . "  x -= i;\n"
	     . "  return AIR_CAST(float, ${kern}\(parm[0], i, x\));\n"
	     . "}\n\n");
    $_N_d = ("void\n"
	     . "_nrrd_${kern}_N_d(double *f, const double *x, size_t len, const double *parm) {\n"
	     . "  double t;\n"
	     . "  size_t I;\n"
	     . "  int i;\n\n"
             . "  AIR_UNUSED(parm); /* ${kern} */\n"
	     . "  for \(I=0; I<len; I++\) {\n"
	     . "    t = x[I] + $sup;\n"
	     . "    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */\n"
	     . "    t -= i;\n"
	     . "    f[I] = ${kern}\(parm[0], i, t\);\n"
	     . "  }\n"
	     . "}\n\n");
    $_N_f = ("void\n"
	     . "_nrrd_${kern}_N_f(float *f, const float *x, size_t len, const double *parm) {\n"
	     . "  float t;\n"
	     . "  size_t I;\n"
	     . "  int i;\n\n"
             . "  AIR_UNUSED(parm); /* ${kern} */\n"
	     . "  for \(I=0; I<len; I++\) {\n"
	     . "    t = x[I] + $sup;\n"
	     . "    i = (int)((t<0) ? t-1 : t); /* HEY scrutinize cast */\n"
	     . "    t -= i;\n"
	     . "    f[I] = AIR_CAST(float, ${kern}\(parm[0], i, t\));\n"
	     . "  }\n"
	     . "}\n\n");
    $kdef = ("NrrdKernel\n"
	     . "_nrrdKernel_${kern} = {\n"
	     . "  \"${kern}\",\n"
	     . "  1, _nrrd_${kern}_Sup, _nrrd_${kern}_Int,\n"
	     . "  _nrrd_${kern}_1_f,  _nrrd_${kern}_N_f,\n"
	     . "  _nrrd_${kern}_1_d,  _nrrd_${kern}_N_d\n"
	     . "};\n\n");
    ($integral 
     . $support 
     . $_1_d 
     . $_1_f
     . $_N_d
     . $_N_f
     . $kdef);
}
