# FerryManager (EcoSim)

Ecosystem simulation with C++17, Win32, OpenGL, ImGui.

## Build

```bash
cd D:/Projects/FerryManager
cmake --build build
```

Two targets:
- `ecosim` — full GUI application
- `debug_sim` — headless terminal simulation for parameter tuning

## Parameter tuning workflow

When asked to tune ecosystem parameters, use `debug_sim`:

```bash
cmake --build build && cd build && ./debug_sim.exe
```

Output columns: Step, Plants, P.min, P.avg, P.max, EnvE, Req

### Key config files

| File | What |
|------|------|
| `config/Config.h` | Engine parameters (absorption rates, thresholds, costs) |
| `config/default_config.json` | Species config (energies, speeds, diets) |
| `core/EnvironmentDefine.cpp` | Per-step environment behavior (solar gain, absorption logic) |
| `core/WorldDefine.cpp` | World init & Update loop |

### Energy flow per step

```
GressLand solar gain (+0.8/cell) → Environment energy
    ↓ plant_absorb_rate (0.2 × cellE, capped)
  Plant absorbs → plant energy += abs, cell energy -= abs
    ↓ step_energy_cost (0.2 × overlay_factor)
  Plant net = absorption - cost
    ↓ accumulate to reproduce_threshold (35)
  Reproduce → cost reproduce_energy_cost (18), spawn offspring
```

### Common issues found during tuning

1. **Negative cell energy** — `EnergyExchange` must clamp absorption to `min(abs, energy)` so cells never go below 0 (otherwise `min(0.2 × negative, cap)` returns negative, plants lose energy)
2. **Solar gain vs plant cost balance** — if gain ≤ cost, plants flatline. Need gain > cost for net positive.
3. **Overlay penalty** — `factor = 1/|overlay - overlay_param|`; when density approaches `overlay_param`, penalty explodes → plants die. Keep `overlay_param` away from typical density ranges.
4. **`g_GameConf` must be set** — via `InitGameConfig(cfg)` before `World::GetWorld(cfg)`.
5. **`last_requests`** — saved before clear in `Reproduce()` for UI to read.
