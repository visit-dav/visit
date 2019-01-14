#!/bin/sh
#------------------------------------------------------------------------------
# Install client-side hooks
#------------------------------------------------------------------------------
basedir=`git rev-parse --show-toplevel`
hooksdir="$basedir/.git/hooks/"
cp -v $basedir/scripts/git-hooks/pre-commit $hooksdir