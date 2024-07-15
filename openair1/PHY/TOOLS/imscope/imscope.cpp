#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#include "implot.h"
#include "openair1/PHY/defs_nr_UE.h"
extern "C" {
#include "openair1/PHY/TOOLS/phy_scope_interface.h"
uint64_t get_softmodem_optmask(void);
}
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
static std::vector<int> rb_boundaries;

void copyDataThreadSafeNoLock(void *scopeData,
                              enum scopeDataType type,
                              void *dataIn,
                              int elementSz,
                              int colSz,
                              int lineSz,
                              int offset);

static void glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

typedef struct ImScopeData {
  scopeGraphData_t *scope_graph_data;
  std::atomic<bool> ptr_available_for_writing;
  std::atomic<bool> ptr_available_for_reading;
} ImScopeData;

static ImScopeData *scope_array;

class IQHist {
 private:
  bool frozen = false;
  bool next = false;
  float range = 100;
  int num_bins = 33;
  float timestamp = 0;
  std::vector<int16_t> real;
  std::vector<int16_t> imag;
  ImScopeData *scope_data;
  std::string label;
  int len = 0;
  float max = 0;
  int nonzero_count = 0;
  float min_nonzero_percentage = 0.9;
  float epsilon = 0.0;
  bool auto_adjust_range = false;

 public:
  IQHist(ImScopeData *scope_data_, const char *label_)
  {
    scope_data = scope_data_;
    label = label_;
  };
  void ReadData(float time)
  {
    if (!frozen || next) {
      auto is_available_for_reading = scope_data->ptr_available_for_reading.load();
      if (is_available_for_reading) {
        timestamp = time;
        scopeGraphData_t *iq_header = scope_data->scope_graph_data;
        len = iq_header->lineSz;
        real.reserve(len);
        imag.reserve(len);
        c16_t *source = (c16_t *)(iq_header + 1);
        max = 0;
        nonzero_count = 0;
        for (auto i = 0; i < len; i++) {
          real[i] = source[i].r;
          imag[i] = source[i].i;
          max = std::max(max, (float)source[i].r);
          max = std::max(max, (float)source[i].i);
          nonzero_count = std::abs(source[i].r) > epsilon || std::abs(source[i].i) > epsilon ? nonzero_count + 1 : nonzero_count;
        }
        if (auto_adjust_range) {
          if (max > range) {
            range = max * 1.1;
          }
        }
        scope_data->ptr_available_for_reading.store(false);
        scope_data->ptr_available_for_writing.store(true);
        float nonzero_ratio = (float)nonzero_count / float(len);
        if (frozen && nonzero_ratio > min_nonzero_percentage) {
          next = false;
        }
      }
    }
  }
  void Draw(float time)
  {
    ImGui::BeginGroup();
    if (ImGui::Checkbox("auto adjust range", &auto_adjust_range)) {
      if (auto_adjust_range) {
        range = max * 1.1;
      }
    }
    ImGui::BeginDisabled(auto_adjust_range);
    ImGui::SameLine();
    ImGui::DragFloat("Range", &range, 1, 0.1, std::numeric_limits<int16_t>::max());
    ImGui::EndDisabled();

    ImGui::DragInt("Number of bins", &num_bins, 1, 33, 101);
    if (ImGui::Button(frozen ? "Unfreeze" : "Freeze")) {
      frozen = !frozen;
      next = false;
    }
    if (frozen) {
      ImGui::SameLine();
      ImGui::BeginDisabled(next);
      if (ImGui::Button("Load next histogram")) {
        next = true;
      }
      ImGui::EndDisabled();
      ImGui::Text("Filter parameters");
      ImGui::DragFloat("%% nonzero elements", &min_nonzero_percentage, 1, 0.0, 100);
      ImGui::DragFloat("epsilon", &epsilon, 1, 0.0, 3000);
    }
    if (ImPlot::BeginPlot(label.c_str(), {(float)ImGui::GetWindowWidth() * 0.3f, (float)ImGui::GetWindowWidth() * 0.3f})) {
      ReadData(time);
      ImPlot::PlotHistogram2D(label.c_str(),
                              real.data(),
                              imag.data(),
                              len,
                              num_bins,
                              num_bins,
                              ImPlotRect(-range, range, -range, range));
      ImPlot::EndPlot();
    }
    ImGui::Text("Maximum value = %f, nonzero elements/total %d/%d", max, nonzero_count, len);
    ImGui::Text("Data is %.2f seconds old", time - timestamp);
    ImGui::EndGroup();
  }
};

