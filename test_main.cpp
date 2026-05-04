#include "Word.h"
#include "Organism.h"
#include "Environment.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <iomanip>
#include <conio.h>
#include <sstream>
#include <string>
#include <cfloat>
#include <algorithm>

void clearScreen() {
    std::cout << "\033[2J\033[H";
}

void drawWorld(const World& world) {
    const auto& orgs = world.GetReproducas();
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth();
    int h = world.GetHeight();

    std::vector<std::vector<char>> grid(h, std::vector<char>(w, ' '));

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

    for (auto org : orgs) {
        int x = org->Pos.first;
        int y = org->Pos.second;
        if (x >= 0 && x < w && y >= 0 && y < h) {
            grid[y][x] = (org->type == PLANT) ? '*' : 'A';
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
            std::cout << grid[y][x];
        std::cout << '\n';
    }
}

void drawEnergyMap(const World& world) {
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth();
    int h = world.GetHeight();

    // Find min and max energy for normalization
    float minEnergy = FLT_MAX, maxEnergy = FLT_MIN;
    for (size_t i = 0; i < envs.size(); ++i) {
        float e = envs[i]->energy;
        if (e < minEnergy) minEnergy = e;
        if (e > maxEnergy) maxEnergy = e;
    }

    // Energy character mapping (low to high)
    const char* energyChars = " .-:+*#@";
    int numLevels = 7;

    std::cout << "\n--- Energy Map (Heatmap) ---\n";
    std::cout << "Energy range: [" << std::fixed << std::setprecision(2)
              << minEnergy << " - " << maxEnergy << "]\n";

    for (int y = 0; y < h; ++y) {
        std::cout << std::setw(2) << y << " ";
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            if (idx < (int)envs.size()) {
                float e = envs[idx]->energy;
                int level = 0;
                if (maxEnergy > minEnergy) {
                    level = static_cast<int>((e - minEnergy) / (maxEnergy - minEnergy) * (numLevels - 1));
                    level = std::max(0, std::min(level, numLevels - 1));
                }
                std::cout << energyChars[level];
            } else {
                std::cout << '?';
            }
        }
        std::cout << " " << std::setw(2) << y << "\n";
    }

    // Print legend
    std::cout << "   ";
    for (int i = 0; i < numLevels; ++i) {
        std::cout << energyChars[i];
    }
    std::cout << " (low to high energy)\n";
    std::cout << "-----------------------------\n";
}

void printEnergyGrid(const World& world) {
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth();
    int h = world.GetHeight();

    std::cout << "\n--- Energy Grid (Numeric) ---\n";
    std::cout << "     ";
    for (int x = 0; x < w; ++x) {
        std::cout << std::setw(5) << x;
    }
    std::cout << "\n     " << std::string(w * 5, '-') << "\n";

    for (int y = 0; y < h; ++y) {
        std::cout << std::setw(3) << y << " |";
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            if (idx < (int)envs.size()) {
                std::cout << std::setw(5) << std::fixed << std::setprecision(1) << envs[idx]->energy;
            } else {
                std::cout << std::setw(5) << "?";
            }
        }
        std::cout << " | " << std::setw(3) << y << "\n";
    }
    std::cout << "     " << std::string(w * 5, '-') << "\n";
    std::cout << "     ";
    for (int x = 0; x < w; ++x) {
        std::cout << std::setw(5) << x;
    }
    std::cout << "\n---------------------------------\n";
}

void printEnergyHistogram(const World& world) {
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth();
    int h = world.GetHeight();

    // Create histogram buckets
    const int numBuckets = 20;
    float minEnergy = FLT_MAX, maxEnergy = FLT_MIN;
    for (size_t i = 0; i < envs.size(); ++i) {
        float e = envs[i]->energy;
        if (e < minEnergy) minEnergy = e;
        if (e > maxEnergy) maxEnergy = e;
    }

    std::vector<int> buckets(numBuckets, 0);
    for (size_t i = 0; i < envs.size(); ++i) {
        float e = envs[i]->energy;
        int bucket = 0;
        if (maxEnergy > minEnergy) {
            bucket = static_cast<int>((e - minEnergy) / (maxEnergy - minEnergy) * (numBuckets - 1));
            bucket = std::max(0, std::min(bucket, numBuckets - 1));
        }
        buckets[bucket]++;
    }

    // Find max bucket value for scaling
    int maxBucket = *std::max_element(buckets.begin(), buckets.end());
    const int barWidth = 30;

    std::cout << "\n--- Energy Distribution Histogram ---\n";
    for (int i = 0; i < numBuckets; ++i) {
        float low = minEnergy + (maxEnergy - minEnergy) * i / numBuckets;
        float high = minEnergy + (maxEnergy - minEnergy) * (i + 1) / numBuckets;
        int barLen = (maxBucket > 0) ? (buckets[i] * barWidth / maxBucket) : 0;

        std::cout << std::fixed << std::setprecision(2)
                  << std::setw(6) << low << "-"
                  << std::setw(6) << high << " |";
        for (int j = 0; j < barLen; ++j) std::cout << "#";
        std::cout << " " << buckets[i] << "\n";
    }
    std::cout << "Total cells: " << envs.size() << "\n";
    std::cout << "-------------------------------------\n";
}

