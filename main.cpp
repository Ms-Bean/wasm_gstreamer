#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "diagram_editor.h"
#include "pipeline_functions.h"

GLFWwindow* g_window;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
bool show_demo_window = true;
bool show_another_window = true;
int g_width;
int g_height;

char *number_source_title = (char *)"Number";
char *number_source_parameter_labels[] = {(char *)"value"};
char *number_source_output_labels[] = {(char *)"output"};

char *add_title = (char *)"Add";
char *add_input_labels[] = {(char *)"input 1", (char *)"input 2"};
char *add_output_labels[] = {(char *)"sum"};

char *sink_title = (char *)"Sink";
char *sink_input_labels[] = {(char *)"input 1"};

struct Diagram diagram;

EM_JS(int, canvas_get_width, (), {
  return Module.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
  return Module.canvas.height;
});

EM_JS(void, resizeCanvas, (), {
  js_resizeCanvas();
});

void on_size_changed()
{
  glfwSetWindowSize(g_window, g_width, g_height);

  ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}

void loop()
{
  int width = canvas_get_width();
  int height = canvas_get_height();

  if (width != g_width || height != g_height)
  {
    g_width = width;
    g_height = height;
    on_size_changed();
  }

  glfwPollEvents();

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  

  show_diagram(&diagram);

  ImGui::Render();

  int display_w, display_h;
  glfwMakeContextCurrent(g_window);
  glfwGetFramebufferSize(g_window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwMakeContextCurrent(g_window);
}


int init_gl()
{
  if( !glfwInit() )
  {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return 1;
  }

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int canvasWidth = g_width;
  int canvasHeight = g_height;
  g_window = glfwCreateWindow(canvasWidth, canvasHeight, "Demo", NULL, NULL);
  if( g_window == NULL )
  {
    fprintf( stderr, "Failed to open GLFW window.\n" );
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(g_window);

  return 0;
}


int init_imgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(g_window, true);
  ImGui_ImplOpenGL3_Init();

  ImGui::StyleColorsDark();

  ImGuiIO& io = ImGui::GetIO();

  io.Fonts->AddFontDefault();

  resizeCanvas();

  return 0;
}

int init_diagram()
{
  diagram = diagram_init();
  diagram_add_component(&diagram, number_source_title, 0, 0, NULL, number_source_output_labels, number_source_parameter_labels, 0, 1, 1, number_source);
  diagram_add_component(&diagram, number_source_title, 0, 1, NULL, number_source_output_labels, number_source_parameter_labels, 0, 1, 1, number_source);
  
  diagram_add_component(&diagram, add_title, 0, 2, add_input_labels, add_output_labels, NULL, 2, 0, 1, add);
  diagram_add_component(&diagram, sink_title, 0, 3, sink_input_labels, NULL, NULL, 1, 0, 0, nop);
  
  return 0;
}

int init()
{
  init_gl();
  init_imgui();
  init_diagram();
  return 0;
}


void quit()
{
  glfwTerminate(); 
}


extern "C" int main(int argc, char** argv)
{
  g_width = canvas_get_width();
  g_height = canvas_get_height();
  if (init() != 0) return 1;

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, 1);
  #endif

  quit();

  return 0;
}