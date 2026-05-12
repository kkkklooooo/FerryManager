#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Word.h"
#include "Organism.h"
#include "Environment.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>

// ============================================================
//  helper: environment -> colour
// ============================================================
static ImU32 EnvColor(const std::string& name, float energy, float maxE) {
    float i = (maxE > 0.001f) ? energy / maxE : 0.5f;
    i = std::clamp(i, 0.25f, 1.0f);
    int r, g, b;
    if (name == "GressLand")      { r = 107; g = 142; b = 35; }
    else if (name == "Water")     { r = 30;  g = 144; b = 255; }
    else if (name == "Forest")    { r = 34;  g = 139; b = 34; }
    else if (name == "Mountain")  { r = 139; g = 137; b = 137; }
    else                           { r = 128; g = 128; b = 128; }
    return IM_COL32((int)(r*i), (int)(g*i), (int)(b*i), 255);
}

static const char* EnvName(const std::string& name) {
    if (name == "GressLand") return "Grassland";
    if (name == "Water")     return "Water";
    if (name == "Forest")    return "Forest";
    if (name == "Mountain")  return "Mountain";
    return name.c_str();
}

static const char* OrganismDisplayName(const std::string& name) {
    return name.c_str();
}

static ImU32 OrganismColor(const std::string& name, float energy, float maxE) {
    float i = (maxE > 0.001f) ? energy / maxE : 0.5f;
    i = std::clamp(i, 0.25f, 1.0f);
    if (name == "Plant")       return IM_COL32(0,            (int)(220*i), (int)(80*i),  210);
    if (name == "Wolf")        return IM_COL32((int)(220*i), (int)(60*i),  (int)(50*i),  210);
    if (name == "Sheep")       return IM_COL32((int)(180*i), (int)(180*i), (int)(220*i), 210);
    if (name == "Animal")      return IM_COL32((int)(255*i), (int)(90*i),  (int)(70*i),  210);
    return IM_COL32((int)(128*i), (int)(128*i), (int)(128*i), 210);
}

// ============================================================
//  World grid
// ============================================================
static void DrawWorldGrid(const World& world) {
    const auto& envs = world.GetEnvironments();
    const auto& orgs = world.GetReproducas();
    int w = world.GetWidth(), h = world.GetHeight();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float cellSize = std::min(avail.x / w, avail.y / h);
    cellSize = std::max(cellSize, 3.0f);

    float maxE = 0.001f;
    for (auto* e : envs) if (e->energy > maxE) maxE = e->energy;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 base = ImGui::GetCursorScreenPos();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            ImVec2 p0(base.x + x*cellSize, base.y + y*cellSize);
            ImVec2 p1(p0.x + cellSize, p0.y + cellSize);

            ImU32 col = IM_COL32(20, 20, 20, 255);
            if (idx < (int)envs.size())
                col = EnvColor(envs[idx]->name, envs[idx]->energy, maxE);

            dl->AddRectFilled(p0, p1, col);
            dl->AddRect(p0, p1, IM_COL32(50, 50, 50, 255), 0, 0, 1.0f);
        }
    }

    float maxPlantE = 0.001f, maxAnimalE = 0.001f;
    for (auto* org : orgs) {
        if (!org)continue;


        if (org->type == PLANT) {
            if (org->energy > maxPlantE) maxPlantE = org->energy;
        } else {
            if (org->energy > maxAnimalE) maxAnimalE = org->energy;
        }
    }

    for (auto* org : orgs) {
        if (!org)continue;
        int x = org->Pos.first, y = org->Pos.second;
        if (x < 0 || x >= w || y < 0 || y >= h) continue;
        ImVec2 c(base.x + x*cellSize + cellSize*0.5f,
                 base.y + y*cellSize + cellSize*0.5f);
        float r = cellSize * 0.38f;
        float maxE = (org->type == PLANT) ? maxPlantE : maxAnimalE;
        ImU32 fill = OrganismColor(org->name, org->energy, maxE);
        dl->AddCircleFilled(c, r, fill);
        dl->AddCircle(c, r, IM_COL32(0, 0, 0, 80), 0, 1.5f);
    }

    ImGui::Dummy(ImVec2(w*cellSize, h*cellSize));

    if (ImGui::IsItemHovered()) {
        ImVec2 m = ImGui::GetMousePos();
        int gx = (int)((m.x - base.x)/cellSize);
        int gy = (int)((m.y - base.y)/cellSize);
        if (gx >= 0 && gx < w && gy >= 0 && gy < h) {
            int idx = gy*w + gx;
            ImGui::BeginTooltip();
            ImGui::Text("(%d, %d)", gx, gy);
            if (idx < (int)envs.size()) {
                ImGui::Text("Terrain: %s", EnvName(envs[idx]->name));
                ImGui::Text("Energy:  %.1f", envs[idx]->energy);
            }
            for (auto* org : orgs) {
                if (org->Pos.first == gx && org->Pos.second == gy)
                    ImGui::Text("%s  E=%.1f",
                        OrganismDisplayName(org->name), org->energy);
            }
            ImGui::EndTooltip();
        }
    }
}

