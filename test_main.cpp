#include "Word.h"
#include "Organism.h"
#include "Environment.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>   // 用于格式化输出

void clearScreen() {
    std::cout << "\033[2J\033[H";  // ANSI 清屏（兼容绝大多数现代终端）
}

// 绘制世界网格
void drawWorld(const World& world) {
    const auto& orgs = world.GetReproducas();
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth();
    int h = world.GetHeight();

    std::vector<std::vector<char>> grid(h, std::vector<char>(w, ' '));

    // 先绘制环境底色
    for (size_t i = 0; i < envs.size(); ++i) {
        int x = i % w;
        int y = i / w;
        switch (envs[i]->name) {
        case GRESSLEND: grid[y][x] = '.'; break;
        case WATER:     grid[y][x] = '~'; break;
        case FOREST:    grid[y][x] = 'T'; break;
        case MOUTAN:    grid[y][x] = '^'; break;
        default:        grid[y][x] = '?';
        }
    }

    // 生物覆盖
    for (auto org : orgs) {
        int x = org->Pos.first;
        int y = org->Pos.second;
        if (x >= 0 && x < w && y >= 0 && y < h) {
            grid[y][x] = (org->type == PLANT) ? '*' : 'A';
        }
    }

    // 打印网格
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
            std::cout << grid[y][x];
        std::cout << '\n';
    }
}

// 统计植物数量并输出每株植物的详细信息
void printPlantDetails(const World& world) {
    const auto& orgs = world.GetReproducas();
    int plantCount = 0;
    std::cout << "\n--- Plant Details ---\n";
    for (const auto* org : orgs) {
        if (org->type == PLANT) {
            // 注意：这里假设所有 PLANT 类型的对象都是 Plant 类实例，向下转型是安全的
            const Plant* plant = static_cast<const Plant*>(org);
            std::cout << "Plant #" << std::setw(3) << plant->id
                << "  Pos: (" << std::setw(3) << org->Pos.first << ","
                << std::setw(3) << org->Pos.second << ")"
                << "  Energy: " << std::fixed << std::setprecision(2) << org->energy
                << "\n";
            ++plantCount;
            // 为了避免输出过长，可限制显示前 20 株（如果需要看全部可注释掉这一行）
            if (plantCount >= 20) {
                std::cout << "... (showing first 20 of " << (orgs.size()) << " plants)\n";
                break;
            }
        }
    }
    std::cout << "Total plants: " << plantCount << "\n";
}

int main() {
    World& world = World::GetWorld();
    world.CurrentWeather = SUN;

    const int totalFrames = 30;
    const int frameDelayMs = 600;

    for (int frame = 0; frame < totalFrames; ++frame) {
        world.Update();

        clearScreen();
        std::cout << "===== Eco Simulation Frame " << (frame + 1)
            << " / " << totalFrames << " =====\n";

        drawWorld(world);
        printPlantDetails(world);

        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelayMs));
    }

    // 最终画面
    clearScreen();
    std::cout << "===== Simulation Finished =====\n";
    drawWorld(world);
    printPlantDetails(world);
    return 0;
}