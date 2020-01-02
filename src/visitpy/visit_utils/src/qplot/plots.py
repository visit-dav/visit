# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: qplot/plots.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 description:
     Qt based offscreen Curve Rendering lib.
"""

import sys
import time
import math

from visit_utils.property_tree import PropertyTree
from visit_utils import ult 

from visit_utils.qannote import *

def log10(val):
    if val != 0.0:
        return math.log10(val)
    else:
        return 0.0

def lerp(s0,s1,xval,log_scale_y=False):
    x0,y0 = s0.x, s0.y
    x1,y1 = s1.x, s1.y
    if log_scale_y:
        y0 = log10(y0)
        y1 = log10(y1)
    res = (y1-y0) * (xval - x0) / (x1 - x0) + y0
    return xval,res

class Plots(object):
    @classmethod
    def create(cls,scene,params):
        itype = params.type.lower()
        cfname = params.curve.file
        cidx = params.curve.index
        curves =ult.Curve.load(cfname)
        curve = curves[cidx]
        if itype == "line":
            return LinePlot(curve,scene,params)
        elif itype == "area":
            return AreaPlot(curve,scene,params)
        elif itype == "tracer_dot":
            return TracerDotPlot(curve,scene,params)
        elif itype == "tracer_line":
            return TracerLinePlot(curve,scene,params)
        return None


class  CurvePlot(CanvasItem):
    def __init__(self,curve,scene,params,defaults=None):
        self.curve = curve
        self.scene = scene
        defs = PropertyTree()
        defs.log_scale_y = False
        defs.color = (255,255,255,255)
        if not defaults is None:
            defs.update(defaults)
        CanvasItem.__init__(self,params,defs)

class IntervalPlot(CurvePlot):
    def __init__(self,curve,scene,params,defaults=None):
        CurvePlot.__init__(self,curve,scene,params,defaults)
        nsamps = len(curve)
        self.start_x     = self.curve.samples[0].x
        self.start_index = 0
        self.stop_x      =  self.curve.samples[-1].x
        self.stop_index  = len(self.curve.samples) -1
        if self.params.has_property("start_x"):
            xval = self.params.start_x
            idx = curve.find_index(xval)
            if not idx is None:
                self.start_index = idx
                self.start_x     = xval
        elif self.params.has_property("start_index"):
            idx = self.params.start_index
            s = curve.get_sample(idx)
            if not s is None:
                self.start_index = idx
                self.start_x = s.x
        if self.params.has_property("stop_x"):
            xval = self.params.stop_x
            idx = curve.find_index(xval)
            if not idx is None:
                self.stop_index = idx
                self.stop_x     = xval
        elif self.params.has_property("stop_index"):
            idx = self.params.stop_index
            s = curve.get_sample(idx)
            if not s is None:
                self.stop_index = idx
                self.stop_x = s.x
    def interval_data(self):
        res = []
        nsamps = len(self.curve.samples)
        start = self.start_index
        stop  = self.stop_index
        start_x = self.start_x
        stop_x  = self.stop_x
        if self.curve.samples[start].x != start_x:
            # interp ...
            s0 = self.curve.samples[start]
            s1 = self.curve.samples[start+1]
            cx = start_x
            cx,cy = lerp(s0,s1,start_x,self.params.log_scale_y)
            res.append(ult.Sample(cx,cy))
        for i in range(start,stop+1):
            s = self.curve.samples[i]
            cx,cy = s.x, s.y
            if self.params.log_scale_y:
                cy = log10(cy)
            res.append(ult.Sample(cx,cy))
        # see if we need to lerp to hit final value
        if stop < nsamps-1 and self.curve.samples[stop].x != stop_x:
            s0 = self.curve.samples[stop]
            s1 = self.curve.samples[stop+1]
            cx,cy = lerp(s0,s1,stop_x,self.params.log_scale_y)
            res.append(ult.Sample(cx,cy))
        return res

class LinePlot(IntervalPlot):
    def __init__(self,curve,scene,params):
        defs = PropertyTree()
        defs.line_width = 3
        IntervalPlot.__init__(self,curve,scene,params,defs)
    def render(self,painter):
        self.scene.set_curve_viewport(painter)
        isamples = self.interval_data()
        nsamps = len(isamples)
        painter.setBrush(Qt.NoBrush)
        c     = self.params.color
        color = QColor(c[0],c[1],c[2],c[3])
        line_width = self.params.line_width
        pen   = QPen(color,line_width,Qt.SolidLine,Qt.RoundCap,Qt.RoundJoin)
        pen.setCosmetic(True)
        painter.setPen(pen)
        path = QPainterPath()
        path.moveTo(isamples[0].x,isamples[0].y)
        for i in range(1,nsamps):
            path.lineTo(isamples[i].x,isamples[i].y)
        painter.drawPath(path)

class AreaPlot(IntervalPlot):
    def __init__(self,curve,scene,params):
        defs = PropertyTree()
        defs.line_width = 3
        IntervalPlot.__init__(self,curve,scene,params,defs)
    def render(self,painter):
        self.scene.set_curve_viewport(painter)
        isamples = self.interval_data()
        nsamps = len(isamples)
        painter.setBrush(Qt.NoBrush)
        c     = self.params.color
        color = QColor(c[0],c[1],c[2],c[3])
        line_width = self.params.line_width
        pen_color = QColor(c[0],c[1],c[2],c[3])
        pen_color.setAlpha(255)
        pen = QPen(color,line_width,Qt.SolidLine)
        pen.setCosmetic(True)
        painter.setPen(pen)
        painter.setBrush(QBrush(color))
        extents  = self.curve.extents()
        xmin = isamples[0].x
        ymin = extents[2]
        if self.params.log_scale_y:
            ymin = log10(ymin)
        path = QPainterPath()
        path.moveTo(xmin,ymin)
        for i in range(0,nsamps):
            path.lineTo(isamples[i].x,isamples[i].y)
        # complete the circit
        path.lineTo(isamples[-1].x,ymin)
        path.lineTo(xmin,ymin)
        painter.drawPath(path)


class TracerDotPlot(CurvePlot):
    def __init__(self,curve,scene,params):
        defs = PropertyTree()
        defs.point_size = 15
        CurvePlot.__init__(self,curve,scene,params,defs)
        self.tracer_x = 0.0
        self.tracer_y = 0.0
        self.tracer_index = None
        if self.params.has_property("tracer_x"):
            xval = self.params.tracer_x
            idx = curve.find_index(xval)
            if not idx is None:
                self.tracer_index = idx
                if xval == curve.samples[idx].x:
                    self.tracer_x     = xval
                    self.tracer_y     = curve.samples[idx].y
                else:
                    s0 =  curve.samples[idx]
                    s1 =  curve.samples[idx+1]
                    self.tracer_x,self.tracer_y = lerp(s0,s1,xval,self.params.log_scale_y)
        elif self.params.has_property("tracer_index"):
            idx = self.params.tracer_index
            xval = curve.find_xvalue(idx)
            if not xval is None:
                self.tracer_index = idx
                self.tracer_x     = xval
                self.tracer_y     = curve.samples[idx].y
                if self.parmas.log_scale_y:
                    self.tracer_y = log10(self.tracer_y)
    def render(self,painter):
        self.scene.set_curve_viewport(painter)
        nsamps = len(self.curve.samples)
        if self.tracer_index is None or self.tracer_index > nsamps:
            return
        painter.setBrush(Qt.NoBrush)
        c = self.params.color
        color = QColor(c[0],c[1],c[2],c[3])
        pen = QPen(color,self.params.point_size,Qt.SolidLine,Qt.RoundCap,Qt.RoundJoin)
        pen.setCosmetic(True)
        painter.setPen(pen)
        tx,ty = self.tracer_x, self.tracer_y
        painter.drawPoint(QPointF(tx,ty))

class TracerLinePlot(CurvePlot):
    def __init__(self,curve,scene,params):
        defs = PropertyTree()
        defs.tracer_orientation = 1
        CurvePlot.__init__(self,curve,scene,params,defs)
        self.tracer_x = 0.0
        self.tracer_y = 0.0
        self.tracer_index = None
        if self.params.has_property("tracer_x"):
            xval = self.params.tracer_x
            idx = curve.find_index(xval)
            if not idx is None:
                if xval == curve.samples[idx].x:
                    self.tracer_x     = xval
                    self.tracer_y     = curve.samples[idx].y
                else:
                    s0 =  curve.samples[idx]
                    s1 =  curve.samples[idx+1]
                    self.tracer_x,self.tracer_y = lerp(s0,s1,xval,self.params.log_scale_y)
        elif self.params.has_property("tracer_index"):
            idx = self.params.tracer_index
            xval = curve.find_xvalue(idx)
            if not xval is None:
                self.tracer_index = idx
                self.tracer_x     = xval
                self.tracer_y     = curve.samples[idx].y
                if self.parmas.log_scale_y:
                    self.tracer_y = log10(self.tracer_y)
    def render(self,painter):
        self.scene.set_curve_viewport(painter)
        painter.setBrush(Qt.NoBrush)
        c = self.params.color
        color = QColor(c[0],c[1],c[2],c[3])
        pen = QPen(color,2,Qt.DashLine)
        pen.setCosmetic(True)
        painter.setPen(pen)
        painter.setBrush(QBrush(color))
        extents = self.curve.extents()
        xmin,xmax, ymin ,ymax = self.scene.params.view
        py = self.tracer_y
        if self.params.log_scale_y:
            ymin = log10(ymin)
            ymax = log10(ymax)
        path = QPainterPath()
        if self.params.tracer_orientation == 1:
            path.moveTo(self.tracer_x,ymin)
            path.lineTo(self.tracer_x,ymax)
        else:
            path.moveTo(xmin,py)
            path.lineTo(xmax,py)
        painter.drawPath(path)