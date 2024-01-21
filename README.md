# This is a project im doing over winter break.


##First achievement, a simple pipeline editor in ImGui (no gstreamer)
Firstly, install the submodules by doing

```
git submodule update
git submodule init
```

Next, build the pipeline editor by doing

```
make
```

while in the wasm\_gstreamer directory.

This will compile a simple pipeline editor made in ImGui into the web directory, which can be served and seen by visiting index.html. Its mechanics are defined in pipeline\_components.h, and diagram\_editor.h. So far, it just takes two numbers and adds them and outputs it to another component. At its core, it works by having a function pointer, and buffers for output values inside each pipeline componenet, with its input values being the output buffers of other pipeline components. The idea is basically to use this to pass instances of Gst\_Element as inputs instead of numbers once we can manage to get gstreamer compiled to wasm, which has proven nigh impossible.

##Second achievement, a very simple gstreamer pipeline builder in interim/gst.pi

This is a python script with functions where each function adds a gstreamer element onto a string, and then executes the string as a gstreamer pipeline using os.system(). The GUI was made using Tkinter
