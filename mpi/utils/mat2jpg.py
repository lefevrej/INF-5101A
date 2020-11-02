import sys
import os
import numpy as np
from PIL import Image

if len(sys.argv) != 2:
    print("usage: mat2jpg.py <mat>...")
    quit()

for i, arg in enumerate(sys.argv[1:], 1):
    f=open(str(arg), 'r')
    dat = []
    for line in f:
        for num in line.split():
            dat.append(float(num))
    img = np.array(dat, dtype=np.float)
    img=img.reshape((500, 500)).astype(np.uint8)
    im = Image.fromarray(img)
    fname=os.path.splitext(arg)[0]+".jpg"
    im.save(fname)
    print("Image %d/%d done -> %s" % (i, len(sys.argv)-1, fname))
