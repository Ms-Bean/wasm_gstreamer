// Dear ImGui: standalone example application for Emscripten, using GLFW + WebGPU
// (Emscripten is a C++-to-javascript compiler, used to publish executables for the web. See https://emscripten.org/)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include "graph.h"
#include <stdio.h>
#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

// Global WebGPU required states
static WGPUDevice        wgpu_device = nullptr;
static WGPUSurface       wgpu_surface = nullptr;
static WGPUTextureFormat wgpu_preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
static WGPUSwapChain     wgpu_swap_chain = nullptr;
static int               wgpu_swap_chain_width = 0;
static int               wgpu_swap_chain_height = 0;

struct Window_Data
{
    char *title;
    bool is_active;
    float pos_x;
    float pos_y;
};

struct Graph nodes;

// Forward declarations
static void MainLoopStep(void* window);
static bool InitWGPU();
static void print_glfw_error(int error, const char* description);
static void print_wgpu_error(WGPUErrorType error_type, const char* message, void*);

// Main code
int main(int, char**)
{

    glfwSetErrorCallback(print_glfw_error);
    if (!glfwInit())
        return 1;

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui example", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    // Initialize the WebGPU environment
    if (!InitWGPU())
    {
        if (window)
            glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glfwShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
    ImGui_ImplWGPU_Init(wgpu_device, 3, wgpu_preferred_fmt, WGPUTextureFormat_Undefined);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Emscripten allows preloading a file or folder to be accessible at runtime. See Makefile for details.
    //io.Fonts->AddFontDefault();
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    //io.Fonts->AddFontFromFileTTF("fonts/segoeui.ttf", 18.0f);
    io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);
#endif

    struct Window_Data win1;
    struct Window_Data win2;

    win1.title = (char *)malloc(sizeof(char) * 50);
    win2.title = (char *)malloc(sizeof(char) * 50);
    win1.is_active = true;
    win2.is_active = true;
    win1.pos_x = 0;
    win1.pos_y = 0;
    win2.pos_x = 0;
    win2.pos_y = 0;

    strcpy(win1.title, "Hello world!\n");
    strcpy(win2.title, "Hello back!\n");

    nodes = graph_init(sizeof(struct Window_Data), 2);

    graph_setval(&nodes, 0, &win1);
    graph_setval(&nodes, 1, &win2);
    graph_add_edge(&nodes, 0, 1);

    // This function will directly return and exit the main function.
    // Make sure that no required objects get cleaned up.
    // This way we can use the browsers 'requestAnimationFrame' to control the rendering.
    emscripten_set_main_loop_arg(MainLoopStep, window, 0, false);

    return 0;
}

static bool InitWGPU()
{
    wgpu_device = emscripten_webgpu_get_device();
    if (!wgpu_device)
        return false;

    wgpuDeviceSetUncapturedErrorCallback(wgpu_device, print_wgpu_error, nullptr);

    // Use C++ wrapper due to misbehavior in Emscripten.
    // Some offset computation for wgpuInstanceCreateSurface in JavaScript
    // seem to be inline with struct alignments in the C++ structure
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
    html_surface_desc.selector = "#canvas";

    wgpu::SurfaceDescriptor surface_desc = {};
    surface_desc.nextInChain = &html_surface_desc;

    wgpu::Instance instance = wgpuCreateInstance(nullptr);
    wgpu::Surface surface = instance.CreateSurface(&surface_desc);
    wgpu::Adapter adapter = {};
    wgpu_preferred_fmt = (WGPUTextureFormat)surface.GetPreferredFormat(adapter);
    wgpu_surface = surface.Release();

    return true;
}

static void MainLoopStep(void* window)
{
    int i;
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);

    // React to changes in screen size
    if (width != wgpu_swap_chain_width && height != wgpu_swap_chain_height)
    {
        ImGui_ImplWGPU_InvalidateDeviceObjects();
        if (wgpu_swap_chain)
            wgpuSwapChainRelease(wgpu_swap_chain);
        wgpu_swap_chain_width = width;
        wgpu_swap_chain_height = height;
        WGPUSwapChainDescriptor swap_chain_desc = {};
        swap_chain_desc.usage = WGPUTextureUsage_RenderAttachment;
        swap_chain_desc.format = wgpu_preferred_fmt;
        swap_chain_desc.width = width;
        swap_chain_desc.height = height;
        swap_chain_desc.presentMode = WGPUPresentMode_Fifo;
        wgpu_swap_chain = wgpuDeviceCreateSwapChain(wgpu_device, wgpu_surface, &swap_chain_desc);
        ImGui_ImplWGPU_CreateDeviceObjects();
    }

    // Start the Dear ImGui frame
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    for(i = 0; i < nodes.num_vertices; i++)
    {                           // Create a window called "Hello, world!" and append into it.
        if(nodes.vertices[i] != NULL)
        {
            int j;
            struct Window_Data *window_data = (struct Window_Data *)graph_getval(&nodes, i);
            ImGui::Begin(window_data->title, &window_data->is_active);     
            ImGui::Text("%s\n", window_data->title);
            window_data->pos_x = ImGui::GetWindowPos().x;
            window_data->pos_y = ImGui::GetWindowPos().y;

            for(j = 0; j < nodes.num_vertices; j++)
            {
                if(graph_has_edge(&nodes, i, j))
                {
                    struct Window_Data *neighbor_data = (struct Window_Data *)graph_getval(&nodes, j);
                    ImVec2 pos_from = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
                    ImVec2 pos_to = ImVec2(neighbor_data->pos_x, neighbor_data->pos_y);
                    ImU32 color = 0xFFFFFFFF;
                    ImGui::GetForegroundDrawList()->AddLine(pos_from, pos_to, color);
                }
            }
            ImGui::End();
        }      
    }

    // Rendering
    ImGui::Render();

    WGPURenderPassColorAttachment color_attachments = {};
    color_attachments.loadOp = WGPULoadOp_Clear;
    color_attachments.storeOp = WGPUStoreOp_Store;
    color_attachments.clearValue = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    color_attachments.view = wgpuSwapChainGetCurrentTextureView(wgpu_swap_chain);
    WGPURenderPassDescriptor render_pass_desc = {};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachments;
    render_pass_desc.depthStencilAttachment = nullptr;

    WGPUCommandEncoderDescriptor enc_desc = {};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpu_device, &enc_desc);

    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
    wgpuRenderPassEncoderEnd(pass);

    WGPUCommandBufferDescriptor cmd_buffer_desc = {};
    WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
    WGPUQueue queue = wgpuDeviceGetQueue(wgpu_device);
    wgpuQueueSubmit(queue, 1, &cmd_buffer);
}

static void print_glfw_error(int error, const char* description)
{
    printf("GLFW Error %d: %s\n", error, description);
}

static void print_wgpu_error(WGPUErrorType error_type, const char* message, void*)
{
    const char* error_type_lbl = "";
    switch (error_type)
    {
    case WGPUErrorType_Validation:  error_type_lbl = "Validation"; break;
    case WGPUErrorType_OutOfMemory: error_type_lbl = "Out of memory"; break;
    case WGPUErrorType_Unknown:     error_type_lbl = "Unknown"; break;
    case WGPUErrorType_DeviceLost:  error_type_lbl = "Device lost"; break;
    default:                        error_type_lbl = "Unknown";
    }
    printf("%s error: %s\n", error_type_lbl, message);
}
