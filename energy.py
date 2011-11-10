#!/usr/bin/env python2

import cairo
import sys
import os
import math
import colorsys

WIDTH, HEIGHT = 800, 800

surface = cairo.ImageSurface (cairo.FORMAT_ARGB32, WIDTH, HEIGHT)
ctx = cairo.Context (surface)

def draw(x, y, e):
    x += 5
    y += 5

    x = x * (800 / 10)
    y = y * (800 / 10)

    y = 800 - y;

    #e = e * 0.65
    #rgb = colorsys.hsv_to_rgb(e, 0.5, 0.5)
    #ctx.set_source_rgb(*rgb)
    ctx.set_source_rgb(e, 0., 0.)
    ctx.rectangle(x, y, 3, 3)
    ctx.fill()

ctx.set_source_rgb(1, 1, 1)
ctx.rectangle(0, 0, 800, 800)
ctx.fill()

fin = open(sys.argv[1], 'r')
for line in fin:
    p = [float(x) for x in line.split()]
    draw(p[0], p[1], p[2])
fin.close()

f = sys.argv[1] + '.png'
#f = 'gnnp.png'
surface.write_to_png(f) # Output to PNG
os.system('display {0}'.format(f))
