# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: qplot/scene.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 description:
     Qt based offscreen Curve Rendering lib.
"""

import sys
import time
import math

from visit_utils.property_tree import PropertyTree
from visit_utils import ult 


from visit_utils.property_tree import PropertyTree
from visit_utils import ult 

from visit_utils.qannote import *

from .plots import *

class PlotGrid(object):
    def __init__(self,params,scene):
        self.scene = scene
        self.params = PropertyTree(init={"line_width":0,
                                         "x_bins":10,
                                         "y_bins":10})
        self.params.update(params)
    def render(self,painter):
        self.scene.set_scene_viewport(painter)
        fg = self.scene.params.fg_color
        x_bins = self.params.x_bins
        y_bins = self.params.y_bins
        pen = QPen(QColor(fg[0],fg[1],fg[2],fg[3]),
                   self.params.line_width,
                   Qt.SolidLine,Qt.SquareCap,Qt.MiterJoin)
        pen.setCosmetic(True)
        painter.setPen(pen)
        painter.setBrush(Qt.NoBrush)
        dx = 100.0 / (x_bins-1)
        dy = 100.0 / (y_bins-1)
        cx,cy = 0.0,0.0
        for i in range(x_bins):
            painter.drawLine(QPointF(cx,0.0),QPointF(cx,100.0))
            cx+=dx
        for i in range(y_bins):
            painter.drawLine(QPointF(0.0,cy),QPointF(100.0,cy))
            cy+=dy

class PlotAxes(object):
    def __init__(self,params,scene):
        self.scene = scene
        self.params = PropertyTree(init={"line_width":2,
                                         "full_box":True,
                                         "x_ticks":5,
                                         "y_ticks":5,
                                         "tick_length":1.0,
                                         "tick_width":2.0,
                                         "show_ticks":True})
        self.params.update(params)
    def render(self,painter):
        self.scene.set_scene_viewport(painter)
        fg = self.scene.params.fg_color
        x_ticks = self.params.x_ticks
        y_ticks  = self.params.y_ticks
        tick_width = self.params.tick_width
        tick_len = self.params.tick_length
        fgcolor = QColor(fg[0],fg[1],fg[2],fg[3])
        pen = QPen(fgcolor,
                   self.params.line_width,
                   Qt.SolidLine,Qt.SquareCap,Qt.MiterJoin)
        pen.setCosmetic(True)
        painter.setPen(pen)
        painter.setBrush(Qt.NoBrush)
        outline_path = QPainterPath()
        outline_path.moveTo(0,100)
        outline_path.lineTo(100,100)
        outline_path.moveTo(0,0)
        outline_path.lineTo(0,100)
        if self.params.full_box:
            outline_path.moveTo(0,0)
            outline_path.lineTo(100,0)
            outline_path.moveTo(100,0)
            outline_path.lineTo(100,100)
            painter.drawPath(outline_path)
        if self.params.show_ticks:
            dx = 100.0/(x_ticks-1)
            dy = 100.0/(y_ticks-1)
            cx,cy = 0.0,0.0
            pen = QPen(fgcolor,
                       tick_width,
                       Qt.SolidLine,Qt.SquareCap,Qt.MiterJoin)
            pen.setCosmetic(True)
            painter.setPen(pen)
            painter.setBrush(Qt.NoBrush)
            ar = self.scene.aspect_ratio()
            xtlen = tick_len
            ytlen = tick_len
            xtstart = 100
            ytstart = 0
            if ar >1:
                ytlen = ytlen / ar
            else:
                xtlen = xtlen / ar

            for i in range(x_ticks):
                painter.drawLine(QPointF(cx,xtstart+xtlen),
                                 QPointF(cx,xtstart))
                cx+=dx
            for i in range(y_ticks):
                painter.drawLine(QPointF(ytstart,cy),
                                 QPointF(ytstart-ytlen,cy))
                cy+=dy

class PlotLabels(object):
    def __init__(self,params,scene):
        self.scene = scene
        self.params = PropertyTree(init={"x_labels":5,
                                         "y_labels":5,
                                         "x_labels_offset":9.0,
                                         "y_labels_offset":1.0,
                                         "x_title":"",
                                         "y_title":"",
                                         "x_title_offset":12.0,
                                         "y_title_offset":9.0,
                                         "x_format":"%2.1f",
                                         "y_format":"%2.1f",
                                         "labels_font/name":"Times New",
                                         "labels_font/size":11,
                                         "labels_font/bold":False,
                                         "titles_font/name":"Times New",
                                         "titles_font/size":12,
                                         "titles_font/bold":False,
                                         "log_scale_y":False})
        self.params.update(params)
    def render(self,painter):
        self.scene.set_regular_viewport(painter)
        orig_font = painter.font()
        font = QFont(self.params.labels_font.name)
        font.setPointSize(self.params.labels_font.size)
        font.setBold(self.params.labels_font.bold)
        painter.setFont(font)
        view = [float (v) for v in self.scene.params.view]
        vwidth, vheight = [float(v) for v in  self.scene.params.size]
        x_labels = self.params.x_labels
        y_labels = self.params.y_labels
        log_scale_y = self.params.log_scale_y
        x_labels_offset = self.params.x_labels_offset
        y_labels_offset = self.params.y_labels_offset
        fstr_x = self.params.x_format
        fstr_y = self.params.y_format
        margins = self.scene.margins()
        tw = float(100 + int(margins[0] + margins[1]))
        th = float(100 + int(margins[2] + margins[3]))
        xmin,xmax = view[0],view[1]
        ymin,ymax = view[2],view[3]
        xdiff,ydiff = xmax - xmin, ymax - ymin
        vdx = xdiff / float(x_labels-1)
        vdy = ydiff / float(y_labels-1)
        vx ,vy = xmin, ymin
        vyl = 0.0
        if log_scale_y:
            vdy = (log10(ymax) - log10(ymin)) / float(y_labels-1)
            vyl = log10(ymin)
        fmtx = painter.fontMetrics()
        cx = margins[0]/tw *vwidth
        fixed_y = (th - (margins[2] - x_labels_offset))/ th * vheight
        cy = (1.0 - margins[2]/th) * vheight
        align_x = (margins[0] - y_labels_offset)/ tw * vwidth
        dx = (100.0 / tw * vwidth)/float(x_labels-1)
        dy = (100.0 / th * vheight)/float(y_labels-1)
        lbl = ""
        fh_offset = fmtx.height() / 4.0
        for i in range(x_labels):
            lbl = fstr_x % vx
            fw_offset = fmtx.width(lbl) / 2.0
            painter.drawText(QPointF(cx-fw_offset,fixed_y+fh_offset*4.0),lbl)
            cx+=dx
            vx+=vdx
        for i in range(y_labels):
            if log_scale_y:
                vy = math.pow(10.0,vyl)
            lbl = fstr_y % vy
            w_offset = fmtx.width(lbl)
            corr = align_x - w_offset
            painter.drawText(QPointF(corr,cy+fh_offset),lbl)
            cy=cy-dy
            if log_scale_y:
               vyl+=vdy
            else:
               vy +=vdy
        font = QFont(self.params.titles_font.name)
        font.setPointSize(self.params.titles_font.size)
        font.setBold(self.params.titles_font.bold)
        painter.setFont(font)
        x_title        = process_encoded_text(self.params.x_title)
        x_title_offset = self.params.x_title_offset
        if x_title != "":
            w_offset = fmtx.width(x_title)/2.0
            xtloc = QPointF(.5 * vwidth - w_offset,
                            (th - (margins[2] - x_title_offset))/th * vheight)
            painter.drawText(xtloc,x_title)
        y_title        = process_encoded_text(self.params.y_title)
        y_title_offset = self.params.y_title_offset
        if y_title != "":
            h_offset = fmtx.width(y_title)/2.0
            painter.save()
            painter.rotate(-90)
            ytloc = QPointF(-.5 * vheight - h_offset,
                            (margins[0] -y_title_offset)/tw * vwidth)
            painter.drawText(ytloc,y_title)
            painter.restore()
        painter.setFont(orig_font)

class CurveScene(object):
    def __init__(self,params):
        self.params = PropertyTree(init={"size": (400,250),
                                         "view":(0.0,0.0,0.0,0.0),
                                         "left_margin":15,
                                         "right_margin":10,
                                         "top_margin":10,
                                         "bottom_margin":15,
                                         "use_antialiasing":True,
                                         "log_scale_y":False,
                                         "fg_color":(255,255,255,255),
                                         "bg_color":(0,0,0,255)})
        self.params.update(params)
        self.params.lock()
        self.items = []
        self.active_view = "scene"
        self.__setup()
    def __setup(self):
        self.__add_bg()
        if self.params.has_property("grid"):
            self.items.append(PlotGrid(self.params.grid,self))
        if self.params.has_property("axes"):
            self.items.append(PlotAxes(self.params.axes,self))
        for p in self.params.plots:
            self.items.append(Plots.create(self,p))
        if self.params.has_property("labels"):
            self.items.append(PlotLabels(self.params.labels,self))
        for i in self.params.annotations:
            self.items.append(Annotations.create(i))
    def __add_bg(self):
        mgns = self.margins()
        w = 100 + int(mgns[0] + mgns[1])
        h = 100 + int(mgns[2] + mgns[3])
        bg = Rect({"x":-mgns[0],"y":-mgns[3],
                  "width":w,"height":h,"color":self.params.bg_color})
        self.items.append(bg)
    def __add_plot(self,params):
        pass
    def __add_annotation(self,params):
        pass
    def aspect_ratio(self):
        return float(self.params.size[1])/float(self.params.size[0])
    def margins(self):
        return [self.params.left_margin,self.params.right_margin,
                self.params.bottom_margin, self.params.top_margin]
    def render(self,ofname):
        mgns =self.margins()
        w = 100 + int(mgns[0] + mgns[1])
        h = 100 + int(mgns[2] + mgns[3])
        view = (-mgns[0],-mgns[3],w,h)
        Canvas.render(self.items,self.params.size,ofname,view)
    def set_scene_viewport(self,painter):
        if not self.active_view == "regular":
            if self.active_view == "curve":
                painter.restore()
            mgns =self.margins()
            w = 100 + int(mgns[0] + mgns[1])
            h = 100 + int(mgns[2] + mgns[3])
            view = (-mgns[0],-mgns[3],w,h)
            painter.setWindow(-mgns[0],-mgns[3],w,h)
            self.active_view = "scene"
    def set_curve_viewport(self,painter):
        if not self.active_view == "curve":
            mgns =self.margins()
            w = 100 + int(mgns[0] + mgns[1])
            h = 100 + int(mgns[2] + mgns[3])
            view = (-mgns[0],-mgns[3],w,h)
            painter.setWindow(-mgns[0],-mgns[3],w,h);
            xmin, xmax,ymax,ymin= self.params.view
            if self.params.log_scale_y:
                ymin = log10(ymin);
                ymax = log10(ymax);
            xdiff = xmax - xmin;
            ydiff = ymax - ymin;
            painter.save()
            painter.scale(100.0/xdiff,100.0/ydiff)
            painter.translate(-xmin,-ymin)
            self.active_view="curve"
    def set_regular_viewport(self,painter):
        if not self.active_view == "regular":
            if self.active_view == "curve":
                painter.restore()
            sz = self.params.size
            painter.setWindow(0,0,sz[0],sz[1])
            self.active_view = "regular"
