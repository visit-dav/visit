#!/usr/bin/perl

use File::Basename;

my $REPOS=$ARGV[0];
my $TXN=$ARGV[1];
my $FLIST=$ARGV[2];

#
# Suck the whole FLIST file into memory
#
my @flist;
open(my $flistfile, "<", $FLIST);
while (<$flistfile>) {
    push @flist, $_;
}
close($flistfile);

#
# build hash of dirnames and files in those dirs that are being
# 'A'dded in this commit
#
my %udirs;
foreach my $line (@flist) {
    if ($line =~ /^A/) {
        chomp($line);
        $line =~ s/^A *(.*)/$1/;
        my $bname = basename($line);
        my $dname = dirname($line);
        push @{ $udirs{$dname} }, $bname;
    }
}
# If nothing is being added, we're done
if (!%udirs) {
    exit 0;
}



#
# First pass to check that nothing *within* this commit
# is a case clash.
#
my %clashes;
while (my ($k,$v)=each %udirs) {
    if ($#$v > 0)
    {
        for (my $i = 0; $i < $#$v+1; $i++) {
            for (my $j = $i+1; $j < $#$v+1; $j++) {
                if (lc($$v[$i]) eq lc($$v[$j])) {
                    my $clashkey = "$k/$$v[$i]";
                    my $clashval = "$k/$$v[$j]";
                    push @{ $clashes{$clashkey} }, $clashval;
                }
            }
        }
    }
}

my $msg;
if (%clashes)
{
    foreach $key (sort(keys %clashes)) {
        $msg = "$msg\nFile \"$key\" case-clashes with...\n";
        foreach $clash (@{$clashes{$key}}) {
            $msg = "$msg    \"$clash\"\n";
        }
    }
    `log "$msg"`;
    exit 1;
}

#
# find the shortest key in udirs. That will be the directory highest up
# in the repo under which all the commits have occured and represents the
# largest 'tree' query we must make on the server to check for clashes.
#
my $minkey;
my $min = 1000;
foreach my $key (sort(keys %udirs)) {
    my $slashcnt = $key =~ tr/\///;
    if ($slashcnt < $min) {
        $min = $slashcnt;
        $minkey = $key;
    } 
}
# back up on more directory
$minkey = dirname($minkey);

#
# If we made it this far, nothing *within* this commit has a clash conflict with
# itself. However, it maybe that something in this commit has a clash conflict
# with the current state of the repository. So, check that next.
#
my @treelist = `svnlook tree --full-paths $REPOS $minkey`;
my %repodirs;
foreach my $line (@treelist) {
    chomp($line);
    my $bname = basename($line);
    my $dname = dirname($line);
    push @{ $repodirs{$dname} }, $bname;
}

#
# Compare the files we're adding in this commit with all the stuff we
# already have in the repository.
#
while (my ($k,$v)=each %udirs) {
    foreach $fc (@{$v}) {
        foreach $fr (@{$repodirs{$k}}) {
            if (lc($fc) eq lc($fr)) {
                my $clashkey = "$k/$fc";
                my $clashval = "$k/$fr";
                push @{ $clashes{$clashkey} }, $clashval;
            }
        }
    }
}

if (%clashes)
{
    foreach $key (sort(keys %clashes)) {
        $msg = "$msg\nFile \"$key\" case-clashes with repository file...\n";
        foreach $clash (@{$clashes{$key}}) {
            $msg = "$msg    \"$clash\"\n";
        }
    }
    `log "$msg"`;
    exit 1;
}
