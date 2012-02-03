#!/bin/sh

# Make the sources
make

# Make the example programs
make main

# Make the Java docs
rm -rf docs
mkdir docs
javadoc -d docs *.java
cp stylesheet.css docs

# Make the jar
make pack

# Now, package it up.
VER=`cat ../VERSION`
rm jvisit*.tar.gz
rm -rf src
mkdir src
mkdir src/plots
mkdir src/operators
cp *.java src
cp ../plots/*/*.java src/plots
cp ../operators/*/*.java src/operators
tar zcf jvisit$VER.tar.gz visit.jar src docs *.class images/*.jpeg
rm -rf src
