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
    return filename;  // 娌℃湁鍚庣紑锛屽師鏍疯繑鍥?
}

static std::string add_json_suffix(const std::string& filename) {
    const std::string suffix = ".json";
    if (filename.size() >= suffix.size() &&
        filename.rfind(suffix) == filename.size() - suffix.size()) {
        return filename;// 鏈夊悗缂€锛屽師鏍疯繑鍥?
    }
    return filename + suffix;
}

static bool LoadConfig(TestConfig& cfg, const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    json j;
    j = json::parse(f);
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
        "../../default_config.json",
        ".../config/default_config.jason",//?????·?????
        "./config/default_config.json"
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
    }
    else {
        snprintf(g_StatusMsg, sizeof(g_StatusMsg),
            "Loaded default_config.json");
    }



    static int activeTab = 0;

    auto renderWorldTab = [&]() {
        ImGui::InputInt("瀹藉害 (Width)", &s_GameConfig.The_Word.width);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("涓栫晫妯?鍚戞牸瀛愭暟锛屾渶灏?10");
        ImGui::InputInt("闀垮害 (Length)", &s_GameConfig.The_Word.length);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("涓栫晫绾靛悜鏍煎瓙鏁帮紝鏈€灏?10");
        if (s_GameConfig.The_Word.width < 10) s_GameConfig.The_Word.width = 10;
        if (s_GameConfig.The_Word.length < 10) s_GameConfig.The_Word.length = 10;
        };



    auto renderEnvironmentsTab = [&]() {
        ImGui::TextDisabled("鐜?澧冨喅瀹氬摢浜涚墿绉嶅彲浠ュ湪璇ュ湴褰?涓婄敓瀛樺拰绻佹畺");
        ImGui::Spacing();
        for (int i = 0; i < (int)s_GameConfig.The_Environments.size(); ++i) {
            auto& env = s_GameConfig.The_Environments[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char envNameBuf[128];
            snprintf(envNameBuf, sizeof(envNameBuf), "%s", env.name.c_str());
            if (ImGui::InputText("鍚嶇О (Name)", envNameBuf, sizeof(envNameBuf)))
                env.name = envNameBuf;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鐜?澧冪被鍨嬪悕绉帮紝濡? GressLand銆乄ater");
            char buf[256];
            VecToStr(env.CanLive, buf, sizeof(buf));
            if (ImGui::InputText("鍙?鐢熷瓨鐗╃?? (CanLive)", buf, sizeof(buf)))
                StrToVec(buf, env.CanLive);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("閫楀彿鍒嗛殧鐨勭墿绉嶅悕鍒楄〃锛屽彧鏈夊湪姝ゅ垪琛ㄤ腑鐨勭墿绉嶆墠鑳藉湪姝ょ幆澧冪箒娈?");
            ImGui::PopID();
        }
        };

    auto renderAnimalsTab = [&]() {
        ImGui::TextDisabled("閰嶇疆姣忕?嶅姩鐗╃殑鍙傛暟锛?-1 琛ㄧず浣跨敤榛樿?ゅ€?");
        ImGui::Spacing();
        for (int i = 0; i < (int)s_GameConfig.The_Animals.size(); ++i) {
            auto& a = s_GameConfig.The_Animals[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char animalNameBuf[128];
            snprintf(animalNameBuf, sizeof(animalNameBuf), "%s", a.name.c_str());
            if (ImGui::InputText("鍚嶇О (Name)", animalNameBuf, sizeof(animalNameBuf)))
                a.name = animalNameBuf;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鍔ㄧ墿鐗╃?嶅悕,闇€涓庣幆澧? CanLive 涓?鐨勫悕绉颁竴鑷?");
            char buf[256];
            VecToStr(a.diet, buf, sizeof(buf));
            if (ImGui::InputText("椋熻氨 (Diet)", buf, sizeof(buf)))
                StrToVec(buf, a.diet);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("閫楀彿鍒嗛殧鐨勯?熺墿鍒楄〃,濡? Gress,Sheep.鍙?鏈夊湪姝ゅ垪琛ㄤ腑鐨勭墿绉嶆墠浼氳??鎹曢??");
            ImGui::InputInt("鍒濆?嬮€熷害 (Init Speed)", &a.reproduce_original_rate);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鍔ㄧ墿鍑虹敓鏃剁殑绉诲姩閫熷害,-1=浣跨敤榛樿?ゅ姩鐗╅厤缃?");
            ImGui::InputInt("鍒濆?嬭兘閲? (Init Energy)", &a.reproduce_original_energy);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鍔ㄧ墿鍑虹敓鏃剁殑鑳介噺鍊?,-1=浣跨敤榛樿?ゅ姩鐗╅厤缃?(18)");
            ImGui::InputFloat("鏈€澶ч€熷害 (Max Speed)", &a.max_rate);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("閫熷害涓婇檺,鐢辫兘閲?*鑳介噺鐜囪?＄畻鐨勯€熷害涓嶄細瓒呰繃姝ゅ€?,-1=浣跨敤榛樿?ゅ€?");
            ImGui::InputFloat("姣忔?ユ秷鑰? (Step Cost)", &a.step_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("姣忓抚娑堣€楃殑鑳介噺,-1=浣跨敤榛樿?ゅ€?(0.3)");
            ImGui::InputInt("鏈€澶ц兘閲? (Max Energy)", &a.max_energy);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鑳介噺涓婇檺锛屽悆楗卞悗涓嶅啀鎹曢??,-1=浣跨敤榛樿?ゅ€?(50)");
            ImGui::InputFloat("鑳介噺杞?鍖栫巼 (Energy Rate)", &a.energy_rate);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鑳介噺鍒伴€熷害鐨勮浆鍖栫郴鏁?: 閫熷害=鑳介噺*姝ゅ€?,-1=浣跨敤榛樿?ゅ€?(0.2)");
            ImGui::InputFloat("绻佹畺闃堝€? (Repro Threshold)", &a.reproduce_energy_threshold);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鑳介噺杈惧埌姝ゅ€兼墠鑳界箒娈?,-1=浣跨敤鍏ㄥ眬寮曟搸榛樿?ゅ€?");
            ImGui::InputFloat("绻佹畺娑堣€? (Repro Cost)", &a.reproduce_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("姣忔?＄箒娈栨秷鑰楃殑鑳介噺,-1=浣跨敤鍏ㄥ眬寮曟搸榛樿?ゅ€?");

            float col[4] = { 1,1,1,1 };
            auto it = OrganismColor.find(a.name);
            if (it != OrganismColor.end()) { col[0] = it->second.x; col[1] = it->second.y; col[2] = it->second.z; col[3] = it->second.w; }
            if (ImGui::ColorEdit4("棰滆壊", col, ImGuiColorEditFlags_NoInputs))
                OrganismColor[a.name] = ImVec4(col[0], col[1], col[2], col[3]);

            ImGui::PopID();
        }
        if (ImGui::Button("娣诲姞鐗╃?? (Add Animal)")) {
            s_GameConfig.User_AddNew_Animal(*(new AnimalConfig));
        }
        };

    auto renderPlantsTab = [&]() {
        ImGui::TextDisabled("閰嶇疆姣忕?嶆?嶇墿鐨勫弬鏁?,-1 琛ㄧず浣跨敤榛樿?ゅ€?");
        ImGui::Spacing();
        for (int i = 0; i < (int)s_GameConfig.The_Plants.size(); ++i) {
            auto& p = s_GameConfig.The_Plants[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char plantSpNameBuf[128];
            snprintf(plantSpNameBuf, sizeof(plantSpNameBuf), "%s", p.name.c_str());
            if (ImGui::InputText("鍚嶇О (Name)", plantSpNameBuf, sizeof(plantSpNameBuf)))
                p.name = plantSpNameBuf;
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("妞嶇墿鐗╃?嶅悕,闇€涓庣幆澧? CanLive 涓?鐨勫悕绉颁竴鑷?");
            ImGui::InputInt("鍒濆?嬭兘閲? (Init Energy)", &p.reproduce_original_energy);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("妞嶇墿鍑虹敓鏃剁殑鑳介噺鍊?,-1=浣跨敤榛樿?ゆ?嶇墿閰嶇疆(5)");
            ImGui::InputFloat("姣忔?ユ秷鑰? (Step Cost)", &p.step_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("姣忓抚娑堣€楃殑鑳介噺,浼氳??鎷ユ尋鍥犲瓙鏀惧ぇ.-1=浣跨敤榛樿?ゅ€?(0.2)");
            ImGui::InputFloat("绻佹畺闃堝€? (Repro Threshold)", &p.reproduce_energy_threshold);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("鑳介噺杈惧埌姝ゅ€兼墠鑳界箒娈?.-1=浣跨敤鍏ㄥ眬寮曟搸榛樿?ゅ€?");
            ImGui::InputFloat("绻佹畺娑堣€? (Repro Cost)", &p.reproduce_energy_cost);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("姣忔?＄箒娈栨秷鑰楃殑鑳介噺.-1=浣跨敤鍏ㄥ眬寮曟搸榛樿?ゅ€?");

            float col[4] = { 1,1,1,1 };
            auto it = OrganismColor.find(p.name);
            if (it != OrganismColor.end()) { col[0] = it->second.x; col[1] = it->second.y; col[2] = it->second.z; col[3] = it->second.w; }
            if (ImGui::ColorEdit4("棰滆壊", col, ImGuiColorEditFlags_NoInputs))
                OrganismColor[p.name] = ImVec4(col[0], col[1], col[2], col[3]);

            ImGui::PopID();
        }
        if (ImGui::Button("娣诲姞鐗╃?? (Add Plant)")) {
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
        ImGui::TextDisabled("褰撳墠涓栫晫");
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
        if (ImGui::Button(g_Create ? "涓嶅垱閫犳柊涓栫晫锛圖on't Add New World)" : "鍒涢€犳柊涓栫晫锛圓dd New World)")) {
            g_Create = !g_Create;
        }
        if (g_Create) {//
            static char theName[32] = "";
            ImGui::InputText("鍚嶇О (Name):", theName, sizeof(theName));
            if (ImGui::Button("Yes I will create this world")) {
                std::string fullName = add_json_suffix(theName);
                //printf("%s", theName);
                s_WorldData.names.push_back(fullName);
                json gameconfig = s_GameConfig;
                json AllGame = s_WorldData;//鑷?鍔ㄥ簭鍒楀寲
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
            if (ImGui::BeginTabItem("World")) { activeTab = 0; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Environments")) { activeTab = 1; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Animals")) { activeTab = 2; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Plants")) { activeTab = 3; ImGui::EndTabItem(); }
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