class IQSlotHeatmap {
 private:
  bool frozen = false;
  bool next = false;
  float timestamp = 0;
  std::vector<float> power;
  ImScopeData *scope_data;
  std::string label;
  int len = 0;
  float max = 0;
  float stop_at_min = 10000;

 public:
  IQSlotHeatmap(ImScopeData *scope_data_, const char *label_)
  {
    scope_data = scope_data_;
    label = label_;
  };
  // Read in the data from the sink and transform it for the use by the scope
  void ReadData(float time, int ofdm_symbol_size, int num_symbols, int first_carrier_offset, int num_rb)
  {
    auto num_sc = num_rb * 12;
    if (!frozen || next) {
      auto is_available_for_reading = scope_data->ptr_available_for_reading.load();
      if (is_available_for_reading) {
        uint16_t first_sc = first_carrier_offset;
        uint16_t last_sc = first_sc + num_rb * 12;
        bool wrapped = false;
        uint16_t wrapped_first_sc = 0;
        uint16_t wrapped_last_sc = 0;
        if (last_sc >= ofdm_symbol_size) {
          last_sc = ofdm_symbol_size - 1;
          wrapped = true;
          auto num_sc_left = num_sc - (last_sc - first_sc + 1);
          wrapped_last_sc = wrapped_first_sc + num_sc_left - 1;
        }
        timestamp = time;
        scopeGraphData_t *iq_header = scope_data->scope_graph_data;
        len = iq_header->lineSz;
        c16_t *source = (c16_t *)(iq_header + 1);

        power.reserve(num_sc * num_symbols);
        for (auto symbol = 0; symbol < num_symbols; symbol++) {
          int subcarrier = 0;
          for (auto sc = first_sc; sc <= last_sc; sc++) {
            auto source_index = sc + symbol * ofdm_symbol_size;
            power[subcarrier * num_symbols + symbol] = std::pow(source[source_index].r, 2) + std::pow(source[source_index].i, 2);
            subcarrier++;
          }
          if (wrapped) {
            for (auto sc = wrapped_first_sc; sc <= wrapped_last_sc; sc++) {
              auto source_index = sc + symbol * ofdm_symbol_size;
              power[subcarrier * num_symbols + symbol] = std::pow(source[source_index].r, 2) + std::pow(source[source_index].i, 2);
              subcarrier++;
            }
          }
        }
        max = *std::max_element(power.begin(), power.end());
        scope_data->ptr_available_for_reading.store(false);
        scope_data->ptr_available_for_writing.store(true);
        if (frozen && max > stop_at_min) {
          next = false;
        }
      }
    }
  }
  void Draw(float time, int ofdm_symbol_size, int num_symbols, int first_carrier_offset, int num_rb)
  {
    ReadData(time, ofdm_symbol_size, num_symbols, first_carrier_offset, num_rb);
    ImGui::BeginGroup();
    if (ImGui::Button(frozen ? "Unfreeze" : "Freeze")) {
      frozen = !frozen;
      next = false;
    }
    if (frozen) {
      ImGui::SameLine();
      ImGui::BeginDisabled(next);
      if (ImGui::Button("Load next data")) {
        next = true;
      }
      ImGui::EndDisabled();
      ImGui::Text("Filter parameters:");
      ImGui::InputFloat("Max Power minimum", &stop_at_min, 10, 100);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Data with maximum power below that value will be discarded.");
      }
    }
    static std::vector<int> symbol_boundaries = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    if (ImPlot::BeginPlot(label.c_str(), {(float)ImGui::GetWindowWidth() * 0.9f, 0})) {
      auto num_sc = num_rb * 12;
      ImPlot::SetupAxes("symbols", "subcarriers");
      ImPlot::SetupAxisLimits(ImAxis_X1, num_symbols, 0);
      ImPlot::SetupAxisLimits(ImAxis_Y1, num_sc, 0);
      ImPlot::PlotHeatmap(label.c_str(),
                          power.data(),
                          num_sc,
                          num_symbols,
                          0,
                          max,
                          nullptr,
                          {0, 0},
                          {(double)num_symbols, (double)num_sc});
      ImPlot::PlotInfLines("Symbol boundary", symbol_boundaries.data(), symbol_boundaries.size());
      ImPlot::PlotInfLines("RB boundary", rb_boundaries.data(), num_rb, ImPlotInfLinesFlags_Horizontal);
      ImPlot::EndPlot();
    }
    ImGui::SameLine();
    ImPlot::ColormapScale("##HeatScale", 0, max);
    ImGui::Text("Data is %.2f seconds old", time - timestamp);
    ImGui::EndGroup();
  }
};

