#!/usr/bin/perl
##############################################################################
#
# Purpose: Subversion hook to check for files being added whose names differ
#          either from each other or from files currently in the repository
#          only in case. This is to ensure SVN clients can operate correctly
#          on systems which preserve case but don't disambiguate based on
#          case (OS X and Windows).
#
# Programmer: Mark C. Miller
# Created:    August 22, 2013
#
# The check involves two passes. The first examines all files being added
# in this commit and checks that there are no files whose names differ
# only in case. However, it restricts its work to only compare files in the
# same directory. If that first pass completes without error, then it moves
# on to get a list of files in the repository (it does some work to try to
# query for as small a list as possible by finding greatest common ancestor
# directory in the commit. It then compares files being added with files in
# the repository.
##############################################################################

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

#
# If clashes hash is empty, we're ok
#
my $msg;
if (keys(%clashes))
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
# If we made it this far, nothing *within* this commit has a clash conflict with
# itself. However, it maybe that something in this commit has a clash conflict
# with the current state of the repository. So, check that next.
#

#
# Find the shortest key in udirs. That will be the directory highest up
# in the repo under which all the commits have occured and represents the
# smallest 'tree' query we must make on the server to check for clashes.
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
# Query the repository for the full-paths of all the files (and dirs)
# this commit could be effecting.
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
    if ($#$v > 0) {
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
}

#
# If clashes hash is empty, we're ok
#
if (keys(%clashes))
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

# all is well!
exit 0;
