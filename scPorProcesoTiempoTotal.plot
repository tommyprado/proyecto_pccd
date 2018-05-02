#

 reset
  set term png size 800,600
  set output "tiempo.png"
set title "SC por proceso"
set term x11


set key outside



set yrange [0:*]



set boxwidth 0.6 relative
set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
plot for [COL=1:5] 'scPorProcesoTiempoTotal.dat' using COL:xticlabels(5) title columnheader

#