static void IQScatterPlot(float time, enum scopeDataType type, const char *label)
{
  ImGui::BeginGroup();
  ImScopeData &scope_data = scope_array[type];
  static float timestamp = 0;
  if (ImPlot::BeginPlot(label)) {
    static std::vector<int16_t> iq_data;
    static int len = 0;
    auto is_available_for_reading = scope_data.ptr_available_for_reading.load();
    if (is_available_for_reading) {
      timestamp = time;
      scopeGraphData_t *iq_header = scope_data.scope_graph_data;
      len = std::min(iq_header->lineSz, 1 << 15); // Limit the number of points in scatterplot
      iq_data.reserve(len);
      memcpy(iq_data.data(), (int16_t *)(iq_header + 1), sizeof(int16_t) * 2 * len);
      scope_data.ptr_available_for_reading.store(false);
      scope_data.ptr_available_for_writing.store(true);
    }
    ImPlot::PlotScatter(label, iq_data.data(), iq_data.data() + 1, len, 0, 0, sizeof(int16_t) * 2);
    ImPlot::EndPlot();
  }
  ImGui::Text("Data is %.2f seconds old", time - timestamp);
  ImGui::EndGroup();
}

static void ChanEstPlot(float time, enum scopeDataType type, const char *label)
{
  ImGui::BeginGroup();
  ImScopeData &scope_data = scope_array[type];
  static float timestamp = 0;
  if (ImPlot::BeginPlot(label)) {
    static std::vector<float> estimates;
    static int len = 0;
    auto is_available_for_reading = scope_data.ptr_available_for_reading.load();
    if (is_available_for_reading) {
      timestamp = time;
      const c16_t *tmp = (c16_t *)(scope_data.scope_graph_data + 1);
      len = scope_data.scope_graph_data->lineSz;
      estimates.reserve(len);
      for (auto i = 0; i < len; i++) {
        auto index = (len / 2 + i) % len;
        estimates[index] = log(tmp[i].r * tmp[i].r + tmp[i].i * tmp[i].i);
      }
      scope_data.ptr_available_for_reading.store(false);
      scope_data.ptr_available_for_writing.store(true);
    }
    ImPlot::PlotLine(label, estimates.data(), len);
    ImPlot::EndPlot();
  }
  ImGui::Text("Data is %.2f seconds old", time - timestamp);
  ImGui::EndGroup();
}

static void LLRPlot(float time, enum scopeDataType type, const char *label)
{
  ImGui::BeginGroup();
  static float timestamp = 0;
  ImScopeData &scope_data = scope_array[type];
  if (ImPlot::BeginPlot(label)) {
    static std::vector<float> llr;
    static int len = 0;
    auto is_available_for_reading = scope_data.ptr_available_for_reading.load();
    if (is_available_for_reading) {
      timestamp = time;
      const int16_t *tmp = (int16_t *)(scope_data.scope_graph_data + 1);
      len = scope_data.scope_graph_data->lineSz;
      llr.reserve(len);
      memcpy(llr.data(), tmp, sizeof(int16_t) * len);
      scope_data.ptr_available_for_reading.store(false);
      scope_data.ptr_available_for_writing.store(true);
    }

    ImPlot::PlotLine(label, llr.data(), len);
    ImPlot::EndPlot();
  }
  ImGui::Text("Data is %.2f seconds old", time - timestamp);
  ImGui::EndGroup();
}

