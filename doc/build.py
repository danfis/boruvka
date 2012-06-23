#!/usr/bin/env python2

import sys
import re
import string
from docutils import nodes
from sphinx.application import Sphinx
from sphinx.writers.html import HTMLTranslator


original_visit_Text = HTMLTranslator.visit_Text

def visit_Text(self, node):
    text = node.astext()

    if text == '_bor_inline':
        encoded = self.encode(text)
        self.body.append('<span class="func_inline">' + encoded + '</span>')
    else:
        original_visit_Text(self, node)


if sys.argv[1] == 'html':
    s = Sphinx(srcdir = '.',
               confdir = '.',
               outdir = 'html',
               doctreedir = 'doctrees',
               buildername = 'html')


s.add_node(nodes.Text, html=(visit_Text, None))
s.add_stylesheet('boruvka.css')
s.build()
