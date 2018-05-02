#
set title "SC por proceso"


set key outside



set yrange [0:*]



set boxwidth 0.6 relative
set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
plot for [COL=1:5] 'scPorProcesoTiempoTotal.dat' using COL:xticlabels(5) title columnheader

#