// utility structure for realtime plot
struct RollingBuffer {
  float Span;
  ImVector<ImVec2> Data;
  RollingBuffer()
  {
    Span = 10.0f;
    Data.reserve(2000);
  }
  void AddPoint(float x, float y)
  {
    float xmod = fmodf(x, Span);
    if (!Data.empty() && xmod < Data.back().x)
      Data.shrink(0);
    Data.push_back(ImVec2(xmod, y));
  }
};

struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer(int max_size = 2000)
  {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y)
  {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase()
  {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
};

void *imscope_thread(void *data_void_ptr)
{
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return nullptr;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(1280, 720, "imscope", nullptr, nullptr);
  if (window == nullptr)
    return nullptr;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // For frame capping

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
  ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
  // ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an
  // assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
  // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See
  // Makefile.emscripten for details.
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr,
  // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  for (auto i = 0U; i < 273; i++) {
    rb_boundaries.push_back(i * 12);
  }
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy
    // of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your
    // copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide them from your application based
    // on those two flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // For reference
    ImPlot::ShowDemoWindow();
    ImGui::ShowDemoWindow();

    const int SOFTMODEM_5GUE_BIT = 1 << 23;
    bool is_ue = (get_softmodem_optmask() & SOFTMODEM_5GUE_BIT) > 0;
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
      ImGui::SetNextWindowPos({0, 0});
      ImGui::Begin("NR KPI");
      if (ImGui::TreeNode("Global settings")) {
        ImGui::ShowFontSelector("Font");
        ImGui::ShowStyleSelector("ImGui Style");
        ImPlot::ShowStyleSelector("ImPlot Style");
        ImPlot::ShowColormapSelector("ImPlot Colormap");
        ImGui::TreePop();
      }
      static float t = 0;
      t += ImGui::GetIO().DeltaTime;
      if (is_ue) {
        PHY_VARS_NR_UE *ue = (PHY_VARS_NR_UE *)data_void_ptr;
        if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
          static float history = 10.0f;
          ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
          static ScrollingBuffer rbs_buffer;
          static ScrollingBuffer bler;
          static ScrollingBuffer mcs;
          rbs_buffer.AddPoint(t, getKPIUE()->nofRBs);
          bler.AddPoint(t, (float)getKPIUE()->nb_nack / (float)getKPIUE()->nb_total);
          mcs.AddPoint(t, (float)getKPIUE()->dl_mcs);
          ImPlot::SetupAxes("time", "noOfRbs");
          ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
          ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 273);
          ImPlot::SetupAxis(ImAxis_Y2, "bler [%]",ImPlotAxisFlags_AuxDefault);
          ImPlot::SetupAxis(ImAxis_Y3, "MCS",ImPlotAxisFlags_AuxDefault);
          ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
          ImPlot::PlotLine("noOfRbs",
                           &rbs_buffer.Data[0].x,
                           &rbs_buffer.Data[0].y,
                           rbs_buffer.Data.size(),
                           0,
                           0,
                           2 * sizeof(float));
          ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
          ImPlot::PlotLine("bler",
                  &bler.Data[0].x,
                  &bler.Data[0].y,
                  bler.Data.size(),
                  0,
                  0,
                  2 * sizeof(float));
          ImPlot::SetAxes(ImAxis_X1, ImAxis_Y3);
          ImPlot::PlotLine("mcs",
                  &mcs.Data[0].x,
                  &mcs.Data[0].y,
                  mcs.Data.size(),
                  0,
                  0,
                  2 * sizeof(float));
          ImPlot::EndPlot();
        }
        if (ImGui::TreeNode("PDSCH IQ")) {
          static auto pdsch_iq_hist = new IQHist(&scope_array[pdschRxdataF_comp], "PDSCH IQ");
          pdsch_iq_hist->Draw(t);
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Broadcast channel")) {
          static auto broadcast_iq_hist =
              new IQHist(&scope_array[ue->sl_mode ? psbchRxdataF_comp : pbchRxdataF_comp], "Broadcast IQ");
          broadcast_iq_hist->Draw(t);
          ChanEstPlot(t, ue->sl_mode ? psbchDlChEstimateTime : pbchDlChEstimateTime, "Broadcast channel estimates");
          LLRPlot(t, ue->sl_mode ? pbchLlr : psbchLlr, "Broadcast LLR");
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("RX IQ")) {
          static auto common_rx_iq_heatmap = new IQSlotHeatmap(&scope_array[commonRxdataF], "common RX IQ");
          common_rx_iq_heatmap->Draw(t,
                                     ue->frame_parms.ofdm_symbol_size,
                                     ue->frame_parms.symbols_per_slot,
                                     ue->frame_parms.first_carrier_offset,
                                     ue->frame_parms.N_RB_DL);
          ImGui::TreePop();
        }
      } else {
        if (ImGui::TreeNode("RX IQ")) {
          scopeParms_t *scope_params = (scopeParms_t *)data_void_ptr;
          PHY_VARS_gNB *gNB = scope_params->gNB;
          static auto gnb_heatmap = new IQSlotHeatmap(&scope_array[gNBRxdataF], "common RX IQ");

          gnb_heatmap->Draw(t,
                            gNB->frame_parms.ofdm_symbol_size,
                            gNB->frame_parms.symbols_per_slot,
                            gNB->frame_parms.first_carrier_offset,
                            gNB->frame_parms.N_RB_UL);
          ImGui::TreePop();
        }
      }
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return nullptr;
}

