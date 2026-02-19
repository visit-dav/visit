import sys
from pathlib import Path

###############################################################################
# Function: main
#
# Purpose:    This is the main function for the program.
#
# Programmer: Brad Whitlock
# Date:       Mon Sep 11 17:26:02 PDT 2017
#
# Modifications:
#   Kathleen Biagas, Tue Feb 10, 2026
#   Use pathlib with Source command to prevent syntax warning on Windows.
#
###############################################################################

def main():
    visitcinema = ""
    for arg in sys.argv[0:]:
        if arg.find("visitcinema") != -1:
            visitcinema = arg.replace("visitcinemamain", "visitcinema")
            break

    if visitcinema == "":
        print("The visitcinemamain script could not locate the visitcinema script!")
        sys.exit(-1)
    else:
        # Source the visitcinema.py script, which contains all of the 
        # code for the visitcinema class.
        Source(Path(visitcinema).as_posix())

        cinema = VisItCinema()
        cinema.ProcessArguments()
        cinema.CreatePlots()
        cinema.Execute()
        sys.exit(0)

#
# Call the main function.
#
main()
