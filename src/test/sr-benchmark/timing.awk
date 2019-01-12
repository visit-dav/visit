# Awk script to get just the `meat' of renderer performance from lines in an
# engine_par .timing file.  It outputs SQL for populating a database of data.
# This requires a `pat' variable to set on startup!
BEGIN {
    print "-- nproc  icet  transparent ngpus ncells  npixels  rendering time"
    n_proc=-1
    if(pat == 0) {
        print "'pat' variable not set!"
        exit 0
    }
    transparent=0
    n_gpus=0
}

/Initializing a [[:digit:]]+ processor engine/ {
    n_proc=$5
}
/Setting up [[:digit:]]+ GPUs for HW/ {
    n_gpus=$5
}

/Checking multipass rendering (enabled)/ {
    transparent=1
}
/Checking multipass rendering (disabled)/ {
    transparent=0
}

# We match against everything and do a match on a variable, because we want
# conditional matching based on a program argument.
{
    if(match($0, pat) != 0) {
        icet=0
        if(match($0, "IceT") != 0) {
            icet=1
        }
        r_time=$9
        n_cells=$4
        n_pixels=$6
        # 0 test; some strange cases render e.g. 0 cells.  These cases aren't
        # really valid data, so we filter them out.
        if($9 != 0 && $4 != 0 && $6 != 0) {
            printf "INSERT INTO rendering VALUES ("                      \
                   "'%d',  '%d',    '%d',      '%d',    '%d',   '%d', "  \
                   "'%f');\n",                                           \
                   n_proc, icet, transparent, n_gpus, n_cells, n_pixels, \
                   r_time
        }
    }
}
