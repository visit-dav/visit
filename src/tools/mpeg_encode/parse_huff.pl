# 
# Copyright (c) 1993 The Regents of the University of California.
# All rights reserved.
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without written agreement is
# hereby granted, provided that the above copyright notice and the following
# two paragraphs appear in all copies of this software.
# 
# IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
# OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
# CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
# ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
# 

#  
#  $Header: /n/picasso/users/dwallach/vid2/mpeg_encode/RCS/parse_huff.pl,v 1.3 1993/01/18 10:56:03 dwallach Exp $
#  $Log: parse_huff.pl,v $
# Revision 1.3  1993/01/18  10:56:03  dwallach
# got RCS headers in huff.c and huff.h working
#
# Revision 1.3  1993/01/18  10:56:03  dwallach
# got RCS headers in huff.c and huff.h working
#
# Revision 1.2  1993/01/18  10:17:29  dwallach
# RCS headers installed, code indented uniformly
#
# Revision 1.2  1993/01/18  10:17:29  dwallach
# RCS headers installed, code indented uniformly
#
#

# this program's purpose in life is to parse the Huffman tables
# and output C code which is awfully useful for translation

# typical usage:
# perl parse_huff.pl huffman.table
# ---> generates huff.c and huff.h

# source format:  # is a comment character
# Otherwise, there are three tab-separated fields in a line:
#    Run, Level, and VLC Code
# Run and Level are integers, corresponding to the RLE coding.
# The VLC code is what bits to generate, and is expected to be
#    composed of 1, 0, space, and 's'.  Spaces are ignored, and
#    s corresponds to the sign bit.  In the output of this program,
#    We'll completely right-shift the data, with a 0 for the sign
#    bit.  The encoder will make appropriate changes before outputing.


open(HUFFC, "> huff.c") || die "Can't write huff.c: $!\n";
open(HUFFH, "> huff.h") || die "Can't write huff.h: $!\n";

sub encode {
    local($run, $level, $vlc) = @_;
    local($result) = 0;
    local($tmp);
    $vlc =~ s/\s//g;           # remove whitespace

    local($bits) = length($vlc);

    foreach (split(//, $vlc)) {
	$_ = 0 if $_ eq 's';
	$result = ($result << 1) + $_;
    }

    $tmp = sprintf("0x%x", $result);
    eval "\$vlc$run[$level] = \$tmp";
    eval "\$bits$run[$level] = \$bits";
}

while(<>) {
    chop;
    s/\s*#.*$//;       # remove comments
    next if /^\s*$/;   # continue if the line is purely whitespace

    ($run, $level, $vlc) = split(/\t/);
    &encode($run, $level, $vlc);

    #
    # we have the +1's to insure the sizes of C arrays are correct
    #
    $maxlevel[$run] = $level+1 if $level >= $maxlevel[$run];
    $maxlevel = $level+1 if $level >= $maxlevel;
    $maxrun = $run+1 if $run >= $maxrun;
}

#
# fix the entries that aren't defined
#
for($run = 0; $run < $maxrun; $run++) {
    eval "\$vlc$run[0] = '0x0' if \$vlc$run[0] eq ''";
    eval "\$bits$run[0] = '0' if \$bits$run[0] eq ''";
}

#
# now, output some C code
#

printf HUFFC <<'EOF', $maxrun, join(", ", @maxlevel);
/*
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*  
 *  $Header: /n/picasso/users/dwallach/vid2/mpeg_encode/RCS/parse_huff.pl,v 1.3 1993/01/18 10:56:03 dwallach Exp $
 */

/*  
 *  THIS FILE IS MACHINE GENERATED!  DO NOT EDIT!
 */
#include "mtypes.h"
#include "huff.h"

int huff_maxlevel[%s] = { %s };

EOF
for($run = 0; $run < $maxrun; $run++) {
    printf HUFFC <<EOF, eval "join(', ', \@vlc$run)", eval "join(', ', \@bits$run)";
uint32 huff_table$run[$maxlevel[$run]] = { %s };
int huff_bits$run[$maxlevel[$run]] = { %s };

EOF
}

printf HUFFC "uint32 *huff_table[$maxrun] = { ";
for($run = 0; $run < $maxrun; $run++) {
    printf HUFFC "%shuff_table$run", ($run)?", ":"";
}
printf HUFFC " };\n";

printf HUFFC "int *huff_bits[$maxrun] = { ";
for($run = 0; $run < $maxrun; $run++) {
    printf HUFFC "%shuff_bits$run", ($run)?", ":"";
}
printf HUFFC " };\n";
close HUFFC;

printf HUFFH <<'EOF', $maxrun, $maxlevel;
/*
 * Copyright (c) 1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*  
 *  $Header: /n/picasso/users/dwallach/vid2/mpeg_encode/RCS/parse_huff.pl,v 1.3 1993/01/18 10:56:03 dwallach Exp $
 */

/*  
 *  THIS FILE IS MACHINE GENERATED!  DO NOT EDIT!
 */
#define HUFF_MAXRUN	%s
#define HUFF_MAXLEVEL	%s

extern int huff_maxlevel[];
extern uint32 *huff_table[];
extern int *huff_bits[];
EOF