// ============================================================
//  Energy heat-map
// ============================================================
static void DrawEnergyHeatmap(const World& world, float cs = 14.0f) {
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth(), h = world.GetHeight();

    float minE = FLT_MAX, maxE = FLT_MIN;
    for (auto* e : envs) {
        if (e->energy < minE) minE = e->energy;
        if (e->energy > maxE) maxE = e->energy;
    }
    float range = (maxE - minE) > 0.001f ? (maxE - minE) : 1.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 base = ImGui::GetCursorScreenPos();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y*w + x;
            ImVec2 p0(base.x + x*cs, base.y + y*cs);
            ImVec2 p1(p0.x + cs, p0.y + cs);
            ImU32 col = IM_COL32(20, 20, 20, 255);
            if (idx < (int)envs.size()) {
                float t = (envs[idx]->energy - minE) / range;
                int r = (int)(t * 255);
                int g = (int)((1.0f - std::abs(t - 0.5f)*2.0f) * 255);
                int b = (int)((1.0f - t) * 255);
                col = IM_COL32(r, g, b, 255);
            }
            dl->AddRectFilled(p0, p1, col);
        }
    }
    ImGui::Dummy(ImVec2(w*cs, h*cs));
    ImGui::Text("Range: %.1f .. %.1f", minE, maxE);
}

