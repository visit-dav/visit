
import subprocess

def sexe(cmd):
    print("[exe: %s]" % cmd)
    subprocess.call(cmd,shell=True)
visit_ver = 3.5
dest_res = [ 64, 128, 512]
cmds = [ "magick convert -resize {0}x{0} VisIt{1}-square-macos-256x256.png VisIt3.5-square-macos-{0}x{0}.png",
         "magick convert -resize {0}x{0} VisIt{1}-square-win-256x256.png VisIt3.5-square-win-{0}x{0}.png"]

for dres in dest_res:
    for cmd in cmds:
        sexe(cmd.format(dres,visit_ver))

