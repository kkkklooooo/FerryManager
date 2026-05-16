#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../ui/SetupUI.h"
#include "../core/World.h"
#include "../core/Organism.h"
#include "../core/Environment.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include"../boids/Genes.h"
#include <GL/gl.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include<iostream>
#include<fstream>
#include <implot/implot.h>

using json = nlohmann::json;

// ============================================================
//  helper: environment -> colour
// ============================================================
static ImU32 EnvColor(const std::string& name, float energy, float maxE) { //保留
    float i = (maxE > 0.001f) ? energy / maxE : 0.5f;
    i = std::clamp(i, 0.25f, 1.0f);
    int r, g, b;
    if (name == "GressLand") { r = 107; g = 142; b = 35; }
    else if (name == "Water") { r = 30;  g = 144; b = 255; }
    else if (name == "Forest") { r = 34;  g = 139; b = 34; }
    else if (name == "Mountain") { r = 139; g = 137; b = 137; }
    else { r = 128; g = 128; b = 128; }
    return IM_COL32((int)(r * i), (int)(g * i), (int)(b * i), 255);
}

//感觉直接调用string的就可以了 啊
static const char* EnvName(const std::string& name) {//还没有呢 但是好像可以加
    if (name == "GressLand") return "Grassland";
    if (name == "Water")     return "Water";
    if (name == "Forest")    return "Forest";
    if (name == "Mountain")  return "Mountain";
    return name.c_str();
}

static const char* OrganismDisplayName(const std::string& name) {
    return name.c_str();
}

static std::unordered_map<std::string, ImVec4>OrganismColor;

