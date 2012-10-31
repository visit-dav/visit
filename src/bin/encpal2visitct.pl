#!/usr/bin/perl

#
# Tool to convert an EnSight Color Pallet (color table) .cpal file to
# a VisIt color table file. Written in Perl because perl also available
# on Windows.
#
# Mark C. Miller, Tue Oct 30 18:08:10 PDT 2012
#
#
local($outfile) = $ARGV[0];
local($numcolors) = 0;
local(@colors);
while (<>) {
    if (/^number_of_colors\s([0-9]*)$/) {
        $numcolors = $1;
        print STDERR "numcolors = $numcolors\n";
    } elsif (/^colors$/) {
        print STDERR "skipping \"colors\" line\n";
    } elsif (/^([0-9]*.?[0-9]*)\s([0-9]*.?[0-9]*)\s([0-9]*.?[0-9]*)(\s[0-9]*.?[0-9]*)?$/) {
        $tmp = $4;
        chomp($tmp);
        if ($tmp ne "") {
            push(@colors, "$1\@$2\@$3\@$tmp");
            print STDERR "got color \"$1, $2, $3, $tmp\"\n";
        } else {
            push(@colors, "$1\@$2\@$3\@0.0");
            print STDERR "got color \"$1, $2, $3\"\n";
        }
  
    } else {
    }
}

$outfile =~ s/(.*).cpal$/$1.ct/;
print STDERR "outfile =\"$outfile\"\n";
open(my $fh, ">", $outfile);

print $fh "<?xml version=\"1.0\"?>\n";
print $fh "<Object name=\"ColorTable\">\n";
print $fh "    <Field name=\"Version\" type=\"string\">2.5.2</Field>\n";
local($i) = 0;
print $fh "    <Object name=\"ColorControlPointList\">\n";
local($len) = $numcolors-1;
foreach $color (@colors) {
    local(@cvals) = split('@',$color);
    local($rval) = int($cvals[0]*255);
    local($gval) = int($cvals[1]*255);
    local($bval) = int($cvals[2]*255);
    local($aval) = int($cvals[3]*255);
    print $fh "        <Object name=\"ColorControlPoint\">\n";
    print $fh "            <Field name=\"colors\" type=\"unsignedCharArray\" length=\"$len\">$rval $gval $bval $aval</Field>\n";
    local($fval) = $i / ($numcolors-1);
    print $fh "            <Field name=\"position\" type=\"float\">$fval</Field>\n";
    print $fh "        </Object>\n";
    $i = $i + 1;
}
print $fh "        <Field name=\"externalFlag\" type=\"bool\">true</Field>\n";
print $fh "    </Object>\n";
print $fh "</Object>\n";
close $fh;
