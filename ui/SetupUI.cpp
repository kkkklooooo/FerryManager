#include "SetupUI.h"
#include "Config.h"
#include "World.h"
#include"data/game_struct.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
std::unordered_map<std::string, ImVec4>OrganismColor;
static TestConfig s_GameConfig;
static gameData   s_WorldData;
// ---- helpers for editing string vectors as comma-separated text ----
static void VecToStr(const std::vector<std::string>& v, char* buf, size_t bufSize) {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ", ";
        s += v[i];
    }
    snprintf(buf, bufSize, "%s", s.c_str());
}

static void StrToVec(const char* buf, std::vector<std::string>& v) {
    v.clear();
    std::string s(buf);
    size_t pos = 0;
    while (pos < s.size()) {
        size_t end = s.find(',', pos);
        std::string tok = s.substr(pos, end - pos);
        // trim
        size_t start = tok.find_first_not_of(" \t");
        if (start != std::string::npos) {
            size_t last = tok.find_last_not_of(" \t");
            tok = tok.substr(start, last - start + 1);
            if (!tok.empty()) v.push_back(tok);
        }
        if (end == std::string::npos) break;
        pos = end + 1;
    }
}

// ----
static std::string remove_json_suffix(const std::string& filename) {
    const std::string suffix = ".json";
    if (filename.size() >= suffix.size() &&
        filename.rfind(suffix) == filename.size() - suffix.size()) {
        return filename.substr(0, filename.size() - suffix.size());
    }
    return filename;  // 没有后缀，原样返回
}

static std::string add_json_suffix(const std::string& filename) {
    const std::string suffix = ".json";
    if (filename.size() >= suffix.size() &&
        filename.rfind(suffix) == filename.size() - suffix.size()) {
        return filename;// 有后缀，原样返回
    }
    return filename+suffix;
}

static bool LoadConfig(TestConfig& cfg, const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    json j;
    j=json::parse(f);
    cfg = j.get<TestConfig>();
    //printf("%s\n\n", cfg.Default_Plant_Config.name.data());
    return true;
}

static bool LoadConfigAny(TestConfig& cfg) {
    // Try build dir first, then parent (project root)
    const char* paths[] = {
        "default_config.json",
        "../default_config.json",
        "../config/default_config.json",
        "../../config/default_config.json",
        "../../default_config.json"
    };
    for (auto p : paths) {
        if (LoadConfig(cfg, p)) return true;
    }
    return false;
}

static bool LoadWorld(gameData& gad, const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    json j;
    j = json::parse(f);
    gad = j.get<gameData>();
    return true;
}

static bool LoadWorldAny(gameData& gad) {
    const char* paths[] = {
          "../../../data/game_data.json",
          "../../data/game_data.json",
          "../data/game_data.json",            
          "data/game_data.json"
    };
    for (auto p : paths) {
        if (LoadWorld(gad, p)) return true;
    }
    return false;
}

static char g_StatusMsg[512] = "";