// ============================================================
//  Plant table
// ============================================================
static void DrawPlantList(const World& world) {
    const auto& orgs = world.GetReproducas();
    if (!ImGui::BeginTable("##plants", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
        return;

    ImGui::TableSetupColumn("ID",     ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("X",      ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Y",      ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Energy", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableHeadersRow();

    int n = 0;
    for (auto* org : orgs) {
        if (!org)continue;
        if (org->type != PLANT) continue;
        const Plant* p = static_cast<const Plant*>(org);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("%d",   p->id);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%d",   (int)org->Pos.first);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%d",   (int)org->Pos.second);
        ImGui::TableSetColumnIndex(3);
        ImVec4 c = org->energy > 5.0f ? ImVec4(0.3f,1,0.3f,1) : ImVec4(1,0.5f,0.3f,1);
        ImGui::TextColored(c, "%.1f", org->energy);
        if (++n >= 50) break;
    }
    ImGui::EndTable();
}

// ============================================================
//  Animal table
// ============================================================
static void DrawAnimalList(const World& world) {
    const auto& orgs = world.GetReproducas();
    if (!ImGui::BeginTable("##animals", 6,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
        return;

    ImGui::TableSetupColumn("ID",     ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Type",   ImGuiTableColumnFlags_WidthFixed, 60);
    ImGui::TableSetupColumn("X",      ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Y",      ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Energy", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Repro",  ImGuiTableColumnFlags_WidthFixed, 50);
    ImGui::TableHeadersRow();

    int n = 0;
    for (auto* org : orgs) {
        if (org->type == PLANT ) continue;
        ImGui::TableNextRow();
        const Animal* a = static_cast<const Animal*>(org);
        ImGui::TableSetColumnIndex(0); ImGui::Text("%d",   a->id);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%s",   OrganismDisplayName(org->name));
        ImGui::TableSetColumnIndex(2); ImGui::Text("%d",   (int)org->Pos.first);
        ImGui::TableSetColumnIndex(3); ImGui::Text("%d",   (int)org->Pos.second);
        ImGui::TableSetColumnIndex(4);
        ImVec4 c = org->energy > 15.0f ? ImVec4(0.3f,1,0.3f,1) : ImVec4(1,0.5f,0.3f,1);
        ImGui::TextColored(c, "%.1f", org->energy);
        ImGui::TableSetColumnIndex(5);
        ImVec4 rc = org->reproduce_able ? ImVec4(0.3f,1,0.3f,1) : ImVec4(0.6f,0.6f,0.6f,1);
        ImGui::TextColored(rc, "%s", org->reproduce_able ? "Y" : "N");
        if (++n >= 50) break;
    }
    ImGui::EndTable();
}

// ============================================================
//  Stats bar
// ============================================================
static void DrawStats(const World& world, int frame, int total) {
    const auto& orgs = world.GetReproducas();
    const auto& envs = world.GetEnvironments();
    int plants = 0, animals = 0, wolves = 0, sheep = 0;
    float pE = 0, aE = 0, envE = 0;
    for (auto* o : orgs) {

        if (!o) continue;

if (o->type == PLANT ) { ++plants;  pE += o->energy; }
        else {
            ++animals; aE += o->energy;
            if (o->name == "Wolf")  ++wolves;
            if (o->name == "Sheep") ++sheep;
        }
    }
    for (auto* e : envs) envE += e->energy;

    ImGui::Text("Frame %d/%d", frame, total); ImGui::SameLine(130);
    ImGui::Text("| Plants: %d", plants);       ImGui::SameLine(250);
    ImGui::Text("Animals: %d", animals);       ImGui::SameLine(370);
    ImGui::Text("W: %d  S: %d", wolves, sheep);ImGui::SameLine(490);
    ImGui::Text("| P-E: %.0f", pE);           ImGui::SameLine(600);
    ImGui::Text("A-E: %.0f", aE);             ImGui::SameLine(710);
    ImGui::Text("Env-E: %.0f", envE);
    ImGui::SameLine(830);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
}

// ============================================================
//  Main UI
// ============================================================
static bool showHeatmap  = false;
static bool showPlants   = true;
static bool showAnimals  = false;
static bool showQuery    = false;

void RenderUI(World& world, int* pFrame, int total,
              bool paused, bool* pPaused, bool* pStep, float* pSpeed, bool* pUnlimited,
              int* pMaxSteps)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Plants",      nullptr, &showPlants);
            ImGui::MenuItem("Animals",     nullptr, &showAnimals);
            ImGui::MenuItem("Heatmap",     nullptr, &showHeatmap);
            ImGui::MenuItem("Query Panel", nullptr, &showQuery);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // ---- controls ----
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button(*pPaused ? ">  Run" : "|| Pause"))
        *pPaused = !*pPaused;
    ImGui::SameLine();
    if (ImGui::Button("Step")) *pStep = true;
    ImGui::SameLine();
    if (ImGui::Button("Reset")) { world.Reset(); *pFrame = 0; }
    ImGui::SetNextItemWidth(120);
    ImGui::SliderFloat("Speed", pSpeed, 0.1f, 100.0f, "%.1fx");
    ImGui::SameLine();
    ImGui::Checkbox("No Limit", pUnlimited);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderInt("Batch", pMaxSteps, 1, 500);

    ImGui::Spacing();
    ImGui::Separator();
    DrawStats(world, *pFrame, total);
    ImGui::End();

    // ---- world grid ----
    ImGui::SetNextWindowPos(ImVec2(10, 80), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 520), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
    ImGui::Begin("World", nullptr, ImGuiWindowFlags_NoScrollbar);
    DrawWorldGrid(world);
    ImGui::End();
    ImGui::PopStyleVar();

    // ---- heatmap ----
    if (showHeatmap) {
        ImGui::SetNextWindowSize(ImVec2(290, 290), ImGuiCond_FirstUseEver);
        ImGui::Begin("Energy Heatmap", &showHeatmap);
        DrawEnergyHeatmap(world);
        ImGui::End();
    }

    // ---- plant list ----
    if (showPlants) {
        ImGui::SetNextWindowSize(ImVec2(260, 380), ImGuiCond_FirstUseEver);
        ImGui::Begin("Plants", &showPlants);
        DrawPlantList(world);
        ImGui::End();
    }

    // ---- animal list ----
    if (showAnimals) {
        ImGui::SetNextWindowSize(ImVec2(290, 380), ImGuiCond_FirstUseEver);
        ImGui::Begin("Animals", &showAnimals);
        DrawAnimalList(world);
        ImGui::End();
    }

    // ---- position query ----
    if (showQuery) {
        ImGui::SetNextWindowSize(ImVec2(260, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin("Position Query", &showQuery);
        static int qx = 0, qy = 0;
        ImGui::InputInt("X", &qx); ImGui::SameLine();
        ImGui::InputInt("Y", &qy);
        int w = world.GetWidth(), h = world.GetHeight();
        if (qx >= 0 && qx < w && qy >= 0 && qy < h) {
            const auto& envs = world.GetEnvironments();
            const auto& orgs = world.GetReproducas();
            int idx = qy*w + qx;
            ImGui::Separator();
            if (idx < (int)envs.size()) {
                ImGui::Text("Terrain: %s", EnvName(envs[idx]->name));
                ImGui::Text("Energy:  %.1f", envs[idx]->energy);
            }
            for (auto* org : orgs) {
                if (org->Pos.first == qx && org->Pos.second == qy) {
                    ImGui::Text("%s  E=%.1f",
                        OrganismDisplayName(org->name), org->energy);
                    if (org->type == PLANT)
                        ImGui::Text("  ID: %d", static_cast<const Plant*>(org)->id);
                    else
                        ImGui::Text("  ID: %d", static_cast<const Animal*>(org)->id);
                }
            }
        } else {
            ImGui::TextDisabled("Out of range [0..%d, 0..%d]", w-1, h-1);
        }
        ImGui::End();
    }
}

// ============================================================
//  Win32 + OpenGL setup
// ============================================================
static HGLRC   g_hRC;
static HDC     g_hDC;
static HWND    g_hWnd;
static bool    g_done = false;
static RECT    g_rect;

// Forward message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SIZE:
        GetClientRect(hWnd, &g_rect);
        return 0;
    case WM_DESTROY:
        g_done = true;
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static bool InitWindow() {
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"EcoSim";
    RegisterClassExW(&wc);

    g_rect = { 0, 0, 1280, 800 };
    AdjustWindowRect(&g_rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindowW(
        L"EcoSim", L"Eco Simulation",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_rect.right - g_rect.left, g_rect.bottom - g_rect.top,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hWnd) return false;

    // OpenGL pixel format
    PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0,0,0,0,0,0, 0,0, 0,0,0,0, 24,8,0,
        PFD_MAIN_PLANE, 0,0,0,0 };
    g_hDC = GetDC(g_hWnd);
    int pf = ChoosePixelFormat(g_hDC, &pfd);
    SetPixelFormat(g_hDC, pf, &pfd);
    g_hRC = wglCreateContext(g_hDC);
    wglMakeCurrent(g_hDC, g_hRC);

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    GetClientRect(g_hWnd, &g_rect);
    return true;
}

static void CleanupWindow() {
    if (g_hRC) { wglMakeCurrent(nullptr, nullptr); wglDeleteContext(g_hRC); }
    if (g_hDC && g_hWnd) ReleaseDC(g_hWnd, g_hDC);
    if (g_hWnd) DestroyWindow(g_hWnd);
    UnregisterClassW(L"EcoSim", GetModuleHandle(nullptr));
}

// ============================================================
//  main
// ============================================================
int main() {
    if (!InitWindow()) return 1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplOpenGL3_Init();

    World& world = World::GetWorld();
    world.CurrentWeather = SUN;

    const int totalFrames = 30000;
    int   frame    = 0;
    bool  paused   = false;
    bool  stepReq  = false;
    bool  unlimited = false;
    int   maxStepsPerFrame = 500;
    float speed    = 1.0f;
    auto lastStep = std::chrono::steady_clock::now();

    while (!g_done && frame < totalFrames) {
        // Pump messages
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) { g_done = true; break; }
        }
        if (g_done) break;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderUI(world, &frame, totalFrames, paused, &paused, &stepReq, &speed, &unlimited,
                  &maxStepsPerFrame);

        // Simulation step
        auto now = std::chrono::steady_clock::now();
        float stepInterval = 100.0f / speed;
        float elapsed = std::chrono::duration<float, std::milli>(now - lastStep).count();

        if (stepReq) {
            world.Update();
            ++frame;
            stepReq = false;
            lastStep = now;
        } else if (!paused) {
            if (unlimited) {
                for (int i = 0; i < maxStepsPerFrame && frame < totalFrames; ++i) {
                    world.Update();
                    ++frame;
                }
                lastStep = now;
            } else if (elapsed >= stepInterval) {
                world.Update();
                ++frame;
                lastStep = now;
            }
        }

        // Render
        ImGui::Render();
        glViewport(0, 0, (int)(g_rect.right - g_rect.left), (int)(g_rect.bottom - g_rect.top));
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(g_hDC);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupWindow();
    return 0;
}
