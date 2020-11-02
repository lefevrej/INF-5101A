# Convert an image to an input file for the project programs. The created matrices are square.
#
# ---------------------------------------------------------------------------------------------------
# author : Josselin Lefevre 11/2020

import os
import sys
import numpy as np
from PIL import Image

if len(sys.argv) != 3:
    print("usage: img2mat.py <mat> <mat_size>")
    quit()

arg = sys.argv[1] #image filename
mat_size = int(sys.argv[2])
img = Image.open(arg).convert('L') #open image
img = np.asarray(img)[:mat_size,:mat_size] #crop image in squared matrix
fname=os.path.splitext(arg)[0] #output filename 
np.savetxt(fname, img, fmt='%3d') #save matrix
print("Done -> %s" % (fname))
