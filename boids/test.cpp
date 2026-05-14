#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "boids.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ============================================================
//  update flock using boids::ComputeFinalForce from boids.h
// ============================================================
static void UpdateFlock(std::vector<boids::Particle>& flock, int w, int h, float dt)
{
    for (size_t i = 0; i < flock.size(); ++i) {
        // build neighbor list excluding self
        std::vector<boids::Particle> neighbors;
        neighbors.reserve(flock.size() - 1);
        for (size_t j = 0; j < flock.size(); ++j) {
            if (j != i) neighbors.push_back(flock[j]);
        }

        auto [fx, fy] = boids::ComputeFinalForce(flock[i], neighbors);

        flock[i].vx += fx * dt;
        flock[i].vy += fy * dt;

        float spd = std::sqrt(flock[i].vx*flock[i].vx + flock[i].vy*flock[i].vy);
        if (spd > flock[i].speed) {
            flock[i].vx = flock[i].vx / spd * flock[i].speed;
            flock[i].vy = flock[i].vy / spd * flock[i].speed;
        }

        flock[i].x += flock[i].vx * dt;
        flock[i].y += flock[i].vy * dt;

        // toroidal wrap
        if (flock[i].x < 0)          flock[i].x += (float)w;
        if (flock[i].x >= (float)w)  flock[i].x -= (float)w;
        if (flock[i].y < 0)          flock[i].y += (float)h;
        if (flock[i].y >= (float)h)  flock[i].y -= (float)h;
    }
}

// ============================================================
//  globals
// ============================================================
static std::vector<boids::Particle> g_flock;
static int   g_worldW = 80;
static int   g_worldH = 60;
static float g_speed  = 3.0f;
static float g_dt     = 0.15f;
static int   g_frame  = 0;
static bool  g_paused = false;
static bool  g_step   = false;

// shared genes (modifiable via sliders)
static float g_cohesion   = 1.0f;
static float g_alignment  = 0.5f;
static float g_separation = 1.0f;
static float g_vision     = 3.0f;

// ============================================================
//  init flock
// ============================================================
static void InitFlock(int nWolves, int nSheep) {
    g_flock.clear();
    srand((unsigned)time(nullptr));

    auto spawn = [](int species, int count) {
        for (int i = 0; i < count; ++i) {
            boids::Particle p;
            p.x = (float)(rand() % (g_worldW * 10)) / 10.0f;
            p.y = (float)(rand() % (g_worldH * 10)) / 10.0f;
            float ang = (float)rand() / RAND_MAX * 6.283185f;
            p.vx = std::cos(ang) * g_speed * 0.5f;
            p.vy = std::sin(ang) * g_speed * 0.5f;
            p.speed = g_speed;
            p.species = species;
            p.genes.cohesion   = g_cohesion;
            p.genes.alignment  = g_alignment;
            p.genes.separation = g_separation;
            p.genes.vision     = g_vision;
            g_flock.push_back(p);
        }
    };
    spawn(0, nWolves);  // Wolf  = red
    spawn(1, nSheep);   // Sheep = blue
}

static void SyncGenes() {
    for (auto& p : g_flock) {
        p.genes.cohesion   = g_cohesion;
        p.genes.alignment  = g_alignment;
        p.genes.separation = g_separation;
        p.genes.vision     = g_vision;
        p.speed = g_speed;
    }
}

