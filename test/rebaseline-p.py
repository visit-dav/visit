import filecmp
import os
import sys
import shutil
import subprocess
import time
import urllib
from optparse import OptionParser
from PyQt4 import QtCore,QtGui

parser = OptionParser()
parser.add_option("-r", "--root", dest="web_root",
                  default="http://portal.nersc.gov/project/visit/",
                  help="Root of web URL where baselines are")
parser.add_option("-d", "--date", dest="web_date",
                  help="Date of last good run, in YYMonDD form")
parser.add_option("-m", "--mode", dest="mode",
                  help="Mode to run in: serial, parallel, sr")
parser.add_option("-w", "--web-url", dest="web_url",
                  help="Manual URL specification; normally generated "
                       "automatically based on (-r, -d, -m)")
parser.add_option("-g", "--git", dest="git", action="store_true",
                  help="Use git to ignore images with local modifications")
parser.add_option("-s", "--svn", dest="svn", action="store_true",
                  help="Use svn to ignore images with local modifications")
(options, args) = parser.parse_args()

if options.web_url is not None:
  uri = options.web_url
else:
  uri = options.web_root + options.web_date + "/"
  mode = ""
  if options.mode == "sr" or options.mode == "scalable,parallel":
    mode="davinci_scalable_parallel_icet"
  else:
    mode="".join([ s for s in ("davinci_", options.mode) ])
  uri += mode + "/"
parser.destroy()

print "uri:", uri

class MW(QtGui.QMainWindow):
  def __init__(self, parent=None):
    QtGui.QMainWindow.__init__(self, parent)

def real_dirname(path):
  """Python's os.path.dirname is not dirname."""
  return path.rsplit('/', 1)[0]

def real_basename(path):
  """Python's os.path.basename is not basename."""
  return path.rsplit('/', 1)[1]

def baseline_current(serial_baseline):
  """Given the path to the serial baseline image, determine if there is a mode
     specific baseline.  Return a 2-tuple of the baseline image and the path to
     the 'current' image."""
  dname = real_dirname(serial_baseline)
  bname = real_basename(serial_baseline)
  baseline = serial_baseline

  if options.mode is not None:
    # Check for a mode specific baseline.
    mode_spec = os.path.join(dname + "/", options.mode + "/", bname)
    if os.path.exists(mode_spec):
      baseline = mode_spec

  # `Current' image never has a mode-specific path; filename/dir is always
  # based on the serial baseline's directory.
  no_baseline = serial_baseline.split('/', 1) # path without "baseline/"
  current = os.path.join("current/", no_baseline[1])

  return (baseline, current)

def local_modifications_git(file):
  vcs_diff = subprocess.call(["git", "diff", "--quiet", file])
  if vcs_diff == 1:
    return True
  return False

def local_modifications_svn(file):
  svnstat = subprocess.Popen("svn stat %s" % file, shell=True,
                             stdout=subprocess.PIPE)
  diff = svnstat.communicate()[0]
  if diff != '':
    return True
  return False

def local_modifications(filepath):
  """Returns true if the file has local modifications.  Always false if the
     user did not supply the appropriate VCS option."""
  if options.git: return local_modifications_git(filepath)
  if options.svn: return local_modifications_svn(filepath)
  return False

def equivalent(baseline, image):
  """True if the files are the same."""
  if not os.path.exists(image): return False
  # Note this is `shallow' by default, but that's fine for our usage.
  return filecmp.cmp(baseline, image)

def trivial_pass(baseline, image):
  """True if we can determine that this image is OK without querying the
     network."""
  return equivalent(baseline, image) or local_modifications(baseline)

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
    self._mainwin.statusBar().update()
    QtCore.QCoreApplication.processEvents() # we're single threaded

  def _next_set_of_images(self):
    """Figures out the next set of images to display.  Downloads 'current' and
       'diff' results from davinci.  Sets filenames corresponding to baseline,
       current and diff images."""

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
            serial_baseline_fn = os.path.join(root, f)

            # Does this path contain "parallel" or "scalable_parallel"?  Then
            # we've got a mode specific baseline.  We'll handle those based on
            # the serial filenames, so ignore them for now.
            if serial_baseline_fn.find("parallel") != -1: continue

            baseline_fn, current_fn = baseline_current(serial_baseline_fn)
            assert os.path.exists(baseline_fn)
            if not trivial_pass(baseline_fn, current_fn):
              self._images.append(baseline_fn)

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
        current_url = uri + "/c_" + filename
        f,info = urllib.urlretrieve(current_url, "local_current.png")
        self.status("".join(["Checking ", current_url, "..."]))

        if info.getheader("Content-Type").startswith("text/html"):
          # then it's a 404 or other error; skip this image.
          continue
        else:
          # We found the next image.
          self._current = "local_current.png"
          diff_url = uri + "/d_" + filename
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
