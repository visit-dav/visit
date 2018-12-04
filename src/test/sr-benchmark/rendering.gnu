set terminal postscript color enhanced
set title "Scalable Rendering Performance"
set output "SR-nodes-time.eps"
set xlabel "Nodes"
set ylabel "Rendering time"
set autoscale

plot \
    "sr.data" using 1:4 smooth unique title 'Existing' with linespoints lw 2, \
    "icet.data" using 1:4 smooth unique title 'IceT' with linespoints lw 2

set title "Scalable Rendering Performance"
set output "SR-cells-time.eps"
set xlabel "Cells"
set ylabel "Rendering Time"
plot \
    "sr.data" using 2:4 smooth unique title 'Existing' with linespoints lw 2, \
    "icet.data" using 2:4 smooth unique title 'IceT' with linespoints lw 2

set title "Scalable Rendering Performance"
set output "SR-pixels-time.eps"
set xlabel "Pixels"
set ylabel "Rendering Time"
plot \
    "sr.data" using 3:4 smooth unique title 'Existing' with linespoints lw 2, \
    "icet.data" using 3:4 smooth unique title 'IceT' with linespoints lw 2

set title "Scalable Rendering Performance"
set output "SR-proc-pixels.eps"
set xlabel "Processes"
set ylabel "Pixels"
set zlabel "Rendering\nTime\n(s)"
set hidden3d
set dgrid3d 32,24,2
set isosample 100
set style data lines
set ticslevel 0
splot \
    "icet.data" using 1:3:4 title 'IceT', \
    "sr.data" using 1:3:4 title 'Existing'

set title "Scalable Rendering Performance"
set output "SR-proc-cells.eps"
set xlabel "Processes"
set ylabel "Cells"
set zlabel "Rendering\nTime\n(s)"
set hidden3d
set dgrid3d 32,24,8
set isosample 100
set parametric
set style data lines
set ticslevel 0
splot \
    "icet.data" using 1:2:4 title 'IceT', \
    "sr.data" using 1:2:4 title 'Existing'
