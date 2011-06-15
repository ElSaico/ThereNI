#!/usr/bin/env python
# coding=utf-8
import freenect
import pyglet
import numpy
import simpleOSC

simpleOSC.initOSCClient("127.0.0.1", 8765)

depth_win = pyglet.window.Window()
video_win = pyglet.window.Window()

depth = pyglet.image.ImageData(640, 480, "I",   numpy.empty((640, 480)))
video = pyglet.image.ImageData(640, 480, "RGB", numpy.empty((640, 480)))

@depth_win.event
def on_draw():
	depth_win.clear()
	feed, _ = freenect.sync_get_depth()
	depth.set_data("I", -640, feed.astype(numpy.uint8).tostring())
	depth.blit(0, 0)

@video_win.event
def on_draw():
	video_win.clear()
	feed, _ = freenect.sync_get_video()
	video.set_data("RGB", -640*3, feed.tostring())
	video.blit(0, 0)

def update_sound(dt):
	feed, _ = freenect.sync_get_depth()
	# menores distâncias, etc
	#simpleOSC.sendOSCMsg("/update", [freq, vol])

if __name__ == "__main__":
	pyglet.clock.schedule(update_sound)
	pyglet.app.run()
