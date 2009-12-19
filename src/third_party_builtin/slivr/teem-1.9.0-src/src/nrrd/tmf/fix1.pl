#!/usr/bin/perl 


$emit = 1;
while (<>) {

    # nix carraige returns
    s///g;
    
    # nix pragmas
    s/\#pragma warning.+$//g;

    if (/w[pm][0-9][a-z] = /) {
	chomp;         # nix newline
	s/ //g;        # nix spaces 
	s/;$//g;       # nix last semicolon
	foreach $pair (split /;/) {
	    ($var, $val) = split /=/, $pair;
	    $$var = $val;
	}
	next;
    }

    # nix the minimal function body before and after the switch lines
    s/  float result;//g;
    s/  int i;//g;
    s/  i = \(t\<0\) \? \(int\)t-1:\(int\)t;//g;
    s/  t = t - i;//g;
    s/  switch \(i\) \{//g;
    s/  \}//g;
    s/  return result;//g;

    # print lines that aren't all whitespace
    s/^ +$//g;
    if (!m/^$/) {
	print;
    }
}
