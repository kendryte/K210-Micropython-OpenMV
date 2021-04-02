#python

from k210 import camera
from k210 import picture

cam = camera()

cam.reset()
cam.set_pixformat(cam.RGB565)
cam.set_framesize(320, 240)

for i in range(0, 1000):
img = cam.snapshot()
img.show()



cam.set_pixformat(cam.YUV422)
for i in range(0, 1000):
img = cam.snapshot()
img.show()





