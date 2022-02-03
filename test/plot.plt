set term pngcairo dashed size 1024,768
set output 'plot.png'
set style fill solid
set xrange [0:45]
set xtics 5
set title 'Planificacion'
set ytics('a.txt' 1, 'b.txt' 2, 'c.txt' 3, 'd.txt' 4, 'e.txt' 5, 'f.txt' 6)
unset key
set style line 1 lt 1 lw 2 lc rgb '#00ff00'
set style line 2 lt 1 lw 2 lc rgb '#ff0000'
set style line 3 lt 1 lw 1 lc rgb '#202020'
set style arrow 1 heads size screen 0.008,90 ls 1
set style arrow 2 heads size screen 0.008,90 ls 2
set style arrow 3 heads size screen 0.008,100 ls 3
set yrange [0:8]
set arrow 1 from 0,1 to 5,1 as 1
set arrow 2 from 5,1 to 9,1 as 2
set arrow 3 from 9,1 to 11,1 as 1
set arrow 4 from 0,2 to 15,2 as 2
set arrow 5 from 15,2 to 18,2 as 1
set arrow 6 from 0,3 to 22,3 as 2
set arrow 7 from 22,3 to 27,3 as 1
set arrow 8 from 27,3 to 31,3 as 2
set arrow 9 from 31,3 to 36,3 as 1
set arrow 10 from 36,3 to 37,3 as 2
set arrow 11 from 37,3 to 40,3 as 1
set arrow 12 from 0,4 to 5,4 as 2
set arrow 13 from 5,4 to 9,4 as 1
set arrow 14 from 9,4 to 27,4 as 2
set arrow 15 from 27,4 to 31,4 as 1
set arrow 16 from 0,5 to 11,5 as 2
set arrow 17 from 11,5 to 15,5 as 1
set arrow 18 from 15,5 to 36,5 as 2
set arrow 19 from 36,5 to 37,5 as 1
set arrow 20 from 2,6 to 18,6 as 2
set arrow 21 from 18,6 to 22,6 as 1
plot NaN
