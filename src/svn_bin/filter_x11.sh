#!/bin/bash

# Filter the engine link line so it will not include X11 libraries. CMake is adding
# them even though we don't want them.
for target in engine_ser_exe.dir engine_par_exe.dir
do
    edir="engine/main/CMakeFiles/$target"
    if test -e "$edir/link.txt" ; then
        sed "s/-lX11//g" $edir/link.txt > $edir/link1.txt
        sed "s/-lXext//g" $edir/link1.txt > $edir/link2.txt
        rm -f $edir/link1.txt
        mv $edir/link2.txt $edir/link.txt
    else
        echo "***** DID NOT SEE: $edir/link.txt   pwd=`pwd`"
    fi
done

exit 0