bool RunSetupPhase(HWND hWnd, bool& quitRequested) {
    bool loaded = LoadConfigAny(s_GameConfig);
    if (!loaded) {
        s_GameConfig = TestConfig();
        snprintf(g_StatusMsg, sizeof(g_StatusMsg),
            "WARNING: default_config.json not found - using empty defaults");
    } else {
        snprintf(g_StatusMsg, sizeof(g_StatusMsg),
            "Loaded default_config.json");
    }

    

    static int activeTab = 0;

    auto renderWorldTab = [&]() {
        ImGui::InputInt("宽度 (Width)",  &s_GameConfig.The_Word.width);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("世界横向格子数，最小10");
        ImGui::InputInt("长度 (Length)", &s_GameConfig.The_Word.length);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("世界纵向格子数，最小10");
        if (s_GameConfig.The_Word.width  < 10) s_GameConfig.The_Word.width  = 10;
        if (s_GameConfig.The_Word.length < 10) s_GameConfig.The_Word.length = 10;
    };



    auto renderEnvironmentsTab = [&]() {
        ImGui::TextDisabled("环境决定哪些物种可以在该地形上生存和繁殖");
        ImGui::Spacing();
        for (int i = 0; i < (int)s_GameConfig.The_Environments.size(); ++i) {
            auto& env = s_GameConfig.The_Environments[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char envNameBuf[128];
            snprintf(envNameBuf, sizeof(envNameBuf), "%s", env.name.c_str());
            if (ImGui::InputText("名称 (Name)", envNameBuf, sizeof(envNameBuf)))
                env.name = envNameBuf;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("环境类型名称，如 GressLand、Water");
            char buf[256];
            VecToStr(env.CanLive, buf, sizeof(buf));
            if (ImGui::InputText("可生存物种 (CanLive)", buf, sizeof(buf)))
                StrToVec(buf, env.CanLive);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("逗号分隔的物种名列表，只有在此列表中的物种才能在此环境繁殖");
            ImGui::PopID();
        }
    };

    auto renderAnimalsTab = [&]() {
        ImGui::TextDisabled("配置每种动物的参数，-1 表示使用默认值");
        ImGui::Spacing();
        for (int i = 0; i < (int)s_GameConfig.The_Animals.size(); ++i) {
            auto& a = s_GameConfig.The_Animals[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char animalNameBuf[128];
            snprintf(animalNameBuf, sizeof(animalNameBuf), "%s", a.name.c_str());
            if (ImGui::InputText("名称 (Name)", animalNameBuf, sizeof(animalNameBuf)))
                a.name = animalNameBuf;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("动物物种名,需与环境 CanLive 中的名称一致");
            char buf[256];
            VecToStr(a.diet, buf, sizeof(buf));
            if (ImGui::InputText("食谱 (Diet)", buf, sizeof(buf)))
                StrToVec(buf, a.diet);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("逗号分隔的食物列表,如 Gress,Sheep.只有在此列表中的物种才会被捕食");
            ImGui::InputInt("初始速度 (Init Speed)",    &a.reproduce_original_rate);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("动物出生时的移动速度,-1=使用默认动物配置");
            ImGui::InputInt("初始能量 (Init Energy)",  &a.reproduce_original_energy);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("动物出生时的能量值,-1=使用默认动物配置(18)");
            ImGui::InputFloat("最大速度 (Max Speed)",     &a.max_rate);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("速度上限,由能量*能量率计算的速度不会超过此值,-1=使用默认值");
            ImGui::InputFloat("每步消耗 (Step Cost)",  &a.step_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("每帧消耗的能量,-1=使用默认值(0.3)");
            ImGui::InputFloat("能量转化率 (Energy Rate)",  &a.energy_rate);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("能量到速度的转化系数: 速度=能量*此值,-1=使用默认值(0.2)");
            ImGui::InputFloat("繁殖阈值 (Repro Threshold)", &a.reproduce_energy_threshold);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("能量达到此值才能繁殖,-1=使用全局引擎默认值");
            ImGui::InputFloat("繁殖消耗 (Repro Cost)", &a.reproduce_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("每次繁殖消耗的能量,-1=使用全局引擎默认值");

            float col[4] = {1,1,1,1};
            auto it = OrganismColor.find(a.name);
            if (it != OrganismColor.end()) { col[0]=it->second.x; col[1]=it->second.y; col[2]=it->second.z; col[3]=it->second.w; }
            if (ImGui::ColorEdit4("颜色", col, ImGuiColorEditFlags_NoInputs))
                OrganismColor[a.name] = ImVec4(col[0], col[1], col[2], col[3]);

            ImGui::PopID();
        }
        if (ImGui::Button("添加物种 (Add Animal)")) {
            s_GameConfig.User_AddNew_Animal(*(new AnimalConfig));
        }
    };

    auto renderPlantsTab = [&]() {
        ImGui::TextDisabled("配置每种植物的参数,-1 表示使用默认值");
        ImGui::Spacing();
        for (int i = 0; i < (int)s_GameConfig.The_Plants.size(); ++i) {
            auto& p = s_GameConfig.The_Plants[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char plantSpNameBuf[128];
            snprintf(plantSpNameBuf, sizeof(plantSpNameBuf), "%s", p.name.c_str());
            if (ImGui::InputText("名称 (Name)", plantSpNameBuf, sizeof(plantSpNameBuf)))
                p.name = plantSpNameBuf;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("植物物种名,需与环境 CanLive 中的名称一致");
            ImGui::InputInt("初始能量 (Init Energy)", &p.reproduce_original_energy);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("植物出生时的能量值,-1=使用默认植物配置(5)");
            ImGui::InputFloat("每步消耗 (Step Cost)", &p.step_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("每帧消耗的能量,会被拥挤因子放大.-1=使用默认值(0.2)");
            ImGui::InputFloat("繁殖阈值 (Repro Threshold)", &p.reproduce_energy_threshold);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("能量达到此值才能繁殖.-1=使用全局引擎默认值");
            ImGui::InputFloat("繁殖消耗 (Repro Cost)", &p.reproduce_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("每次繁殖消耗的能量.-1=使用全局引擎默认值");

            float col[4] = {1,1,1,1};
            auto it = OrganismColor.find(p.name);
            if (it != OrganismColor.end()) { col[0]=it->second.x; col[1]=it->second.y; col[2]=it->second.z; col[3]=it->second.w; }
            if (ImGui::ColorEdit4("颜色", col, ImGuiColorEditFlags_NoInputs))
                OrganismColor[p.name] = ImVec4(col[0], col[1], col[2], col[3]);

            ImGui::PopID();
        }
        if (ImGui::Button("添加物种 (Add Plant)")) {
            s_GameConfig.User_AddNew_Plant(*(new PlantConfig));
        }
    };

    bool startRequested = false;

    auto renderCreateWorldTab = [&]() {
        static bool g_Create = false;
        bool finded = LoadWorldAny(s_WorldData);
        if (!finded) {
            s_WorldData = gameData();
            snprintf(g_StatusMsg, sizeof(g_StatusMsg), "WARNING: game_data.json not find - using empty defaults");
        }
        else
        {
            snprintf(g_StatusMsg, sizeof(g_StatusMsg),
                "Loaded game_data.json");
        }
        const char* paths[] = {
                "../../../data/game_data.json",
                "../../data/game_data.json",
                "../data/game_data.json",
                "data/game_data.json"
        };
        ImGui::TextDisabled("当前世界");
        ImGui::Spacing();
        for (auto i : s_WorldData.names) {
            ImGui::PushID(i.data());
            ImGui::Separator();
            std::string lable = remove_json_suffix(i);
            std::vector<std::string> path = {
                "../../../data/" + i,
                "../../data" + i,
                "data/" + i,
                i,
                "../" + i,
                "../data/" + i
            };
            if (ImGui::Button(lable.c_str())) {
                for (auto p : path) {
                    if (LoadConfig(s_GameConfig, p.c_str())) {
                        ImGui::PopID(); 
                        InitGameConfig(s_GameConfig);
                        World::GetWorld(s_GameConfig);
                        startRequested = true;
                        return;
                    }
                }
            }
            ImGui::PopID();
        }
        if (ImGui::Button(g_Create ? "不创造新世界（Don't Add New World)" : "创造新世界（Add New World)")) {
            g_Create = !g_Create;
        }
        if (g_Create) {//
            static char theName[32]="";
            ImGui::InputText("名称 (Name):", theName, sizeof(theName));
            if (ImGui::Button("Yes I will create this world")) {
                std::string fullName = add_json_suffix(theName);
                //printf("%s", theName);
                s_WorldData.names.push_back(fullName);
                json gameconfig = s_GameConfig;
                json AllGame = s_WorldData;//自动序列化
                fs::path P = "../../../data/game_data.json";
                std::ifstream f(P);
                if (f.is_open()) {
                    f.close();
                    std::ofstream F(P);
                    F << AllGame;
                    F.close();
                    fs::path dir = P.parent_path();
                    fs::path cur = dir / fullName;
                    std::ofstream FF(cur);
                    if (FF.is_open()) {
                        FF << gameconfig;
                        FF.close();
                    }
                    else
                    {
                        throw("can't Open!!");
                    }
                    InitGameConfig(s_GameConfig);
                    World::GetWorld(s_GameConfig);
                    startRequested = true;
                    }

            }
        }
    };

    while (!quitRequested && !startRequested) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) { quitRequested = true; break; }
        }
        if (quitRequested) break;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(440, 520), ImGuiCond_Once);
        ImGui::Begin("EcoSim Setup", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Configuration");
        ImGui::SameLine();
        if (!loaded)
            ImGui::TextColored(ImVec4(1, 0.4f, 0.2f, 1), "%s", g_StatusMsg);
        else
            ImGui::TextDisabled("%s", g_StatusMsg);

        if (ImGui::BeginTabBar("Tabs")) {
            if (ImGui::BeginTabItem("World"))      { activeTab = 0; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Environments")) { activeTab = 1; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Animals"))    { activeTab = 2; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Plants"))     { activeTab = 3; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("CreatWorlds")) { activeTab = 4; ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        ImGui::BeginChild("TabContent", ImVec2(0, -50), true);
        switch (activeTab) {
            case 0: renderWorldTab();        break;
            //case 1: renderDefaultsTab();     break;
            case 1: renderEnvironmentsTab(); break;
            case 2: renderAnimalsTab();      break;
            case 3: renderPlantsTab();       break;
            case 4:renderCreateWorldTab();   break;
        }
        ImGui::EndChild();

        if (ImGui::Button("Start Simulation", ImVec2(-1, 35))) {
            InitGameConfig(s_GameConfig);
            World::GetWorld(s_GameConfig);
            startRequested = true;
        }

        ImGui::End();

        ImGui::Render();
        RECT rect;
        GetClientRect(hWnd, &rect);
        glViewport(0, 0, (int)(rect.right - rect.left), (int)(rect.bottom - rect.top));
        glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(GetDC(hWnd));
    }

    return startRequested;
}
