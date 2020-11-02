import os
import sys
import numpy as np
from PIL import Image

if len(sys.argv) != 2:
    print("usage: img2mat.py <mat>...")
    quit()

for i, arg in enumerate(sys.argv[1:], 1):
    img = Image.open(arg).convert('L')
    img = np.asarray(img)[:500,:500]
    fname=os.path.splitext(arg)[0]
    np.savetxt(fname, img, fmt='%3d')
    print("Image %d/%d done -> %s" % (i, len(sys.argv)-1, fname))
