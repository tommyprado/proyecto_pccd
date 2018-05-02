#
 reset
  set term png size 800,600
  set output "proceso.png"
  set term x11

set title "SC por proceso"


set key outside



set yrange [0:*]



set boxwidth 0.6 relative
set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
plot for [COL=1:4] 'scPorProceso.dat' using COL:xticlabels(4) title columnheader

#