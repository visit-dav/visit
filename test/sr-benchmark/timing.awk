# Awk script to get just the `meat' of renderer performance from lines in an
# engine_par .timing file.
BEGIN {
    printf "# %-20s %-8s %-10s %-10s\n", "Renderer", "Time", "cells", "pixels"
    switch=0
}

/IceTNM::Render/ {
    if(switch == 0) {
        # `index' separator; see gnuplot `help datafile index'.
        printf "\n\n"
        switch = 1
    }
    # 0 test; some strange cases have render e.g. 0 cells.  These cases aren't
    # really valid data, so we filter them out.
    if($9 != 0 && $4 != 0 && $6 != 0) {
        printf "%6u %21.4f %10d %10d\n", 1, $9, $4, $6
    }
}

/for\W*NM::Render/ {
    if($9 != 0 && $4 != 0 && $6 != 0) {
        printf "%6u %21.4f %10d %10d\n", 0, $9, $4, $6
    }
}
