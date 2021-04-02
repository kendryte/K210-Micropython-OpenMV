python

import sensor,lcd

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)

lcd.init()
# if gc0328
sensor.sw(0)    # 0,1
while(True):
    lcd.display(sensor.snapshot()) 


#复制上面内容到msh后按三四次回车
