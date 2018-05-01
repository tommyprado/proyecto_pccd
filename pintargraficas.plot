clear
reset
unset key

set multiplot layout 2,1 rowsfirst

# Make the x axis labels easier to read.
# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border


set xtics auto rotate

set title "ACCESOS A LA SECCION CRITICA"
set xlabel 'instante de tiempo T'
set ylabel 'sección critica'
set autoscale
set key outside
unset ytics

plot "pagos.dat" title "pagos" with fillsteps fs solid 0.3 noborder lt rgb "green" , \
"anulaciones.dat" title "anulaciones" with fillsteps fs solid 0.3 noborder lt rgb "red" , \
"prereservas.dat" title "prereservas" with fillsteps fs solid 0.3 noborder lt rgb "blue"  , \

set xlabel
set ylabel

unset xtics
set border 3
set autoscale
set style histogram rowstacked
set boxwidth 0.6 relative
plot for [COL=2:3] 'porcentajeSCtotal.dat' using COL:xticlabels(1) title columnheader



unset multiplot