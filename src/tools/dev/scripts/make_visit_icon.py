import os, sys

visitpng = "visit210.png"
try:
    visitpng = sys.argv[1]
except:
    print("Usage: python make_visit_icon.py filename")
    sys.exit(0)

resolutions = (
("icon_512x512@2x", "1024x1024"),
("icon_512x512", "512x512"),
("icon_256x256@2x", "512x512"),
("icon_256x256", "256x256"),
("icon_128x128@2x", "256x256"),
("icon_128x128", "128x128"),
("icon_32x32@2x", "64x64"),
("icon_32x32", "32x32"),
("icon_16x16@2x", "32x32"),
("icon_16x16", "16x16"))

try:
    os.mkdir("VisItIcon.iconset")
except:
    print("VisItIcon.iconset already exists.")

for res in resolutions:
    cmd = "convert -resize %s\\! %s VisItIcon.iconset/%s.png" % (res[1], visitpng, res[0])
    print(cmd)
    os.system(cmd)

cmd = "iconutil -c icns VisItIcon.iconset VisItIcon.icns"
os.system(cmd)
