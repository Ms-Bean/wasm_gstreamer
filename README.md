# This is a project im doing over winter break.


##First achievement, a simple pipeline editor in ImGui (no gstreamer)
Firstly, install the submodules by doing

```
git submodule update
git submodule init
```
Next, activate emsdk
```
source ./emsdk/emsdk_env.sh
```
Next, build the pipeline editor by doing

```
make
```

while in the wasm\_gstreamer directory.

This will compile a simple pipeline editor made in ImGui into the web directory, which can be served and seen by visiting index.html. Its mechanics are defined in pipeline\_components.h, and diagram\_editor.h. So far, it just takes two numbers and adds them and outputs it to another component. At its core, it works by having a function pointer, and buffers for output values inside each pipeline componenet, with its input values being the output buffers of other pipeline components. The idea is basically to use this to pass instances of Gst\_Element as inputs instead of numbers once we can manage to get gstreamer compiled to wasm, which has proven nigh impossible.

This is how the functions in pipeline_components.h work.

```c
struct Pipeline pipeline_init(void);
```
This will return a Pipeline struct. It stores an array of pipeline components, "components", as well as the number of components in the pipeline.

```c
int pipeline_add_component(struct Pipeline *pipeline, int n_inputs, int n_params, int n_outputs, void (*action)(void **, void **, void ***));
```
This will add a component to the pipeline. It takes in a pointer to the Pipeline, the number of inputs the component has, the number of internal parameters it has, the number of outputs it has, and a pointer to the function that it should execute in that order. The function should read the first parameter as an array of pointers to buffers that hold input values. It cshould read the second parameter as an array of pointers to buffers that hold internal parameters. It should then read the last parameter as an array of pointers to pointers to output buffers, and allocate space for the outputs using the pointers, and store its output in that space. i.e. `*(outputs[i]) = malloc...`. In fact, the input buffers are really just pointers to the output buffers allocated by other pipeline components. It will return the index of the new component.
Here is an example of a function that can be passed in as a function pointer
```c
void add(void **inputs, void **params, void ***outputs)
{
    /*Get inputs*/
    int a = *((int *)inputs[0]);
    int b = *((int *)inputs[1]);

    /*Send outputs*/
    *(outputs[0]) = malloc(sizeof(int));
    if(*(outputs[0]) == NULL)
    {
        fprintf(stderr, "add: malloc error\n");
        exit(4);
    }
    *((int *)(*(outputs[0]))) = a + b;
}
```

```c
void pipeline_add_connection(struct Pipeline *pipeline, int component_index_from, int output_index_from, int component_index_to, int input_index_to);
```
This function will add a connection from the pipeline component at index `component_index_from` from it's `output_index_from`th input, to the `input_index_to`th input on the pipeline component at `component_index_to`. This works by setting one pipeline's input to point to the output buffer of another pipeline component.
```c

void pipeline_set_param(struct Pipeline *pipeline, int component_index, int param_index, void *param);
{
    pipeline->components[component_index]->params[param_index] = param;
}
```
This function will set the `param_index`th internal parameter of the pipeline at index `component_index` to be `param`. `param` should therefore point to a chunk of memory that you want the pipeline component to read as a parameter.
```c
void pipeline_remove_param(struct Pipeline *pipeline, int component_index, int param_index);
```
This function will remove the `param_index`th internal parameter from the pipeline component at index `component_index`. It does not free the memory holding the parameter. The caller must do that themself if it is heap allocated.

```c
void pipeline_remove_connection(struct Pipeline *pipeline, int component_index_from, int output_index_from);
```
This function will remove the outgoing connection from the `output_index_from`th output of the pipeline component at `component_index_from`.

```c
void pipeline_remove_component(struct Pipeline *pipeline, int component_index);
```
This function will remove a component from the pipeline. 

```c
void pipeline_free_outputs(struct Pipeline *pipeline)
```
This function will free all of the output buffers.
```c
void pipeline_destroy(struct Pipeline *pipeline);
```
This function will destroy the pipeline, removing all of the components and freeing all output buffers.
```c
void pipeline_execute(struct Pipeline *pipeline);
```
This will execute the pipeline by searching for pipeline components that have recieved of their inputs, and calling the function pointer that the pipeline component contains. It will repeat this until there are no more pipeline components that have received all of its inputs.
```c
void **pipeline_fetch_inputs(struct Pipeline *pipeline, int component_index);
```
This function will return an array of pointers to the inputs fed into the pipeline component at `component_index`.

##Second achievement, a very simple gstreamer pipeline builder in interim/gst.pi

This is a python script with functions where each function adds a gstreamer element onto a string, and then executes the string as a gstreamer pipeline using os.system(). The GUI was made using Tkinter

