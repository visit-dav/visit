#!/bin/bash

# This script needs to be run after cmake on BG/Q systems to filter out
# unwanted X11 dependencies that CMake places in the link line.

# Filter the engine link line so it will not include X11 libraries. CMake is adding
# them even though we don't want them. Also get rid of extra static/dynamic 
# link keywords that prevent the linker from making a good static executable.
for target in engine_ser_exe.dir engine_par_exe.dir
do
    edir="engine/main/CMakeFiles/$target"
    if test -e "$edir/link.txt" ; then
        sed "s/-lX11//g" $edir/link.txt > $edir/link1.txt
        sed "s/-lXext//g" $edir/link1.txt > $edir/link2.txt
        sed "s/-Wl,-Bstatic//g" $edir/link2.txt > $edir/link3.txt
        sed "s/-Wl,-Bdynamic//g" $edir/link3.txt > $edir/link4.txt
        rm -f $edir/link1.txt $edir/link2.txt $edir/link3.txt
        mv $edir/link4.txt $edir/link.txt
    else
        echo "***** DID NOT SEE: $edir/link.txt   pwd=`pwd`"
    fi
    if test -e "$edir/relink.txt" ; then
        sed "s/-lX11//g" $edir/relink.txt > $edir/relink1.txt
        sed "s/-lXext//g" $edir/relink1.txt > $edir/relink2.txt
        sed "s/-Wl,-Bstatic//g" $edir/relink2.txt > $edir/relink3.txt
        sed "s/-Wl,-Bdynamic//g" $edir/relink3.txt > $edir/relink4.txt
        rm -f $edir/relink1.txt $edir/relink2.txt $edir/relink3.txt
        mv $edir/relink4.txt $edir/relink.txt
    else
        echo "***** DID NOT SEE: $edir/relink.txt   pwd=`pwd`"
    fi
done
# Filter the visitconvert link line so it will not include X11 libraries. CMake 
# is adding them even though we don't want them. Also get rid of extra static/dynamic 
# link keywords that prevent the linker from making a good static executable.
for target in visitconvert_ser.dir visitconvert_par.dir
do
    edir="tools/convert/CMakeFiles/$target"
    if test -e "$edir/link.txt" ; then
        sed "s/-lX11//g" $edir/link.txt > $edir/link1.txt
        sed "s/-lXext//g" $edir/link1.txt > $edir/link2.txt
        sed "s/-Wl,-Bstatic//g" $edir/link2.txt > $edir/link3.txt
        sed "s/-Wl,-Bdynamic//g" $edir/link3.txt > $edir/link4.txt
        rm -f $edir/link1.txt $edir/link2.txt $edir/link3.txt
        mv $edir/link4.txt $edir/link.txt
    else
        echo "***** DID NOT SEE: $edir/link.txt   pwd=`pwd`"
    fi
    if test -e "$edir/relink.txt" ; then
        sed "s/-lX11//g" $edir/relink.txt > $edir/relink1.txt
        sed "s/-lXext//g" $edir/relink1.txt > $edir/relink2.txt
        sed "s/-Wl,-Bstatic//g" $edir/relink2.txt > $edir/relink3.txt
        sed "s/-Wl,-Bdynamic//g" $edir/relink3.txt > $edir/relink4.txt
        rm -f $edir/relink1.txt $edir/relink2.txt $edir/relink3.txt
        mv $edir/relink4.txt $edir/relink.txt
    else
        echo "***** DID NOT SEE: $edir/relink.txt   pwd=`pwd`"
    fi
done

# Filter the osmesavtktest link line so it will not include X11 libraries. CMake
# is adding them even though we don't want them. Also get rid of extra static/dynamic
# link keywords that prevent the linker from making a good static executable.
for target in osmesavtktest_ser.dir osmesavtktest_par.dir
do
    edir="tools/diagnostics/osmesatest/CMakeFiles/$target"
    if test -e "$edir/link.txt" ; then
        sed "s/-lX11//g" $edir/link.txt > $edir/link1.txt
        sed "s/-lXext//g" $edir/link1.txt > $edir/link2.txt
        sed "s/-Wl,-Bstatic//g" $edir/link2.txt > $edir/link3.txt
        sed "s/-Wl,-Bdynamic//g" $edir/link3.txt > $edir/link4.txt
        rm -f $edir/link1.txt $edir/link2.txt $edir/link3.txt
        mv $edir/link4.txt $edir/link.txt
    else
        echo "***** DID NOT SEE: $edir/link.txt   pwd=`pwd`"
    fi
    if test -e "$edir/relink.txt" ; then
        sed "s/-lX11//g" $edir/relink.txt > $edir/relink1.txt
        sed "s/-lXext//g" $edir/relink1.txt > $edir/relink2.txt
        sed "s/-Wl,-Bstatic//g" $edir/relink2.txt > $edir/relink3.txt
        sed "s/-Wl,-Bdynamic//g" $edir/relink3.txt > $edir/relink4.txt
        rm -f $edir/relink1.txt $edir/relink2.txt $edir/relink3.txt
        mv $edir/relink4.txt $edir/relink.txt
    else
        echo "***** DID NOT SEE: $edir/relink.txt   pwd=`pwd`"
    fi
done

exit 0






