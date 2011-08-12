#!/bin/bash

X=100
Y="2 4 6 8 10 12 14 16 18 20"
Y="$Y 22 24 26 28 30 32 34 36 38 40"
Y="$Y 42 44 46 48 50 52 54 56 58 60"
Y="$Y 62 64 66 68 70 72 74"
#76 78 80"
#Y="$Y 82 84 86 88 90 92 94 96 98 100"
THREADS="1 4 8 12"
plotcmd=""
plotfile=".tmp.test-cd-ode.plot"

for threads in $THREADS; do
    file="$plotfile.$threads"
    rm -f "$file"

    if [ "$plotcmd" != "" ]; then
        plotcmd="$plotcmd, \"$file\" w l"
    else
        plotcmd="\"$file\" w l"
    fi

    for y in $Y; do
        num=$(($y * $X))
        t=$(./test-cd-ode test $X $y $threads 2>&1 | grep -o '[0-9]\+')
#t=$((t / 1000))
        echo "$num $t" >> "$file"
        echo "$threads: $num $t us"
    done;
done;

plotcmd="plot $plotcmd"
echo "$plotcmd" | gnuplot -p
