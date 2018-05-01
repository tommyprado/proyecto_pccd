
clear
reset


# Make the x axis labels easier to read.
# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border


set xlabel
set ylabel
set key outside
unset xtics
set border 3
set autoscale
set style histogram rowstacked
set boxwidth 0.6 relative
plot for [COL=2:3] 'porcentajeSCtotal.dat' using COL:xticlabels(1) title columnheader