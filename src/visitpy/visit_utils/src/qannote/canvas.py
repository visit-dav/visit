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
import os
import platform
from . import xinit


try:
    from PySide2.QtCore import *
    from PySide2.QtGui import *
    from PySide2.QtWidgets import *
except:
    QWidget = object


from visit_utils.common import require_pyside
from visit_utils.qannote.items import Rect

class CanvasWidget(QWidget):
    @require_pyside
    def __init__(self,items,size,view,parent=None):
        super(CanvasWidget,self).__init__(parent)
        self.setBackgroundRole(QPalette.Base)
        self.resize(QSize(size[0],size[1]))
        self.items = items
        self.size  = size
        self.view  = view
    @require_pyside
    def paintEvent(self,pe):
        img = QImage(self.size[0],self.size[1],QImage.Format_ARGB32)
        QWidget.paintEvent(self,pe)
        pt = QPainter(img)
        if not self.view is None:
            pt.setWindow(self.view[0],self.view[1],self.view[2],self.view[3])
        else:
            self.view = [0,0,self.size[0],self.size[1]]
        pt.setRenderHint(QPainter.Antialiasing)
        pt.setCompositionMode(QPainter.CompositionMode_Clear)
        pt.fillRect(self.view[0],self.view[1],self.view[2],self.view[3],QBrush(QColor(0,0,0,0)))
        pt.setCompositionMode(QPainter.CompositionMode_SourceOver)
        pt.fillRect(self.view[0],self.view[1],self.view[2],self.view[3],QBrush(QColor(0,0,0,0)))
        for itm in self.items:
            itm.render(pt)
        del pt
        wpainter = QPainter(self)
        wpainter.setRenderHint(QPainter.Antialiasing)
        wpainter.setCompositionMode(QPainter.CompositionMode_Clear)
        wpainter.drawImage(0,0,img)
        wpainter.setCompositionMode(QPainter.CompositionMode_SourceOver)
        wpainter.drawImage(0,0,img)

class Canvas(object):
    @classmethod
    def render(cls,items,size,ofname,view=None):
        cls.init_qapp()
        w,h = size
        canvas = CanvasWidget(items,size,view)
        img = QImage(size[0],size[1],QImage.Format_ARGB32)
        canvas.render(img)
        img.save(ofname)
    @classmethod
    def init_qapp(cls):
        # make sure we have a QApplication instance
        if QApplication.instance() is None:
            # check if we need a fake x-server
            # (applies to some linux batch nodes)
            if platform.system() == "Linux":
                xinit.launch_x()
            QApplication(sys.argv)
