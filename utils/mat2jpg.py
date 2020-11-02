# Convert a matrix to the input format for our program into "jpg" image.
# Input mat should be a squared matrix.
#
# ---------------------------------------------------------------------------------------------------
# author : Josselin Lefevre 11/2020

import sys
import os
import math
import numpy as np
from PIL import Image

if len(sys.argv) != 2:
    print("usage: mat2jpg.py <mat>...")
    quit()

for i, arg in enumerate(sys.argv[1:], 1):
    f=open(str(arg), 'r') #open matrix file
    dat = []
    for line in f:
        for num in line.split():
            dat.append(float(num)) #read values
    shp = int(math.sqrt(len(dat))) #get matrix size
    img = np.array(dat, dtype=np.uint8).reshape((shp, shp))
    im = Image.fromarray(img)
    fname = os.path.splitext(arg)[0]+".jpg"
    im.save(fname) #save image
    print("Image %d/%d done -> %s" % (i, len(sys.argv)-1, fname))
