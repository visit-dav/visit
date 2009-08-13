import os
import sys
import shutil
import subprocess
import time
import urllib
from PyQt4 import QtCore,QtGui

# TODO take a command line argument to replace the "09Aug12" part.
davinci_serial="http://portal.nersc.gov/project/visit/09Aug13/davinci_serial"

class MW(QtGui.QMainWindow):
  def __init__(self, parent=None):
    QtGui.QMainWindow.__init__(self, parent)

def current_from_baseline(path):
  """Given a path to a baseline image, return the path to the current image."""
  splitpath = path.split('/', 1)
  return os.path.join("current/", splitpath[1])

def in_svn_repo():
  a = subprocess.Popen("svn stat", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  result = a.communicate()[0]
  if result.find('not a working copy') >= 0:
    return False
  return True

# TODO: take a command line argument to dictate whether we care about the
# vcs_diff case.  It's certainly cheaper than having a human run through the
# files, but it makes this function very slow.
def investigate_git(fileA, fileB):
  """Indicate whether or not an image is worth investigating more deeply."""
  # if the VCS says there the file is modified locally, say there aren't
  # differences.  This covers the case that the file has been rebaselined, but
  # only here -- davinci doesn't know yet!
  vcs_diff = subprocess.call(["git", "diff", "--quiet", fileA])
  if vcs_diff == 1:
    print fileA, " is locally modified, ignoring."
    return False

def investigate_svn(fileA, fileB):
  """Indicate whether or not an image is worth investigating more deeply."""
  a = subprocess.Popen("svn stat %s"%fileA, shell=True, stdout=subprocess.PIPE)
  diff = a.communicate()[0]
  if diff != '':
    print fileA, " is locally modified, ignoring."
    return False

  return subprocess.call(["cmp", "-s", fileA, fileB]) 

class Image(QtGui.QWidget):
  def __init__(self, path, parent=None):
    self._filename = path
    self._parent = parent
    self._display = QtGui.QLabel(self._parent)
    self._load()

  def _load(self):
    pixmap = QtGui.QPixmap(300,300)
    pixmap.load(self._filename)
    self._display.resize(pixmap.size())
    self._display.setPixmap(pixmap)

  def widget(self): return self._display

  def update(self, path):
    self._filename = path
    self._load()

class Layout(QtGui.QWidget):
  def __init__(self, parent=None):
    QtGui.QWidget.__init__(self, parent)
    self._mainwin = parent
    self._mainwin.statusBar().insertPermanentWidget(0,QtGui.QLabel())

    quit = QtGui.QPushButton('Quit', self)
    if parent is None: parent = self
    parent.connect(quit, QtCore.SIGNAL('clicked()'), QtGui.qApp,
                   QtCore.SLOT('quit()'))
    parent.connect(self, QtCore.SIGNAL('closeApp()'), self._die)
    self._init_signals()
    self._bugs = [] # list which keeps track of which images we think are bugs.

    # guess an initial size; we don't know a real size until we've downloaded
    # images.
    self.resize_this_and_mainwin(800, 600)
    self.setFocus()
    #self.grabKeyboard()

    self._baseline = None
    self._current = None
    self._diff = None
    self._images = (None, None, None)
    self._next_set_of_images()
    self._images[0] = Image(self._baseline, self)
    self._images[1] = Image(self._current, self)
    self._images[2] = Image(self._diff, self)

    grid = QtGui.QGridLayout()
    grid.setHorizontalSpacing(10)
    grid.setVerticalSpacing(1)

    label_baseline = QtGui.QLabel(grid.widget())
    label_current = QtGui.QLabel(grid.widget())
    label_diff = QtGui.QLabel(grid.widget())
    label_baseline.setText("Baseline image:")
    label_current.setText("Davinci's current:")
    label_diff.setText("difference between them:")

    label_directions = QtGui.QLabel(grid.widget())
    label_directions.setText("Keyboard shorcuts:\n\n"
                             "y: yes, rebaseline\n"
                             "n: no, current image is wrong\n"
                             "u: unknown, I can't/don't want to decide now\n"
                             "q: quit")

    grid.addWidget(label_baseline,           0,0)
    grid.addWidget(label_current,            0,1)
    grid.addWidget(self._images[0].widget(), 1,0)
    grid.addWidget(self._images[1].widget(), 1,1)
    grid.addWidget(label_diff,               2,0)
    grid.addWidget(quit,                     2,1)
    grid.addWidget(self._images[2].widget(), 3,0)
    grid.addWidget(label_directions,         3,1)

    self.setLayout(grid)
    self.resize_this_and_mainwin(self.calc_width(), self.calc_height())
    self.show()

  def resize_this_and_mainwin(self, w, h):
    self.resize(w, h)
    # try not to resize the mainwin if we don't need to; it's annoying.
    cur_w = self._mainwin.width()
    cur_h = self._mainwin.height()
    self._mainwin.resize(max(w,cur_w), max(h,cur_h))

  def _die(self):
    print "You thought these test results were bugs:"
    for f in self._bugs:
      print "\t", f
    self._mainwin.close()

  def calc_width(self):
    return self._images[0].widget().width() + self._images[1].widget().width()

  def calc_height(self):
    return self._images[0].widget().height() + \
           self._images[2].widget().height() + \
           96   # cover labels + status bar

  def _update_images(self):
    self._images[0].update(self._baseline)
    self._images[1].update(self._current)
    self._images[2].update(self._diff)
    self._images[0].widget().update()
    self.resize_this_and_mainwin(self.calc_width(), self.calc_height())
    self.update()

  def _rebaseline(self):
    self.status("".join(["rebaselining ", self._baseline, "..."]))
    print "moving", self._current, "on top of", self._baseline
    shutil.move(self._current, self._baseline) # do the rebaseline!
    self._next_set_of_images()
    self._update_images()

  def _ignore(self):
    self.status("".join(["ignoring ", self._baseline, "..."]))
    self._bugs.append(self._baseline)
    self._next_set_of_images()
    self._update_images()

  def _unknown(self):
    self.status("".join(["unknown ", self._baseline, "..."]))
    self._next_set_of_images()
    self._update_images()

  def status(self, msg):
    self._mainwin.statusBar().showMessage(msg)
    QtCore.QCoreApplication.processEvents() # we're single threaded

  def _next_set_of_images(self):
    """Figures out the next set of images to display.  Downloads 'current' and
       'diff' results from davinci.  Sets filenames corresponding to baseline,
       current and diff images."""

    # use the right investigate routine
    if in_svn_repo():
      investigate = investigate_svn
    else:
      investigate = investigate_git

    if self._baseline is None:  # first call, build list.
      self._images = []
      print "Building initial file list... please wait."
      self.status("Building initial file list... please wait.")
      for root, dirs, files in os.walk("baseline"):
        for f in files:
          fn, ext = os.path.splitext(f)
          if ext == ".png":
            # In some cases, we can trivially reject a file.  Don't bother
            # adding it to our list in that case.
            baseline_fn = os.path.join(root, f)
            current_fn = current_from_baseline(baseline_fn)
            if investigate(baseline_fn, current_fn):
              self._images.append(os.path.join(root, f))

    try:
      while True:  # stupid python doesn't have a do-while.
        self._baseline = self._images.pop()

        # now derive other filenames based on that one.
        filename = None
        # os.path.split fails if there's no /
        try:
          filename = os.path.split(self._baseline)
          filename = filename[1]
        except AttributeError, e:
          self.status("No slash!")
          break
        current_url = davinci_serial + "/c_" + filename
        f,info = urllib.urlretrieve(current_url, "local_current.png")
        self.status("".join(["Checking ", current_url, "..."]))

        if info.getheader("Content-Type").startswith("text/html"):
          # then it's a 404 or other error; skip this image.
          continue
        else:
          # We found the next image.
          self._current = "local_current.png"
          diff_url = davinci_serial + "/d_" + filename
          f,info = urllib.urlretrieve(diff_url, "local_diff.png")
          if info.getheader("Content-Type").startswith("text/html"):
            raise Exception("Could not download diff image.")
          self._diff = "local_diff.png"
          self.status("Waiting for input on " + filename)
          break
    except KeyError, e:
      print e
      print "No more images!"
      self.emit(QtCore.SIGNAL('closeApp()'))

  def _init_signals(self):
    self.connect(self, QtCore.SIGNAL('rebaseline()'), self._rebaseline)
    self.connect(self, QtCore.SIGNAL('ignore()'), self._ignore)
    self.connect(self, QtCore.SIGNAL('unknown()'), self._unknown)

  def keyPressEvent(self, event):
    if event.key() == QtCore.Qt.Key_Q:
      self.emit(QtCore.SIGNAL('closeApp()'))
    if event.key() == QtCore.Qt.Key_Y:
      self.emit(QtCore.SIGNAL('rebaseline()'))
    if event.key() == QtCore.Qt.Key_N:
      self.emit(QtCore.SIGNAL('ignore()'))
    if event.key() == QtCore.Qt.Key_U:
      self.emit(QtCore.SIGNAL('unknown()'))
    QtCore.QCoreApplication.processEvents()

app = QtGui.QApplication(sys.argv)
mw = MW()
mw.show()

mw.setWindowTitle("visit rebaseline -p")

layout = Layout(mw)
layout.show()

sys.exit(app.exec_())