extern "C" void imscope_autoinit(void *dataptr)
{
  // Copied here due to issues with headers
  const int SOFTMODEM_GNB_BIT = 1 << 21;
  const int SOFTMODEM_5GUE_BIT = 1 << 23;
  AssertFatal((get_softmodem_optmask() & SOFTMODEM_5GUE_BIT) || (get_softmodem_optmask() & SOFTMODEM_GNB_BIT),
              "Scope cannot find NRUE or GNB context");

  scope_array = static_cast<ImScopeData *>(calloc(MAX_SCOPE_TYPES, sizeof(*scope_array)));
  for (auto i = 0U; i < MAX_SCOPE_TYPES; i++) {
    scope_array[i].ptr_available_for_writing.store(true);
    scope_array[i].ptr_available_for_reading.store(false);
    scope_array[i].scope_graph_data = nullptr;
  }

  if (SOFTMODEM_GNB_BIT & get_softmodem_optmask()) {
    scopeParms_t *scope_params = (scopeParms_t *)dataptr;
    scopeData_t *scope = (scopeData_t *)calloc(1, sizeof(scopeData_t));
    scope->copyData = copyDataThreadSafeNoLock;
    scope_params->gNB->scopeData = scope;
  } else {
    PHY_VARS_NR_UE *ue = (PHY_VARS_NR_UE *)dataptr;
    scopeData_t *scope = (scopeData_t *)calloc(1, sizeof(scopeData_t));
    scope->copyData = copyDataThreadSafeNoLock;
    ue->scopeData = scope;
  }
  pthread_t thread;
  threadCreate(&thread, imscope_thread, dataptr, (char *)"imscope", -1, sched_get_priority_min(SCHED_RR));
}

void copyDataThreadSafeNoLock(void *scopeData,
                              enum scopeDataType type,
                              void *dataIn,
                              int elementSz,
                              int colSz,
                              int lineSz,
                              int offset)
{
  ImScopeData &scope_data = scope_array[type];
  auto is_available_for_writing = scope_data.ptr_available_for_writing.load();
  if (!is_available_for_writing) {
    return;
  }
  if (scope_data.ptr_available_for_writing.compare_exchange_weak(is_available_for_writing, false)) {
    scopeGraphData_t *data = scope_data.scope_graph_data;
    int oldDataSz = data ? data->dataSize : 0;
    int newSz = elementSz * colSz * lineSz;
    if (data == NULL || oldDataSz < newSz) {
      scopeGraphData_t *ptr = (scopeGraphData_t *)realloc(data, sizeof(scopeGraphData_t) + newSz);
      if (!ptr) {
        LOG_E(PHY, "can't realloc\n");
        return;
      } else {
        data = ptr;
      }
    }

    data->elementSz = elementSz;
    data->colSz = colSz;
    data->lineSz = lineSz;
    memcpy(((void *)(data + 1)), dataIn, newSz);
    scope_data.scope_graph_data = data;
    scope_data.ptr_available_for_reading.store(true);
  }
}