// ============================================================
//  render
// ============================================================
static void RenderBoidsUI() {
    // -- controls --
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Boids Controls", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button(g_paused ? "> Run" : "|| Pause"))
        g_paused = !g_paused;
    ImGui::SameLine();
    if (ImGui::Button("Step")) g_step = true;
    ImGui::SameLine();
    if (ImGui::Button("Reset")) { InitFlock(30, 50); g_frame = 0; }

    ImGui::SameLine();
    ImGui::Text("Frame: %d", g_frame);

    bool genesChanged = false;
    genesChanged |= ImGui::SliderFloat("Cohesion",   &g_cohesion,   0.0f, 3.0f, "%.2f");
    genesChanged |= ImGui::SliderFloat("Alignment",  &g_alignment,  0.0f, 3.0f, "%.2f");
    genesChanged |= ImGui::SliderFloat("Separation", &g_separation, 0.0f, 3.0f, "%.2f");
    genesChanged |= ImGui::SliderFloat("Vision",     &g_vision,     1.0f, 15.0f, "%.1f");
    ImGui::SliderFloat("Speed", &g_speed, 0.5f, 10.0f, "%.1f");
    ImGui::SliderFloat("dt",    &g_dt,    0.01f, 0.5f, "%.2f");

    if (genesChanged) SyncGenes();

    int wolves = 0, sheep = 0;
    for (auto& p : g_flock) {
        if (p.species == 0) wolves++; else sheep++;
    }
    ImGui::Text("Wolves: %d  |  Sheep: %d  |  Total: %d", wolves, sheep, (int)g_flock.size());
    ImGui::End();

    // -- world grid --
    ImGui::SetNextWindowPos(ImVec2(10, 180), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(820, 640), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::Begin("Boids World", nullptr, ImGuiWindowFlags_NoScrollbar);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float cellSize = std::min(avail.x / g_worldW, avail.y / g_worldH);
    cellSize = std::max(cellSize, 4.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 base = ImGui::GetCursorScreenPos();

    // background grid
    for (int y = 0; y < g_worldH; ++y) {
        for (int x = 0; x < g_worldW; ++x) {
            ImVec2 p0(base.x + x*cellSize, base.y + y*cellSize);
            ImVec2 p1(p0.x + cellSize, p0.y + cellSize);
            dl->AddRectFilled(p0, p1, IM_COL32(18, 18, 22, 255));
            dl->AddRect(p0, p1, IM_COL32(45, 45, 55, 255), 0, 0, 0.5f);
        }
    }

    // draw boids — float coords cast directly to grid cells
    for (auto& p : g_flock) {
        int gx = (int)p.x;
        int gy = (int)p.y;
        if (gx < 0 || gx >= g_worldW || gy < 0 || gy >= g_worldH) continue;

        // snap to grid center — discrete cell-aligned display
        ImVec2 c(base.x + (gx + 0.5f) * cellSize,
                 base.y + (gy + 0.5f) * cellSize);
        float r = cellSize * 0.35f;

        ImU32 col = (p.species == 0)
            ? IM_COL32(255, 80, 60, 220)   // wolf  = red
            : IM_COL32(60, 140, 255, 220); // sheep = blue

        dl->AddCircleFilled(c, r, col);
        dl->AddCircle(c, r, IM_COL32(0, 0, 0, 100), 0, 1.0f);

        // velocity direction line
        float spd = std::sqrt(p.vx*p.vx + p.vy*p.vy);
        if (spd > 0.01f) {
            ImVec2 dir(c.x + p.vx/spd * r * 1.8f, c.y + p.vy/spd * r * 1.8f);
            dl->AddLine(c, dir, IM_COL32(255, 255, 255, 120), 1.2f);
        }
    }

    ImGui::Dummy(ImVec2(g_worldW * cellSize, g_worldH * cellSize));

    // hover tooltip
    if (ImGui::IsItemHovered()) {
        ImVec2 m = ImGui::GetMousePos();
        int gx = (int)((m.x - base.x) / cellSize);
        int gy = (int)((m.y - base.y) / cellSize);
        if (gx >= 0 && gx < g_worldW && gy >= 0 && gy < g_worldH) {
            ImGui::BeginTooltip();
            ImGui::Text("Grid: (%d, %d)", gx, gy);
            int n = 0;
            for (auto& p : g_flock) {
                if ((int)p.x == gx && (int)p.y == gy) {
                    ImGui::Text("  %s  v=(%.2f,%.2f)",
                        p.species == 0 ? "Wolf" : "Sheep", p.vx, p.vy);
                    n++;
                }
            }
            if (n == 0) ImGui::Text("  (empty)");
            ImGui::EndTooltip();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

// ============================================================
//  Win32 + OpenGL boilerplate
// ============================================================
static HGLRC g_hRC;
static HDC   g_hDC;
static HWND  g_hWnd;
static bool  g_done = false;
static RECT  g_rect;

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
    wc.lpszClassName = L"BoidsTest";
    RegisterClassExW(&wc);

    g_rect = { 0, 0, 1100, 850 };
    AdjustWindowRect(&g_rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindowW(
        L"BoidsTest", L"Boids Algorithm Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_rect.right - g_rect.left, g_rect.bottom - g_rect.top,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hWnd) return false;

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
    UnregisterClassW(L"BoidsTest", GetModuleHandle(nullptr));
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

    InitFlock(30, 50);

    while (!g_done) {
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

        RenderBoidsUI();

        // simulation step
        if (g_step) {
            UpdateFlock(g_flock, g_worldW, g_worldH, g_dt);
            g_frame++;
            g_step = false;
        } else if (!g_paused) {
            UpdateFlock(g_flock, g_worldW, g_worldH, g_dt);
            g_frame++;
        }

        ImGui::Render();
        glViewport(0, 0, (int)(g_rect.right - g_rect.left), (int)(g_rect.bottom - g_rect.top));
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
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
