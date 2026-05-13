#include "SetupUI.h"
#include "Config.h"
#include "World.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GL/gl.h>
#include <fstream>
#include <sstream>
#include <cstdio>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static TestConfig s_GameConfig;

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

static bool LoadConfig(TestConfig& cfg, const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    json j;

    j=json::parse(f);
    cfg = j.get<TestConfig>();
    printf("%s\n\n", cfg.Default_Plant_Config.name.data());
    return true;
}

static bool LoadConfigAny(TestConfig& cfg) {
    // Try build dir first, then parent (project root)
    const char* paths[] = {
        "default_config.json",
        "../default_config.json",
        ".../config/default_config.jason",//修改完路径之后
    };
    for (auto p : paths) {
        if (LoadConfig(cfg, p)) return true;
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
        ImGui::InputInt("Width",  &s_GameConfig.The_Word.width);
        ImGui::InputInt("Length", &s_GameConfig.The_Word.length);
        if (s_GameConfig.The_Word.width  < 10) s_GameConfig.The_Word.width  = 10;
        if (s_GameConfig.The_Word.length < 10) s_GameConfig.The_Word.length = 10;
    };

    auto renderDefaultsTab = [&]() {
        if (ImGui::CollapsingHeader("Default Animal", ImGuiTreeNodeFlags_DefaultOpen)) {
            AnimalConfig& a = s_GameConfig.Default_Animal_Config;
            char nameBuf[128];
            snprintf(nameBuf, sizeof(nameBuf), "%s", a.name.c_str());
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
                a.name = nameBuf;
            ImGui::InputInt("Reproduce Rate",  &a.reproduce_original_rate);
            ImGui::InputInt("Reproduce Energy",&a.reproduce_original_energy);
            ImGui::InputFloat("Max Rate",      &a.max_rate);
            ImGui::InputFloat("Step Energy",   &a.step_energy_cost);
            ImGui::InputFloat("Energy Rate",   &a.energy_rate);
        }
        if (ImGui::CollapsingHeader("Default Plant", ImGuiTreeNodeFlags_DefaultOpen)) {
            PlantConfig& p = s_GameConfig.Default_Plant_Config;
            char plantNameBuf[128];
            snprintf(plantNameBuf, sizeof(plantNameBuf), "%s", p.name.c_str());
            if (ImGui::InputText("Name", plantNameBuf, sizeof(plantNameBuf)))
                p.name = plantNameBuf;
            ImGui::InputInt("Reproduce Energy", &p.reproduce_original_energy);
            ImGui::InputFloat("Step Energy",    &p.step_energy_cost);
        }
    };

    auto renderEnvironmentsTab = [&]() {
        for (int i = 0; i < (int)s_GameConfig.The_Environments.size(); ++i) {
            auto& env = s_GameConfig.The_Environments[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char envNameBuf[128];
            snprintf(envNameBuf, sizeof(envNameBuf), "%s", env.name.c_str());
            if (ImGui::InputText("Name", envNameBuf, sizeof(envNameBuf)))
                env.name = envNameBuf;
            char buf[256];
            VecToStr(env.CanLive, buf, sizeof(buf));
            if (ImGui::InputText("CanLive", buf, sizeof(buf)))
                StrToVec(buf, env.CanLive);
            ImGui::PopID();
        }
        if (ImGui::Button("Add Environment")) {
            s_GameConfig.The_Environments.push_back({"NewEnv", {}});
        }
    };

    auto renderAnimalsTab = [&]() {
        for (int i = 0; i < (int)s_GameConfig.The_Animals.size(); ++i) {
            auto& a = s_GameConfig.The_Animals[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char animalNameBuf[128];
            snprintf(animalNameBuf, sizeof(animalNameBuf), "%s", a.name.c_str());
            if (ImGui::InputText("Name", animalNameBuf, sizeof(animalNameBuf)))
                a.name = animalNameBuf;
            char buf[256];
            VecToStr(a.diet, buf, sizeof(buf));
            if (ImGui::InputText("Diet", buf, sizeof(buf)))
                StrToVec(buf, a.diet);
            ImGui::InputInt("Reprod Rate",    &a.reproduce_original_rate);
            ImGui::InputInt("Reprod Energy",  &a.reproduce_original_energy);
            ImGui::InputFloat("Max Rate",     &a.max_rate);
            ImGui::InputFloat("Step Energy",  &a.step_energy_cost);
            ImGui::InputFloat("Energy Rate",  &a.energy_rate);
            ImGui::PopID();
        }
        if (ImGui::Button("Add Animal Species")) {
            s_GameConfig.The_Animals.push_back({"NewSpecies", {}, -1, -1, 100, 0.01f, -1});
        }
    };

    auto renderPlantsTab = [&]() {
        for (int i = 0; i < (int)s_GameConfig.The_Plants.size(); ++i) {
            auto& p = s_GameConfig.The_Plants[i];
            ImGui::PushID(i);
            ImGui::Separator();
            char plantSpNameBuf[128];
            snprintf(plantSpNameBuf, sizeof(plantSpNameBuf), "%s", p.name.c_str());
            if (ImGui::InputText("Name", plantSpNameBuf, sizeof(plantSpNameBuf)))
                p.name = plantSpNameBuf;
            ImGui::InputInt("Reprod Energy", &p.reproduce_original_energy);
            ImGui::InputFloat("Step Energy", &p.step_energy_cost);
            ImGui::PopID();
        }
        if (ImGui::Button("Add Plant Species")) {
            s_GameConfig.The_Plants.push_back({"NewPlant", 10, 0.01f});
        }
    };

    bool startRequested = false;

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
            if (ImGui::BeginTabItem("Defaults"))   { activeTab = 1; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Environments")){ activeTab = 2; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Animals"))    { activeTab = 3; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Plants"))     { activeTab = 4; ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        ImGui::BeginChild("TabContent", ImVec2(0, -50), true);
        switch (activeTab) {
            case 0: renderWorldTab();        break;
            case 1: renderDefaultsTab();     break;
            case 2: renderEnvironmentsTab(); break;
            case 3: renderAnimalsTab();      break;
            case 4: renderPlantsTab();       break;
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
