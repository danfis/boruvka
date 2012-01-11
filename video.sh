#!/bin/bash

for f in g-*[0-9]; do
    f2=h${f##g};
    echo $f $f2;
    svt-to-png --output-width 600 $f >$f.png
    svt-to-png --output-width 600 $f2 >$f2.png
    montage $f.png $f2.png -mode concatenate $f-$f2.png
done;

#rm -f video.avi
#mencoder mf://*.png -mf w=1100:h=760:fps=25:type=png -ovc lavc \
#         -lavcopts vcodec=mpeg4:mbd=2:trell:vbitrate=1000000 -oac copy \
#         -o video.avi
