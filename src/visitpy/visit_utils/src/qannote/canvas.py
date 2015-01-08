#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
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
import xinit


try:
    from PySide.QtCore import *
    from PySide.QtGui import *
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
