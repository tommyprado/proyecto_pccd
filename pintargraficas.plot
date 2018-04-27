clear
reset
unset key

# Make the x axis labels easier to read.
# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border

set xtics rotate
set border 3
set title "ACCESOS A LA SECCION CRITICA"
set xlabel 'instante de tiempo T'
set ylabel 'sección critica'
set autoscale
set key bottom right
unset ytics

plot "pagos.dat" title "pagos" with fillsteps fs solid 0.3 noborder lt rgb "green" , \
"anulaciones.dat" title "anulaciones" with fillsteps fs solid 0.3 noborder lt rgb "red" , \
"prereservas.dat" title "prereservas" with fillsteps fs solid 0.3 noborder lt rgb "blue"  , \



