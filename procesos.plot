clear
reset
unset key

# DESCOMENTAR PARA GRAFICAS DE MUCHOS PUNTOS !
# reset
#  set term png
#  set output "data.png"

# Make the x axis labels easier to read.
# Select histogram data
set style data histogram
# Give the bars a plain fill pattern, and draw a solid line around them.
set style fill solid border

unset ytics
unset xtics

set title "ACCESOS A LA SECCION CRITICA"
set xlabel 'instante de tiempo T'
set ylabel 'sección critica'
set autoscale
set key outside
unset ytics

plot "pagos.dat" title "pagos" with fillsteps fs solid 0.3   , \
"anulaciones.dat" title "anulaciones" with fillsteps fs solid 0.3   , \
"prereservas.dat" title "prereservas" with fillsteps fs solid 0.3   , \
"consultores.dat" title "consultores" with fillsteps fs solid 0.3   , \
