#############################################################################
#   The Parser class is the Python Source Parser
#   taken from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/52298
#############################################################################
from __future__ import print_function
# Imports
import sys
import cgi
import string

if (sys.version_info > (3, 0)):
    import io
else:
    import cStringIO as io

import keyword, token, tokenize

from os.path import join as pjoin

_KEYWORD = token.NT_OFFSET + 1
_TEXT    = token.NT_OFFSET + 2

_colors = {
    token.NUMBER:       '#0080C0',
    token.OP:           '#4000C0',
    token.STRING:       '#008000',
    tokenize.COMMENT:   '#a02030',
    token.NAME:         '#000000',
    token.ERRORTOKEN:   '#FF8080',
    _KEYWORD:           '#C00000',
    _TEXT:              '#000000',
}

class Parser:
    """ Send colored python source.
    """

    def __init__(self, raw, out = sys.stdout):
        """ Store the source text.
        """
        self.raw = string.strip(string.expandtabs(raw))
        self.out = out

    def format(self, formatter, form, title):
        """ Parse and send the colored source.
        """
        # store line offsets in self.lines
        self.lines = [0, 0]
        pos = 0
        while 1:
            pos = string.find(self.raw, '\n', pos) + 1
            if not pos: break
            self.lines.append(pos)
        self.lines.append(len(self.raw))

        # parse the source and write it
        self.pos = 0
        text = io.StringIO(self.raw)
        self.out.write('<html><body bgcolor="#e0e0e0"><head><title>%s</title></head><pre><font face="Lucida,Courier New">'%title)
        try:
            tokenize.tokenize(text.readline, self)
        except tokenize.TokenError as ex:
            msg = ex[0]
            line = ex[1][0]
            self.out.write("<h3>ERROR: %s</h3>%s\n" % (
                msg, self.raw[self.lines[line]:]))
        self.out.write('</font></pre></body></html>')

    def __call__(self, toktype, toktext, tok_start, tok_end, line):
        """ Token handler.
        """
        (srow,scol) = tok_start
        (erow,ecol) = tok_end
        if 0:
            print("type", toktype, token.tok_name[toktype], "text", toktext, end=' ')
            print("start", srow,scol, "end", erow,ecol, "<br>")

        # calculate new positions
        oldpos = self.pos
        newpos = self.lines[srow] + scol
        self.pos = newpos + len(toktext)

        # handle newlines
        if toktype in [token.NEWLINE, tokenize.NL]:
            self.out.write('\n')
            return

        # send the original whitespace, if needed
        if newpos > oldpos:
            self.out.write(self.raw[oldpos:newpos])

        # skip indenting tokens
        if toktype in [token.INDENT, token.DEDENT]:
            self.pos = newpos
            return

        # map token type to a color group
        if token.LPAR <= toktype and toktype <= token.OP:
            toktype = token.OP
        elif toktype == token.NAME and keyword.iskeyword(toktext):
            toktype = _KEYWORD
        color = _colors.get(toktype, _colors[_TEXT])

        style = ''
        if toktype == token.ERRORTOKEN:
            style = ' style="border: solid 1.5pt #FF0000;"'

        # send text
        self.out.write('<font color="%s"%s>' % (color, style))
        self.out.write(cgi.escape(toktext))
        self.out.write('</font>')

def ColorizePython(test_script ,result_dir,category, filename, filebase):
    source = open(test_script).read()
    Parser(source, open(pjoin(result_dir,"html",'%s_%s_py.html' % (category, filebase)), 'wt')).format(None, None, "%s/%s"%(category,filename))
