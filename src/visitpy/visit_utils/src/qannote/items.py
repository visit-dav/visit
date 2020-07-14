# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: canvas.py
 author: Cyrus Harrison (cyrush@llnl.gov)
 description:
      Simple PySide / Qt based Text & Image annotation overlay lib.

"""


import sys
import time
from math import *

try:
    from PySide2.QtCore import *
    from PySide2.QtGui import *
except:
    pass

from visit_utils.common import require_pyside
from visit_utils.property_tree import PropertyTree


def py2to3_compat_unichr(c):
    if (sys.version_info > (3, 0)):
        return chr(c)
    else:
        return unichr(c)

@require_pyside
def process_encoded_text(val):
    rexp = QRegExp("(\\\\0x[0-9a-fA-F]{4})")
    res  = ""
    curr = 0
    prev = 0
    curr  = rexp.indexIn(val,curr)
    while curr != -1:
        res += val[prev:curr]
        cval = int(rexp.cap(1)[1:],base=16)
        res +=  py2to3_compat_unichr(cval)
        prev = curr + rexp.matchedLength()
        curr = prev
        curr = rexp.indexIn(val,curr)
    res += val[prev:]
    return res

class Annotations(object):
    @classmethod
    def create(cls,params):
        itype = params.type.lower()
        if itype == "text":
            return Text(params)
        elif itype == "image":
            return Image(params)
        elif itype == "line":
            return Line(params)
        elif itype == "arrow":
            return Arrow(params)
        elif itype == "rect":
            return Rect(params)
        elif itype == "circle":
            return Circle(params)
        elif itype == "ellipse":
            return Ellipse(params)
        elif itype == "textbox":
            return TextBox(params)
        elif itype == "multiprogressbar":
            return MultiProgressBar(params)
        else:
            return None


class CanvasItem(object):
    def __init__(self,params,defaults=None):
        self.params = PropertyTree()
        if not defaults is None:
            self.params.update(defaults)
        if isinstance(params,PropertyTree):
            self.params.update(params)
        elif isinstance(params,dict):
            self.params.update(PropertyTree(init=params))
        self.params.lock()

class CanvasItemSet(CanvasItem):
    def __init__(self,params,defs):
        CanvasItem.__init__(self,params,defs)
        self.params.lock()
    @require_pyside
    def render(self,painter):
        for itm in self.items:
            itm.render(painter)


class Text(CanvasItem):
    def __init__(self,params):
        # setup defaults ...
        defs = PropertyTree()
        defs.font.size = 12
        defs.font.name = "Times New Roman"
        defs.font.bold = False
        defs.font.ital = False
        defs.color  = (255,255,255,255)
        defs.vert_align = "left"
        defs.horz_align = "top"
        defs.orient = "horz"
        defs.wrap = -1
        CanvasItem.__init__(self,params,defs)
        self.params.lock()
    @require_pyside
    def render(self,painter):
        txt = process_encoded_text(self.params.text)
        x   = self.params.x
        y   = self.params.y
        vza = self.params.vert_align
        hza = self.params.horz_align
        c = self.params.color
        color = QColor(c[0],c[1],c[2],c[3])
        font = QFont(self.params.font.name)
        font.setPointSize(self.params.font.size)
        if self.params.font.bold:
            font.setBold(True)
        painter.setFont(font)
        pen = QPen(color)
        painter.setPen(pen)
        painter.setBrush(Qt.NoBrush)
        fmtx = painter.fontMetrics()
        tlines = self.__wrap_text(txt,fmtx,self.params.wrap)
        if self.params.orient == "vert":
            painter.save()
            painter.rotate(-90)
        # calc largest width
        cw = max([fmtx.width(t) for t in tlines])
        # calc total height
        th =  fmtx.height() * len(tlines) - fmtx.descent()
        cy = y + fmtx.height() - fmtx.descent()
        if vza == "center":
            cy -= .5 * th
        elif vza == "bottom":
            cy -= th
        for t in tlines:
            rx = x
            ry = cy
            if hza == "center":
                rx -= .5 * fmtx.width(t)
            elif hza == "right":
                rx -= fmtx.width(t)
            painter.drawText(QPointF(rx,ry),t)
            cy += fmtx.height()
        if self.params.orient  == "vert":
            painter.restore()
    def __wrap_text(self,txt,fmtx,length):
        res = []
        lines = txt.split("\n")
        if length < 0:
            return lines
        for l in lines:
            toks = l.split(" ")
            ctxt = toks[0]
            tlen = fmtx.width(ctxt)
            for t in toks[1:]:
                clen =  fmtx.width(" " + t)
                if  clen + tlen > length:
                    res.append(ctxt)
                    tlen = clen
                    ctxt = t
                else:
                    tlen += clen
                    ctxt += " " + t
            if clen > 0:
                res.append(ctxt)
        return res


class Image(CanvasItem):
    def __init__(self,params):
        # setup defaults ...
        defs = PropertyTree()
        defs.x = 0.0
        defs.y = 0.0
        defs.vert_align = "left"
        defs.horz_align = "bottom"
        CanvasItem.__init__(self,params,defs)
    @require_pyside
    def render(self,painter):
        ifile = self.params.image
        x = self.params.x
        y = self.params.y
        vza = self.params.vert_align
        hza = self.params.horz_align
        img = QImage(ifile)
        iw = float(img.size().width())
        ih = float(img.size().height())
        if hza == "center":
            x -=.5 * iw
        elif hza == "right":
            x -= iw
        if vza == "center":
            y+= .5 * ih
        elif vza == "top":
            y+= ih
        painter.drawImage(x,y,img)
    def size(self):
        img = QImage(self.params.image)
        return img.size().width(),img.size().height()

class Line(CanvasItem):
    def __init__(self,params):
        # setup defaults ...
        defs = PropertyTree()
        defs.width = 5
        defs.color = (255,255,255,255)
        CanvasItem.__init__(self,params,defs)
    @require_pyside
    def render(self,painter):
        x0 = self.params.x0
        y0 = self.params.y0
        x1 = self.params.x1
        y1 = self.params.y1
        w  = self.params.width
        c  = self.params.color
        color = QColor(c[0],c[1],c[2],c[3])
        pen = QPen(color,w)
        pen.setJoinStyle(Qt.RoundJoin)
        pen.setCapStyle(Qt.RoundCap)
        painter.setPen(pen)
        painter.setBrush(Qt.NoBrush)
        painter.drawLine(x0,y0,x1,y1)

class Arrow(CanvasItem):
    def __init__(self,params):
        # setup defaults ...
        defs = PropertyTree()
        defs.width = 5
        defs.color = (255,255,255,255)
        defs.tip_angle = 30
        CanvasItem.__init__(self,params,defs)
    @require_pyside
    def render(self,painter):
        x0 = self.params.x0
        y0 = self.params.y0
        x1 = self.params.x1
        y1 = self.params.y1
        w = self.params.width
        c = self.params.color
        dx = x1 - x0
        dy = y1 - y0
        llen = sqrt(dx*dx+dy*dy)
        lang = atan2(dy,dx)
        tip_angle = self.params.tip_angle
        tip_len   = self.params.tip_len
        ta_half = .5 * tip_angle
        color = QColor(c[0],c[1],c[2],c[3])
        painter.setBrush(Qt.NoBrush)
        pen = QPen(color,w)
        pen.setJoinStyle(Qt.RoundJoin)
        pen.setCapStyle(Qt.RoundCap)
        painter.setPen(pen)
        painter.save()
        painter.translate(x0,y0)
        # this expects degrees NOT radians ...
        painter.rotate(lang * 180.0 / pi)
        # draw main line
        painter.drawLine(0,0,llen,0)
        # draw tip prongs
        painter.drawLine(llen,0,llen - tip_len, tip_len*tan(ta_half))
        painter.drawLine(llen,0,llen - tip_len,-tip_len*tan(ta_half))
        painter.restore()

class Rect(CanvasItem):
    def __init__(self,params):
        defs = PropertyTree()
        defs.color = (255,255,255,255)
        defs.width = 1
        defs.outline = False
        CanvasItem.__init__(self,params,defs)
    @require_pyside
    def render(self,painter):
        x = self.params.x
        y = self.params.y
        w = self.params.width
        h = self.params.height
        c = self.params.color
        width = self.params.width
        outline = self.params.outline
        color = QColor(c[0],c[1],c[2],c[3])
        if outline:
            pen = QPen(QBrush(color),width)
            painter.setPen(pen)
            painter.setBrush(Qt.NoBrush)
        else:
            painter.setPen(Qt.NoPen)
            painter.setBrush(QBrush(color))
        painter.drawRect(x,y,w,h)

class Circle(CanvasItem):
    def __init__(self,params):
        defs = PropertyTree()
        defs.outline = False
        defs.width = 1
        CanvasItem.__init__(self,params,defs)
    @require_pyside
    def render(self,painter):
        r = self.params.radius
        x = self.params.x - r/2.0
        y = self.params.y - r/2.0
        c = self.params.color
        outline = self.params.outline
        width = self.params.width
        color = QColor(c[0],c[1],c[2],c[3])
        if outline:
            pen = QPen(QBrush(color),width)
            painter.setPen(pen)
            painter.setBrush(Qt.NoBrush)
        else:
            painter.setPen(Qt.NoPen)
            painter.setBrush(QBrush(color))
        painter.drawEllipse(x,y,r,r)
        painter.drawPoint(x+r/2.0,y+r/2.0)


class Ellipse(CanvasItem):
    def __init__(self,params):
        defs = PropertyTree()
        defs.outline = False
        defs.width = 1
        CanvasItem.__init__(self,params,defs)
    @require_pyside
    def render(self,painter):
        w = self.params.width
        h = self.params.height
        x = self.params.x - w/2.0
        y = self.params.y - h/2.0
        c = self.params.color
        outline = self.params.outline
        width = self.params.width
        color = QColor(c[0],c[1],c[2],c[3])
        if outline:
            pen = QPen(QBrush(color),width)
            painter.setPen(pen)
            painter.setBrush(Qt.NoBrush)
        else:
            painter.setPen(Qt.NoPen)
            painter.setBrush(QBrush(color))
        painter.drawEllipse(x,y,w,h)


class TextBox(CanvasItemSet):
    @require_pyside
    def __init__(self,params):
        defs = PropertyTree()
        defs.fg_color = (255,255,255,255)
        defs.padding = 2
        defs.text_vert_align = "top"
        defs.text_horz_align = "left"
        defs.bg_color     = (0,0,0,255)
        defs.font.name   = "Times New Roman"
        defs.font.bold   = True
        defs.font.size   = 15
        CanvasItemSet.__init__(self,params,defs)
        self.items = self.generate()
    @require_pyside
    def generate(self):
        items = []
        x = self.params.x
        y = self.params.y
        w = self.params.width
        h = self.params.height
        txt = self.params.text
        pad = self.params.padding
        items.append(Rect({"x":x-pad,"y":y-pad,
                           "width":w+pad*2,"height":h+pad*2,
                           "color":self.params.fg_color}))
        items.append(Rect({"x":x,"y":y,
                           "width":w,"height":h,
                           "color":self.params.bg_color}))
        items.append(Text( {"text": txt,
                             "x": x+pad,
                             "y": y+pad,
                             "wrap":w-pad*2,
                             "color": self.params.fg_color,
                             "vert_align": self.params.text_vert_align,
                             "horz_align": self.params.text_horz_align,
                             "font/name": self.params.font.name,
                             "font/bold": self.params.font.bold,
                             "font/size": self.params.font.size}))
        return items


class MultiProgressBar(CanvasItemSet):
    def __init__(self,params):
        defs = PropertyTree()
        defs.force_labels = False
        defs.padding    = 2
        defs.line_width = 2
        defs.bg_color = (0,0,0,255)
        defs.font.name = "Times New Roman"
        defs.font.bold = True
        defs.font.size = 15
        CanvasItemSet.__init__(self,params,defs)
        self.items = self.generate()
    @require_pyside
    def generate(self):
        items = []
        x  = self.params.x
        y  = self.params.y
        w  = self.params.width
        h  = self.params.height
        lw = self.params.line_width
        pad        = self.params.padding
        bgcolor    = self.params.bg_color
        seg_ranges = self.params.segment.ranges
        seg_lbls   = self.params.segment.labels
        seg_colors = self.params.segment.colors
        val = self.params.position
        items.append(Rect({"x":x-pad,"y":y-pad,
                           "width":w+pad*2,"height":h+pad*2,
                           "color":bgcolor}))
        cx = x
        sx = x
        # convert percentage val to length in pixels
        val = w * val + x;
        for i in range(len(seg_ranges)):
            sr = seg_ranges[i] * w
            cx += sr/2.0
            # check if this segment is full covered
            if val >= sx + sr:
                items.append( Rect({"x":sx,"y":y,
                                    "width":sr,"height":h,
                                    "color":seg_colors[i]}))
            # check if this segment is partially covered
            elif val >= sx:
                items.append( Rect({"x":sx,"y":y,
                                    "width":val-sx,"height":h,
                                    "color":seg_colors[i]}))

                items.append( Line({"x0": val,"y0":y + h /2.0,
                                    "x1":sx+sr,"y1":y + h /2.0,
                                    "width":lw,
                                    "color":seg_colors[i]}))
            # check if this segment is partially uncovered
            else:
                items.append( Line({"x0":sx,"y0":y + h /2.0,
                                    "x1":sx+sr,"y1":y + h /2.0,
                                    "width":2,
                                    "color":seg_colors[i]}))

                items.append( Line({"x0":sx,"y0":y,
                                    "x1":sx,"y1":y+h,
                                    "width":2,
                                    "color":seg_colors[i]}))
            # check if we should draw the labels for this segment
            if self.params.force_labels or (val >= sx + sr or val >=sx):
                items.append( Text({"text": seg_lbls[i],
                                    "x": cx,
                                    "y": y + h,
                                    "color": seg_colors[i],
                                    "vert_align":"top",
                                    "horz_align":"center",
                                    "font/name": self.params.font.name,
                                    "font/bold": self.params.font.bold,
                                    "font/size": self.params.font.size}))
            cx += sr/2.0
            sx += sr
        return items



