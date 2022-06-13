# gst-fpslogger

FPS logging Gstreamer element.

# Requrements

```
$ sudo apt install -y build-essential cmake libgstreamer1.0-dev
```

# Installation

```
$ mkdir build && cd build 
$ cmake ../
$ make -j4
$ make install
```

# Example

```
$ gst-launch-1.0 videotestsrc ! fpslogger ! fakesink sync=true
Setting pipeline to PAUSED ...
Pipeline is PREROLLING ...
Pipeline is PREROLLED ...
Setting pipeline to PLAYING ...
New clock: GstSystemClock
fpslogger : fps = 31.986981
fpslogger : fps = 29.999641
fpslogger : fps = 30.000027
```