void printPlantDetails(const World& world) {
    const auto& orgs = world.GetReproducas();
    int plantCount = 0;
    std::cout << "\n--- Plant Details ---\n";
    for (const auto* org : orgs) {
        if (org->type == PLANT) {
            const Plant* plant = static_cast<const Plant*>(org);
            std::cout << "Plant #" << std::setw(3) << plant->id
                << "  Pos: (" << std::setw(3) << org->Pos.first << ","
                << std::setw(3) << org->Pos.second << ")"
                << "  Energy: " << std::fixed << std::setprecision(2) << org->energy
                << "\n";
            ++plantCount;
            if (plantCount >= 20) {
                std::cout << "... (showing first 20 of " << (orgs.size()) << " plants)\n";
                break;
            }
        }
    }
    std::cout << "Total plants: " << plantCount << "\n";
}

void queryPosition(const World& world, int x, int y) {
    const auto& orgs = world.GetReproducas();
    const auto& envs = world.GetEnvironments();
    int w = world.GetWidth();
    int h = world.GetHeight();

    std::cout << "\n--- Position (" << x << ", " << y << ") Info ---\n";

    if (x < 0 || x >= w || y < 0 || y >= h) {
        std::cout << "Coordinates out of bounds!\n";
        return;
    }

    int envIdx = y * w + x;
    if (envIdx >= 0 && envIdx < (int)envs.size()) {
        std::cout << "Environment: ";
        switch (envs[envIdx]->name) {
        case GRESSLEND: std::cout << "GRASSLAND"; break;
        case WATER:     std::cout << "WATER"; break;
        case FOREST:    std::cout << "FOREST"; break;
        case MOUTAN:    std::cout << "MOUNTAIN"; break;
        default:        std::cout << "UNKNOWN"; break;
        }
        std::cout << "\n";
    }

    int found = 0;
    for (const auto* org : orgs) {
        if (org->Pos.first == x && org->Pos.second == y) {
            if (org->type == PLANT) {
                const Plant* plant = static_cast<const Plant*>(org);
                std::cout << "Organism #" << plant->id;
            } else {
                std::cout << "Organism [animal]";
            }
            std::cout << "  Type: " << (org->type == PLANT ? "PLANT" : "ANIMAL")
                << "  Energy: " << std::fixed << std::setprecision(2) << org->energy << "\n";
            ++found;
        }
    }
    if (found == 0) {
        std::cout << "No organism at this position.\n";
    }
    std::cout << "--------------------------\n";
}

int main() {
    World& world = World::GetWorld();
    world.CurrentWeather = SUN;

    const int totalFrames = 3000;
    const int frameDelayMs = 100;

    bool paused = false;
    int frame = 0;

    std::cout << "\n=== Debug Controls ===\n";
    std::cout << "Space: Pause/Resume\n";
    std::cout << "[while paused] Enter 'x y' to query position\n";
    std::cout << "n: Step (single frame)\n";
    std::cout << "s: Continue simulation (auto mode)\n";
    std::cout << "q: Quit\n";
    std::cout << "====================\n\n";

    while (frame < totalFrames) {
        if (_kbhit()) {
            int ch = _getch();

            if (ch == ' ') {
                paused = !paused;
                std::cout << (paused ? "\n[PAUSED] Enter 'n' step, 's' auto, 'q' quit, or type 'x y': " : "[RESUMED]\n");
            }
            else if (ch == 's' || ch == 'S') {
                paused = false;
                std::cout << "[AUTO MODE]\n";
            }
            else if (ch == 'n' || ch == 'N') {
                world.Update();
                ++frame;
                clearScreen();
                std::cout << "===== Eco Simulation Frame " << (frame)
                    << " / " << totalFrames << " =====\n";
                drawWorld(world);
                printPlantDetails(world);
                std::cout << "\n[STEP] Enter 's' auto, 'n' next, 'q' quit, or type 'x y': ";
            }
            else if (ch == 'q' || ch == 'Q') {
                std::cout << "\n===== Simulation Ended =====\n";
                break;
            }
            else if (paused && ch >= '0' && ch <= '9') {
                std::string line;
                line += ch;
                while (_kbhit()) {
                    int ch2 = _getch();
                    if (ch2 == '\r' || ch2 == '\n') break;
                    if (ch2 == ' ') {
                        line += ' ';
                        while (_kbhit()) {
                            int ch3 = _getch();
                            if (ch3 == '\r' || ch3 == '\n') break;
                            line += ch3;
                        }
                        break;
                    }
                    line += ch2;
                }
                std::istringstream iss(line);
                int x = -1, y = -1;
                char space;
                if (iss >> x >> space >> y) {
                    queryPosition(world, x, y);
                } else {
                    iss.clear();
                    iss.str(line);
                    if (iss >> x >> y) {
                        queryPosition(world, x, y);
                    } else {
                        std::cout << "Invalid input. Use format: 'x y' (e.g., '5 10')\n";
                    }
                }
                std::cout << "\n[PAUSED] Enter 'n' step, 's' auto, 'q' quit, or type 'x y': ";
            }
        }

        if (paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        world.Update();
        ++frame;

        clearScreen();
        std::cout << "===== Eco Simulation Frame " << (frame)
            << " / " << totalFrames << " =====\n";

        drawWorld(world);
        drawEnergyMap(world);
        printEnergyGrid(world);
        // printEnergyHistogram(world);
        printPlantDetails(world);

        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelayMs));
    }

    clearScreen();
    std::cout << "===== Simulation Finished =====\n";
    drawWorld(world);
    drawEnergyMap(world);
    printEnergyGrid(world);
    // printEnergyHistogram(world);
    printPlantDetails(world);
    return 0;
}