// ============================================================
//  World grid
// ============================================================
static void DrawWorldGrid(const World& world, bool flat, bool showReq) {
    const auto& envs = world.GetEnvironments();
    const auto& orgs = world.GetReproducas();
    int w = world.GetWidth(), h = world.GetHeight();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float cellSize = std::min(avail.x / w, avail.y / h);
    cellSize =std::max(cellSize, 3.0f);

    float maxE = 0.001f;
    for (auto* e : envs) if (e->energy > maxE) maxE = e->energy;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 base = ImGui::GetCursorScreenPos();

    for (int y = 0; y < h; ++y) {//画环境的格子
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            ImVec2 p0(base.x + x * cellSize, base.y + y * cellSize);
            ImVec2 p1(p0.x + cellSize, p0.y + cellSize);

            ImU32 col;
            if (flat) {
                col = IM_COL32(35, 35, 38, 255);
            }
            else {
                col = IM_COL32(20, 20, 20, 255);
                if (idx < (int)envs.size())
                    col = EnvColor(envs[idx]->name, envs[idx]->energy, maxE);
            }

            dl->AddRectFilled(p0, p1, col);
            dl->AddRect(p0, p1, IM_COL32(55, 55, 58, 255), 0, 0, 1.0f);
        }
    }

    
    auto drawOrg = [&](Reproducable* org, bool flat) {
        ImVec4 Color = OrganismColor[org->name];
        int idx = org->Pos.second* w + org->Pos.first;
        ImVec2 p0(base.x + org->Pos.first * cellSize, base.y + org->Pos.second * cellSize);
        ImVec2 p1(p0.x + cellSize, p0.y + cellSize);
        ImU32 col;
        col = ImGui::ColorConvertFloat4ToU32(OrganismColor[org->name]);
        dl->AddRectFilled(p0, p1, col);
        dl->AddRect(p0, p1, IM_COL32(55, 55, 58, 255), 0, 0, 1.0f);
    };
    // plants first (bottom layer)
    for (auto* org : orgs) {
        if (org && org->type == PLANT) drawOrg(org, flat);
    }
    // animals on top
    for (auto* org : orgs) {
        if (org && org->type == ANIMAL) drawOrg(org, flat);
    }

    // reproduction request markers
    if (showReq) {
        const auto& requests = world.GetReproduceRequests();
        for (auto& req : requests) {
            int rx = req.pos.first, ry = req.pos.second;
            if (rx < 0 || rx >= w || ry < 0 || ry >= h) continue;
            ImVec2 c(base.x + rx * cellSize + cellSize * 0.5f,
                base.y + ry * cellSize + cellSize * 0.5f);
            float r = cellSize * 0.25f;
            ImU32 col = req.type == PLANT ? IM_COL32(0, 255, 100, 220) : IM_COL32(255, 220, 50, 220);
            // draw X marker
            float s = r * 0.7f;
            dl->AddLine(ImVec2(c.x - s, c.y - s), ImVec2(c.x + s, c.y + s), col, 2.0f);
            dl->AddLine(ImVec2(c.x + s, c.y - s), ImVec2(c.x - s, c.y + s), col, 2.0f);
        }
    }

    ImGui::Dummy(ImVec2(w * cellSize, h * cellSize));

    if (ImGui::IsItemHovered()) {//展示环境的
        ImVec2 m = ImGui::GetMousePos();
        int gx = (int)((m.x - base.x) / cellSize);
        int gy = (int)((m.y - base.y) / cellSize);
        if (gx >= 0 && gx < w && gy >= 0 && gy < h) {
            int idx = gy * w + gx;
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
// Organism statistics
// ============================================================
static std::unordered_map<std::string,std::vector<float>>Organism_hestory;

static void ResetPopulationHistory() {
    Organism_hestory.clear();
}


static void DrawPopulationHistory() {
    ImGui::SetNextWindowSize(ImVec2(900, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Plot");
    long long int count = Organism_hestory["&&ALLORGANISM&&"].size();
    //printf("%d ", count);
    if (count < 3) {
        ImGui::TextDisabled("Waiting for data...");
        ImGui::End();
        return;
    }
    if (count > 5000) {
        ResetPopulationHistory();
        ImGui::End();
        return;
    }
    float yMax = Organism_hestory["&&ALLORGANISM&&"][count-1];
    yMax = std::max(yMax * 1.15f, 10.0f);

    if (ImPlot::BeginPlot("##PopPlot", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Frame", "Count");
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)(count + 10), ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, (double)yMax, ImGuiCond_Always);
        for (auto i : Organism_hestory) {
            if (i.first == "&&ALLORGANISM&&")continue;
            std::string na = i.first;
            ImGui::PushID(na.data());
            ImPlot::PlotLine(na.data(), Organism_hestory[na].data(),count);
            ImGui::PopID();
        }
        ImPlot::EndPlot();
    }
    ImGui::End();
}


// ============================================================
//  Plant table
// ============================================================
static void DrawPlantList(const World& world) {//保留
    const auto& orgs = world.GetReproducas();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (!ImGui::BeginTable("##plants", 5,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit,
        ImVec2(0, avail.y)))
        return;
    
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Energy", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableHeadersRow();

    static const ImVec4 high_energy(0.3f, 1.0f, 0.3f, 1.0f);
    static const ImVec4 low_energy(1.0f, 0.5f, 0.3f, 1.0f);

    for (auto* org : orgs) {
        if (!org) continue;
        if (org->type != PLANT) continue;
        const Plant* p = static_cast<const Plant*>(org);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("%d", p->id);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%s", p->name.data());
        ImGui::TableSetColumnIndex(2); ImGui::Text("%d", (int)org->Pos.first);
        ImGui::TableSetColumnIndex(3); ImGui::Text("%d", (int)org->Pos.second);
        ImGui::TableSetColumnIndex(4);
        ImVec4 c = org->energy > 5.0f ? high_energy : low_energy;
        ImGui::TextColored(c, "%.1f", org->energy);
    }
    ImGui::EndTable();
}

// ============================================================
//  Animal table
// ============================================================
static void DrawAnimalList(const World& world) {
    const auto& orgs = world.GetReproducas();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (!ImGui::BeginTable("##animals", 6,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit,
        ImVec2(0, avail.y)))
        return;

    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60);
    ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 40);
    ImGui::TableSetupColumn("Energy", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Repro", ImGuiTableColumnFlags_WidthFixed, 50);
    ImGui::TableHeadersRow();

    for (auto* org : orgs) {
        if (org->type == PLANT) continue;
        ImGui::TableNextRow();
        const Animal* a = static_cast<const Animal*>(org);
        ImGui::TableSetColumnIndex(0); ImGui::Text("%d", a->id);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%s", org->name.data());
        ImGui::TableSetColumnIndex(2); ImGui::Text("%d", (int)org->Pos.first);
        ImGui::TableSetColumnIndex(3); ImGui::Text("%d", (int)org->Pos.second);
        ImGui::TableSetColumnIndex(4);
        ImVec4 c = org->energy > 15.0f ? ImVec4(0.3f, 1, 0.3f, 1) : ImVec4(1, 0.5f, 0.3f, 1);
        ImGui::TextColored(c, "%.1f", org->energy);
        ImGui::TableSetColumnIndex(5);
        ImVec4 rc = org->reproduce_able ? ImVec4(0.3f, 1, 0.3f, 1) : ImVec4(0.6f, 0.6f, 0.6f, 1);
        ImGui::TextColored(rc, "%s", org->reproduce_able ? "Y" : "N");
    }
    ImGui::EndTable();
}

// ============================================================
//  Stats bar
// ============================================================
static void DrawStats(const World& world, int frame, int total,bool persue,bool stepReq) {
    const auto& orgs = world.GetReproducas();
    const auto& envs = world.GetEnvironments();
    static std::unordered_map<std::string, int>size;
    int plants=0 ,animals=0,Organisms=0;
    float pE = 0, aE = 0, envE = 0;
    size.clear();
    for (auto* o : orgs) {

        if (!o) continue;

        if (o->type == PLANT) {
            ++plants;  
            pE += o->energy; 
        }
        else {
            ++animals; aE += o->energy;
             
        }
        size[o->name]++;
    }
    Organisms = plants + animals;
    if (!persue || stepReq) { 
        //printf("111");
        Organism_hestory["&&ALLORGANISM&&"].push_back(Organisms); 
    }
    for (auto* e : envs) envE += e->energy;

    ImGui::Text("|Frame %d/%d", frame, total); ImGui::SameLine(130);
    ImGui::Text("| Plants: %d", plants);       ImGui::SameLine(250);
    ImGui::Text("Animals: %d", animals);       ImGui::SameLine(370);
    ImGui::Text("| P-E: %.0f", pE);           ImGui::SameLine(600);
    ImGui::Text("A-E: %.0f", aE);             ImGui::SameLine(710);
    ImGui::Text("Env-E: %.0f|", envE);
    for (auto O : size) {
        ImGui::Text("%s: %d", O.first.data(), O.second);
        if(!persue || stepReq)Organism_hestory[O.first].push_back(O.second);
    }
    DrawPopulationHistory();
    ImGui::SameLine(830);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
}


static void DrawAddList(World& world) {
    const auto& AllAnaimals = world.game_conf.The_Animals;
    const auto& AllPlants = world.game_conf.The_Plants;
    static int x=0;
    static int y=0;
    ImGui::SliderInt("x", &x, 0, world.GetWidth()-1);
    ImGui::SliderInt("y", &y, 0, world.GetHeight()-1);
    ImGui::BeginListBox("Organism", ImVec2(0, 150));
    static int selected = 0;
    int id = 0;
    for (auto TheAnimal : AllAnaimals) {
        ImGui::RadioButton(TheAnimal.name.data(), &selected,id);
        id++;
    }
    for (auto ThePlant : AllPlants) {
        ImGui::RadioButton(ThePlant.name.data(), &selected,id);
        id++;
    }
    ImGui::EndListBox();
    if (AllAnaimals.size() > selected) {
        static float cohesion;
        static float alignment;
        static float separation;
        static float vision;
        static bool change = false;
        ImGui::Checkbox("Set Gen", &change);
        if (change) {
            ImGui::SliderFloat("cohesion", &cohesion, 0.0f, 2.0f);
            ImGui::SliderFloat("alignment", &alignment, 0.0f, 2.0f);
            ImGui::SliderFloat("separation", &separation, 0.0f, 2.0f);
            ImGui::SliderFloat("vision", &vision, 3.0f, 12.0f);
        }
        if (ImGui::Button("Confirm")) {
            if (change)
            {
                world.AddReproduceRequest({ ANIMAL,AllAnaimals[selected].name,std::make_pair(x,y),world.conf.Organism_interact_radius, {{cohesion, alignment, separation, vision}} });
            }
            else
            {
                world.AddReproduceRequest({ ANIMAL,AllAnaimals[selected].name,std::make_pair(x,y),world.conf.Organism_interact_radius,std::make_optional<boids::Genes>()});
            }
        }
    }
    else
    {
        if (ImGui::Button("Confirm")) {
           // printf("%d", selected - id);
           world.AddReproduceRequest({ PLANT,AllPlants[1+selected - id].name,std::make_pair(x,y),world.conf.Plant_init_radius });
        }
    }
}


// ============================================================
//  Main UI
// ============================================================
static bool showHeatmap = false;
static bool showPlants = true;
static bool showAnimals = false;
static bool showQuery = false;
static bool flatColors = true;
static bool showRequests = true;
static bool showAddList = false;
static bool showColorSet = true;
void RenderUI(World& world, int* pFrame, int total,
    bool paused, bool* pPaused, bool* pStep, float* pSpeed, bool* pUnlimited,
    int* pMaxSteps)
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Plants", nullptr, &showPlants);
            ImGui::MenuItem("Animals", nullptr, &showAnimals);
            //ImGui::MenuItem("Heatmap", nullptr, &showHeatmap);
            ImGui::MenuItem("Query Panel", nullptr, &showQuery);//搜索面板
            ImGui::MenuItem("ColorSet", nullptr, &showColorSet);
            ImGui::Separator();
            ImGui::MenuItem("Flat Colors", nullptr, &flatColors);
            ImGui::MenuItem("Repro Requests", nullptr, &showRequests);
            ImGui::Separator();
            ImGui::MenuItem("AddNewOrganism", nullptr, &showAddList);
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
    ImGui::Checkbox("Flat Colors", &flatColors);
    ImGui::Separator();
    DrawStats(world, *pFrame, total, *pPaused,*pStep);
    ImGui::End();

    // ---- world grid ----
    ImGui::SetNextWindowPos(ImVec2(10, 80), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 520), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
    ImGui::Begin("World", nullptr, ImGuiWindowFlags_NoScrollbar);
    DrawWorldGrid(world, flatColors, showRequests);
    ImGui::End();
    ImGui::PopStyleVar();
    // ---- reproduce requests grid ----

    // ---- heatmap ----
    // TODO
    //if (showHeatmap) {
    //    ImGui::SetNextWindowSize(ImVec2(290, 290), ImGuiCond_FirstUseEver);
    //    ImGui::Begin("Energy Heatmap", &showHeatmap);
    //    DrawEnergyHeatmap(world);
    //    ImGui::End();
    //}

    // ---- plant list ----
    if (showPlants) {
        ImGui::SetNextWindowSize(ImVec2(260, 380), ImGuiCond_FirstUseEver);
        ImGui::Begin("Plants", &showPlants);
        DrawPlantList(world);
        ImGui::End();
    }

    // ---- animal list ----
    if (showAnimals) {
        ImGui::SetNextWindowSize(ImVec2(340, 420), ImGuiCond_FirstUseEver);
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
            int idx = qy * w + qx;
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
        }
        else {
            ImGui::TextDisabled("Out of range [0..%d, 0..%d]", w - 1, h - 1);
        }
        ImGui::End();
    }

    if (showAddList) {
        ImGui::SetNextWindowSize(ImVec2(260, 380), ImGuiCond_FirstUseEver);
        ImGui::Begin("Add new Organsim");
        DrawAddList(world);
        ImGui::End();
    }

    if (showColorSet) {
        //设置每个动植物的颜色的区域
        const auto& AllAnaimals = world.game_conf.The_Animals;
        const auto& AllPlants = world.game_conf.The_Plants;

        static float rab_color[4] = { 1.0f,1.0f,1.0f,1.0f };
        ImGui::Begin("OrganismColor");
        if (ImGui::BeginTable("##Color", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit, ImVec2(0, 0))) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("Current", ImGuiTableColumnFlags_WidthFixed, 250);
            for (auto TheAnimal : AllAnaimals) {
                ImGui::PushID(TheAnimal.name.c_str());
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", TheAnimal.name.data());
                ImGui::TableSetColumnIndex(1);
                if (ImGui::ColorEdit4("##color", rab_color,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
                {
                    // 颜色被修改后，更新映射表
                    OrganismColor[TheAnimal.name] = ImVec4(rab_color[0], rab_color[1],
                        rab_color[2], rab_color[3]);
                }
                ImGui::TableSetColumnIndex(2); ImGui::Text("%f %f %f %f", OrganismColor[TheAnimal.name].x, OrganismColor[TheAnimal.name].y,
                    OrganismColor[TheAnimal.name].z, OrganismColor[TheAnimal.name].w);
                ImGui::PopID();
            }
            for (auto ThePlant : AllPlants) {
                ImGui::PushID(ThePlant.name.c_str());
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", ThePlant.name.data());
                ImGui::TableSetColumnIndex(1);
                if (ImGui::ColorEdit4("##color", rab_color,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
                {
                    // 颜色被修改后，更新映射表
                    OrganismColor[ThePlant.name] = ImVec4(rab_color[0], rab_color[1],
                        rab_color[2], rab_color[3]);
                }
                ImGui::TableSetColumnIndex(2); ImGui::Text("%f %f %f %f", OrganismColor[ThePlant.name].x, OrganismColor[ThePlant.name].y,
                    OrganismColor[ThePlant.name].z, OrganismColor[ThePlant.name].w);
                ImGui::PopID();
            }
            ImGui::EndTable();
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
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
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

static  Config conf = Config::GetConfig();

int main() {
    if (!InitWindow()) return 1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplOpenGL3_Init();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 16.0f,
        nullptr, io.Fonts->GetGlyphRangesChineseFull());

    if (!RunSetupPhase(g_hWnd, g_done)) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        ImPlot::DestroyContext();
        CleanupWindow();
        return 0;
    }

    World& world = World::GetWorld();
    world.CurrentWeather = SUN;

    const int totalFrames = 30000;
    int   frame = 0;
    bool  paused = false;
    bool  stepReq = false;
    bool  unlimited = false;
    int   maxStepsPerFrame = 500;
    float speed = 1.0f;
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
        }
        else if (!paused) {
            if (unlimited) {
                for (int i = 0; i < maxStepsPerFrame && frame < totalFrames; ++i) {
                    world.Update();
                    ++frame;
                }
                lastStep = now;
            }
            else if (elapsed >= stepInterval) {
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
    ImPlot::DestroyContext();
    CleanupWindow();
    return 0;
}
