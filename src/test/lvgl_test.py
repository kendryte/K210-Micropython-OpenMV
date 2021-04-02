#需要配置为32位色、开启GC、关闭demo、选择last版本;
#配置项:online pakeges\system\LittlevGL2RTT
#lvdrv.init()中已经调用lv.init(),多次调用lv.init()也无妨
#python
import lvgl as lv
import lvdrv

lvdrv.init()

scr = lv.obj()
btn = lv.btn(scr)
btn.align(lv.scr_act(), lv.ALIGN.CENTER, 0, 0)
label = lv.label(btn)
label.set_text("Hello World!")
lv.scr_load(scr)
