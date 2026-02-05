/*
    VORTIX ESP - FINAL PRO (PERFECT ANATOMICAL CHAMS UPDATE)
    Date: February 05, 2026
    Features:
    - Perfect Anatomical Chams (Capsule-Based Volumes)
    - High-Fidelity Body Representation Using Convex Hull
    - Custom Glow & Outline (Color/Toggle)
    - Optimized Rendering & Logic
    - 4-Column Layout
*/

#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>

#pragma comment(lib, "winmm.lib")

#include "inc/Renderer.hpp"
#include "imgui/ImAnimate/ImAnimate.h"
#include "imgui/ImToggle/imgui_toggle.h"
#include "inc/VisCheck.h"
#include "inc/mem.hpp"
#include "inc/utils.hpp"
#include "inc/input.hpp"
#include "inc/offsets.hpp"
#include "inc/hitbox.hpp"

#include "inc/firaCode.hpp"
#include "inc/weaponFont.hpp"

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

namespace FlagOffsets {
    constexpr ptrdiff_t m_pInGameMoneyServices = 0x808;
    constexpr ptrdiff_t m_iAccount = 0x40;
    constexpr ptrdiff_t m_vecAbsVelocity = 0x3E0;
    constexpr ptrdiff_t m_pItemServices = 0x13E0;
    constexpr ptrdiff_t m_bHasDefuser = 0x48;
    constexpr ptrdiff_t m_bIsScoped = 0x26F8;
    constexpr ptrdiff_t m_bIsDefusing = 0x1408;
    constexpr ptrdiff_t m_bIsPlanting = 0x27D0;
    constexpr ptrdiff_t m_bHasHelmet = 0x49;
    constexpr ptrdiff_t m_ArmorValue = 0x272C;
    constexpr ptrdiff_t m_flFlashDuration = 0x15F8;
    constexpr ptrdiff_t m_flFlashMaxAlpha = 0x15FC;
    constexpr ptrdiff_t m_pWeaponServices = 0x13D8;
    constexpr ptrdiff_t m_hMyWeapons = 0x48;
    constexpr ptrdiff_t m_AttributeManager = 0x13B8;
    constexpr ptrdiff_t m_Item = 0x50;
    constexpr ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
    constexpr ptrdiff_t m_pObserverServices = 0x13F0;
    constexpr ptrdiff_t m_hObserverTarget = 0x4C;
    constexpr ptrdiff_t dwGameTypes = 0x1B8000;
    constexpr ptrdiff_t m_vecViewOffset = 0xCF8;

}

namespace BombOffsets {
    constexpr ptrdiff_t dwGameRules = 0x23035F8;
    constexpr ptrdiff_t dwPlantedC4 = 0x230FCF0;
    constexpr ptrdiff_t dwGlobalVars = 0x20572A8;
    constexpr ptrdiff_t m_nBombSite = 0x1174;
    constexpr ptrdiff_t m_bBombPlanted = 0x1F7B;
    constexpr ptrdiff_t m_flC4Blow = 0x11A0;
    constexpr ptrdiff_t m_flDefuseCountDown = 0x11C0;
    constexpr ptrdiff_t m_bBeingDefused = 0x11AC;
    constexpr ptrdiff_t m_flTimerLength = 0x11A8;
    constexpr ptrdiff_t m_flDefuseLength = 0x11BC;
}

// === MATHEMATICAL CONSTANTS ===
namespace Math {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 6.28318530717958647692f;
    constexpr float HALF_PI = 1.57079632679489661923f;
    constexpr float DEG_TO_RAD = 0.01745329251994329576f;
    constexpr float RAD_TO_DEG = 57.2957795130823208767f;
}

// --- VARIÁVEIS GLOBAIS ---
ImFont* espNameFont = nullptr;
ImFont* tahomaBoldFont = nullptr;
ImFont* smallFont = nullptr;
ImFont* pixelFont = nullptr;
ImFont* gamesenseFont = nullptr;
ImFont* tahomaRegularFont;

VisCheck vCheck("");
std::string currentMapName = "";
ULONGLONG lastMapCheckTime = 0;

// SISTEMA DE MAPAS
std::vector<std::string> availableMaps;
int selectedMapIndex = 0;
bool autoDetectMap = true;

// SISTEMA DE HITSOUNDS
std::vector<std::string> availableHitsounds;
int selectedHitsoundIndex = 0;
std::string currentHitsoundPath = "";

// SISTEMA DE NOMES DE TECLAS
std::string VirtualKeyNames[256];

void InitKeyNames() {
    for (int i = 0; i < 256; i++) VirtualKeyNames[i] = "Unknown";
    VirtualKeyNames[0] = "None";
    VirtualKeyNames[VK_LBUTTON] = "M1"; VirtualKeyNames[VK_RBUTTON] = "M2"; VirtualKeyNames[VK_MBUTTON] = "M3";
    VirtualKeyNames[VK_XBUTTON1] = "M4"; VirtualKeyNames[VK_XBUTTON2] = "M5";
    VirtualKeyNames[VK_BACK] = "Backspace"; VirtualKeyNames[VK_TAB] = "Tab"; VirtualKeyNames[VK_CLEAR] = "Clear";
    VirtualKeyNames[VK_RETURN] = "Enter"; VirtualKeyNames[VK_SHIFT] = "Shift"; VirtualKeyNames[VK_CONTROL] = "Ctrl";
    VirtualKeyNames[VK_MENU] = "Alt"; VirtualKeyNames[VK_PAUSE] = "Pause"; VirtualKeyNames[VK_CAPITAL] = "CapsLock";
    VirtualKeyNames[VK_ESCAPE] = "Esc"; VirtualKeyNames[VK_SPACE] = "Space"; VirtualKeyNames[VK_PRIOR] = "PageUp";
    VirtualKeyNames[VK_NEXT] = "PageDown"; VirtualKeyNames[VK_END] = "End"; VirtualKeyNames[VK_HOME] = "Home";
    VirtualKeyNames[VK_LEFT] = "Left"; VirtualKeyNames[VK_UP] = "Up"; VirtualKeyNames[VK_RIGHT] = "Right"; VirtualKeyNames[VK_DOWN] = "Down";
    VirtualKeyNames[VK_SELECT] = "Select"; VirtualKeyNames[VK_PRINT] = "Print"; VirtualKeyNames[VK_EXECUTE] = "Execute";
    VirtualKeyNames[VK_SNAPSHOT] = "PrintScreen"; VirtualKeyNames[VK_INSERT] = "Insert"; VirtualKeyNames[VK_DELETE] = "Delete";
    VirtualKeyNames[VK_HELP] = "Help";
    for (int i = '0'; i <= '9'; i++) VirtualKeyNames[i] = std::string(1, (char)i);
    for (int i = 'A'; i <= 'Z'; i++) VirtualKeyNames[i] = std::string(1, (char)i);
    VirtualKeyNames[VK_LWIN] = "LWin"; VirtualKeyNames[VK_RWIN] = "RWin"; VirtualKeyNames[VK_APPS] = "Apps";
    for (int i = 0; i < 24; i++) VirtualKeyNames[VK_F1 + i] = "F" + std::to_string(i + 1);
    VirtualKeyNames[VK_NUMLOCK] = "NumLock"; VirtualKeyNames[VK_SCROLL] = "Scroll";
    VirtualKeyNames[VK_LSHIFT] = "LShift"; VirtualKeyNames[VK_RSHIFT] = "RShift";
    VirtualKeyNames[VK_LCONTROL] = "LCtrl"; VirtualKeyNames[VK_RCONTROL] = "RCtrl";
    VirtualKeyNames[VK_LMENU] = "LAlt"; VirtualKeyNames[VK_RMENU] = "RAlt";
}

const char* GetKeyNameSafe(int id) {
    if (id < 0 || id > 255) return "Unknown";
    return VirtualKeyNames[id].c_str();
}

// Offsets moved to inc/offsets.hpp for automatic updating



namespace Globals
{
    bool imguiVisible{ false };
    float copiedColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float bombTimeLeft = 0.0f;
    float defuseTimeLeft = 0.0f;
    bool isBombPlanted = false;
    bool isBeingDefused = false;
    int bombSite = 0; // 0 = A, 1 = B
    bool isPlanting = false;
}

struct SoundRipple {
    Vector3 position;
    ULONGLONG startTime;
    float maxRadius = 30.0f;
    float duration = 700.0f;
    ImU32 color;
};

std::vector<SoundRipple> activeRipples;
std::map<int, ULONGLONG> entitySoundCooldown;
std::map<int, long long> entityRunStartTime;

struct SnowFlake {
    float x, y;
    float speed;
    float size;
    float opacity;
};

std::vector<SnowFlake> snowFlakes;
ULONGLONG lastHitTime = 0;
int lastHitCount = 0;

enum KeyMode { AlwaysOn = 0, Hold, Toggle };

struct KeyBind {
    int key = 0;
    int mode = 2;
    bool active = true;
    bool waitingForKey = false;
};

struct ConfigItem {
    bool enabled;
    float color[4];
    ConfigItem() {
        enabled = false;
        for (int i = 0; i < 4; i++) color[i] = 1.0f;
    }
};

struct WeaponSettings {
    bool enabled = false;
    bool text_enabled = false; float text_color[4] = { 1, 1, 1, 1 };
    bool icon_enabled = false; float icon_color[4] = { 1, 1, 1, 1 };
};

struct SoundSettings {
    bool enabled = false;
    float color[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
};

struct HealthSettings {
    bool enabled = false;
    int mode = 1;
    float static_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float gradient_low[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float gradient_high[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
};

struct ArmorSettings {
    bool enabled = false;
    float static_color[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
};

struct FlagItem {
    bool enabled;
    float color[4];
    FlagItem(float r, float g, float b) {
        enabled = false;
        color[0] = r; color[1] = g; color[2] = b; color[3] = 1.0f;
    }
    FlagItem() { enabled = false; color[0] = 1; color[1] = 1; color[2] = 1; color[3] = 1; }
};

struct ESPGroup {
    ConfigItem master;
    KeyBind masterKey;

    ConfigItem box;
    float boxVisibleColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

    ConfigItem name;

    ConfigItem skeleton;
    float skeletonVisibleColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

    ConfigItem chams;
    float chamsVisibleColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float chamsAlpha = 0.75f;
    
    // Chams modes: 0=Flat, 1=Gradient, 2=Wireframe, 3=Glow, 4=Pulse
    int chamsMode = 0;
    
    // Gradient settings
    float chamsGradientTop[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
    float chamsGradientBottom[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
    
    // Pulse settings
    float chamsPulseSpeed = 2.0f;

    bool chamsOutline = true;
    float chamsOutlineColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float chamsOutlineVisibleColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

    bool chamsGlow = false;
    float chamsGlowColor[4] = { 1.0f, 1.0f, 1.0f, 0.6f };
    float chamsGlowVisibleColor[4] = { 0.0f, 1.0f, 0.0f, 0.8f };
    bool chamsGlowOnly = false;

    ConfigItem snaplines;
    
    // Out of FOV arrows
    bool outOfFovEnabled = false;
    float outOfFovColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float outOfFovSize = 20.0f;
    float outOfFovDistance = 100.0f; // Distância da borda da tela
    
    ConfigItem ammoBar;

    bool visCheckEnabled = false;

    SoundSettings sound;
    WeaponSettings weapon;
    HealthSettings health;
    ArmorSettings armor;
    ConfigItem flagsMaster;
    FlagItem flagMoney{ 0.45f, 0.7f, 0.2f };
    FlagItem flagArmor{ 1.0f, 1.0f, 1.0f };
    FlagItem flagZoom{ 0.0f, 0.7f, 1.0f };
    FlagItem flagKit{ 0.2f, 0.8f, 0.2f };
    FlagItem flagBlind{ 1.0f, 0.8f, 0.0f };

    ESPGroup() {
        master.enabled = true;
        masterKey.key = 0;
        masterKey.mode = KeyMode::AlwaysOn;
        box.color[0] = 0.9f; box.color[1] = 0.2f; box.color[2] = 0.2f; box.color[3] = 1.f;
        boxVisibleColor[0] = 0.0f; boxVisibleColor[1] = 1.0f; boxVisibleColor[2] = 0.0f; boxVisibleColor[3] = 1.0f;
        name.color[0] = 1.f; name.color[1] = 1.f; name.color[2] = 1.f; name.color[3] = 1.f;
        skeleton.color[0] = 0.9f; skeleton.color[1] = 0.9f; skeleton.color[2] = 0.9f; skeleton.color[3] = 1.f;
        skeletonVisibleColor[0] = 0.0f; skeletonVisibleColor[1] = 1.0f; skeletonVisibleColor[2] = 0.0f; skeletonVisibleColor[3] = 1.0f;
        chams.color[0] = 1.0f; chams.color[1] = 0.0f; chams.color[2] = 0.0f; chams.color[3] = 0.6f;
        chamsVisibleColor[0] = 0.0f; chamsVisibleColor[1] = 1.0f; chamsVisibleColor[2] = 0.0f; chamsVisibleColor[3] = 0.6f;
        snaplines.color[0] = 1.f; snaplines.color[1] = 1.f; snaplines.color[2] = 1.f; snaplines.color[3] = 1.f;
        ammoBar.color[0] = 0.3f; ammoBar.color[1] = 0.6f; ammoBar.color[2] = 1.0f; ammoBar.color[3] = 1.f;
        armor.static_color[0] = 0.0f; armor.static_color[1] = 0.5f; armor.static_color[2] = 1.0f; armor.static_color[3] = 1.0f;
        weapon.text_enabled = true; weapon.icon_enabled = false;
        weapon.text_color[0] = 1.f; weapon.text_color[1] = 1.f; weapon.text_color[2] = 1.f; weapon.text_color[3] = 1.f;
        weapon.icon_color[0] = 1.f; weapon.icon_color[1] = 1.f; weapon.icon_color[2] = 1.f; weapon.icon_color[3] = 1.f;
        visCheckEnabled = false;
    }
};

struct HitmarkerSettings {
    bool enabled = false;
    float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float outline_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float thickness = 1.4f;
    float outline_thickness = 2.4f;
    int duration_ms = 500;
    bool show_outline = true;
    float size = 10.0f;
    float gap = 3.0f;
};

struct CrosshairSettings {
    bool enabled = false;
    float color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    bool outline = true;
    float outline_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float thickness = 1.0f;
    float length = 5.0f;
    float gap = 2.0f;
    bool sniper_only = true;
    bool show_when_scoped = false;
};

struct MiscSettings {
    ConfigItem master;
    KeyBind masterKey;
    bool spectator_list = false;
    bool stream_proof = false;
    CrosshairSettings crosshair;
    HitmarkerSettings hitmarker;
    
    // Hitsound
    bool hitsound_enabled = false;
    float hitsound_volume = 50.0f;
    
    // Screen Color
    bool screencolor_enabled = false;
    float screencolor[4] = { 1.0f, 0.0f, 0.0f, 0.15f };
    
    // Radar
    bool radar_enabled = false;
    float radar_scale = 1.0f;
    float radar_alpha = 0.8f;
    
    // Effects
    bool snow_effect = false;
    int snow_amount = 150;
    float snow_speed = 1.0f;
    
    MiscSettings() {
        master.enabled = true;
        masterKey.mode = KeyMode::AlwaysOn;
    }
};

// Weapon Visuals removido - World ESP simplificado

struct WorldSettings {
    ConfigItem master;
    KeyBind masterKey;
    ConfigItem bomb_indicator;
    
    WorldSettings() {
        master.enabled = true;
        masterKey.mode = KeyMode::AlwaysOn;
        bomb_indicator.color[0] = 1.0f; bomb_indicator.color[1] = 0.0f; bomb_indicator.color[2] = 0.0f;
    }
};

// === AIM SETTINGS ===
struct WeaponCategoryAim {
    // Aimbot
    bool aimbot_enabled = false;
    int aimbot_bone = 6;
    float aimbot_fov = 5.0f;
    float aimbot_smooth = 10.0f;
    KeyBind aimbot_key;
    
    // Advanced Aimbot
    bool aimbot_silentaim = false;
    bool aimbot_autopenetration = false;
    bool aimbot_autoscope = false;
    bool aimbot_autostop = false;
    float aimbot_hitchance = 60.0f;
    float aimbot_mindamage = 30.0f;
    int aimbot_multipoint = 0;
    float aimbot_headscale = 0.5f;
    float aimbot_bodyscale = 0.5f;
    
    // Backtrack
    bool backtrack_enabled = false;
    int backtrack_ticks = 12;
    
    // Resolver
    bool resolver_enabled = false;
    int resolver_mode = 0;
    
    // Triggerbot
    bool triggerbot_enabled = false;
    int triggerbot_delay = 0;
    KeyBind triggerbot_key;
    
    // Advanced Triggerbot
    bool triggerbot_burstfire = false;
    int triggerbot_burstshots = 3;
    bool triggerbot_magnet = false;
    float triggerbot_hitchance = 60.0f;
    
    // RCS (Recoil Control System)
    bool rcs_enabled = false;
    float rcs_x = 100.0f;
    float rcs_y = 100.0f;
    bool rcs_standalone = false;
    
    // Autofire
    bool autofire_enabled = false;
    
    WeaponCategoryAim() {
        aimbot_key.mode = KeyMode::Hold;
        aimbot_key.key = VK_XBUTTON1;
        triggerbot_key.mode = KeyMode::Hold;
        triggerbot_key.key = VK_XBUTTON2;
    }
};

struct AimSettings {
    ConfigItem master;
    KeyBind masterKey;
    
    WeaponCategoryAim pistol;
    WeaponCategoryAim smg;
    WeaponCategoryAim rifle;
    WeaponCategoryAim sniper;
    WeaponCategoryAim shotgun;
    WeaponCategoryAim heavy;
    
    AimSettings() {
        master.enabled = false; // Desabilitado por padrão
        masterKey.mode = KeyMode::AlwaysOn;
    }
};

static ESPGroup enemy, team;
static WorldSettings world;
static MiscSettings misc;
static AimSettings aim;

// --- DADOS PARA OVERLAY CHAMS ---
struct BodyPart {
    int bone1;
    int bone2;
    float radius_factor;
};

// Definição Anatômica Perfeita (Baseada nas hitboxes oficiais do CS2)
const std::vector<BodyPart> anatomicalParts = {
    // === TRONCO (Core) ===
    {0, 2, 0.135f},   // Pelvis -> Spine1
    {2, 4, 0.145f},   // Spine1 -> Spine2  
    {4, 5, 0.140f},   // Spine2 -> Neck
    
    // === CABEÇA ===
    {6, 6, 0.105f},   // Head (círculo único)
    
    // === BRAÇO ESQUERDO ===
    {5, 13, 0.060f},  // Neck -> L Upperarm
    {13, 14, 0.065f}, // L Upperarm -> L Forearm
    {14, 15, 0.050f}, // L Forearm -> L Hand
    
    // === BRAÇO DIREITO ===
    {5, 8, 0.060f},   // Neck -> R Upperarm
    {8, 9, 0.065f},   // R Upperarm -> R Forearm
    {9, 10, 0.050f},  // R Forearm -> R Hand
    
    // === PERNA ESQUERDA ===
    {0, 25, 0.085f},  // Pelvis -> L Thigh
    {25, 26, 0.100f}, // L Thigh -> L Calf (mais grossa)
    {26, 27, 0.065f}, // L Calf -> L Foot
    
    // === PERNA DIREITA ===
    {0, 22, 0.085f},  // Pelvis -> R Thigh
    {22, 23, 0.100f}, // R Thigh -> R Calf (mais grossa)
    {23, 24, 0.065f}  // R Calf -> R Foot
};


// --- FUNÇÕES AUXILIARES ---

void RefreshMapList() {
    availableMaps.clear();
    if (!std::filesystem::exists("maps")) {
        try { std::filesystem::create_directory("maps"); }
        catch (...) {}
    }
    if (std::filesystem::exists("maps")) {
        for (const auto& entry : std::filesystem::directory_iterator("maps")) {
            if (entry.path().extension() == ".opt") {
                availableMaps.push_back(entry.path().filename().string());
            }
        }
    }
    if (availableMaps.empty()) {
        availableMaps.push_back("No files found");
        selectedMapIndex = 0;
    }
    else {
        if (selectedMapIndex >= availableMaps.size()) selectedMapIndex = 0;
    }
}

void RefreshHitsoundList() {
    availableHitsounds.clear();
    if (!std::filesystem::exists("hitsounds")) {
        try { 
            std::filesystem::create_directory("hitsounds"); 
            std::cout << "[INFO] Created hitsounds folder. Place your .wav files there!\n";
        }
        catch (...) {}
    }
    if (std::filesystem::exists("hitsounds")) {
        for (const auto& entry : std::filesystem::directory_iterator("hitsounds")) {
            if (entry.path().extension() == ".wav") {
                availableHitsounds.push_back(entry.path().filename().string());
            }
        }
    }
    
    // Adicionar som padrão do Windows
    availableHitsounds.insert(availableHitsounds.begin(), "Default (Windows Ding)");
    
    if (availableHitsounds.size() == 1) {
        std::cout << "[INFO] No custom hitsounds found. Using default Windows sound.\n";
    }
    
    if (selectedHitsoundIndex >= availableHitsounds.size()) selectedHitsoundIndex = 0;
    
    // Atualizar path do hitsound atual
    if (selectedHitsoundIndex == 0) {
        currentHitsoundPath = "C:\\Windows\\Media\\Windows Ding.wav";
    } else {
        currentHitsoundPath = "hitsounds\\" + availableHitsounds[selectedHitsoundIndex];
    }
}

ImU32 ColorToU32(float color[4]) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3]));
}

ImU32 GetAlphaModifiedColor(float color[4], float alpha_mult) {
    ImVec4 c = ImVec4(color[0], color[1], color[2], color[3] * alpha_mult);
    return ImGui::ColorConvertFloat4ToU32(c);
}

ImU32 GetGradientColor(float percentage, float lowColor[4], float highColor[4]) {
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;
    float r = lowColor[0] + (highColor[0] - lowColor[0]) * percentage;
    float g = lowColor[1] + (highColor[1] - lowColor[1]) * percentage;
    float b = lowColor[2] + (highColor[2] - lowColor[2]) * percentage;
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
}

bool IsBlinded(Memory& mem, uintptr_t pawnAddress) {
    float flashDuration = mem.read<float>(pawnAddress + FlagOffsets::m_flFlashDuration);
    return flashDuration > 0.1f;
}

std::string GetMapName(Memory& mem, uintptr_t client) {
    try {
        uintptr_t globalVarsPtr = mem.read<uintptr_t>(client + Offsets::dwGlobalVars);
        if (!globalVarsPtr) return "";
        
        // Offset para mapname está em +0x180
        char mapBuffer[128] = { 0 };
        for (int i = 0; i < 127; i++) {
            char c = mem.read<char>(globalVarsPtr + 0x180 + i);
            if (c == 0) break;
            if (c < 32 || c > 126) break;
            mapBuffer[i] = c;
        }
        
        std::string mapName = std::string(mapBuffer);
        if (mapName.empty()) return "";
        
        // Remover extensão se houver
        size_t extension = mapName.find(".vpk");
        if (extension != std::string::npos) mapName = mapName.substr(0, extension);
        
        // Pegar apenas o nome do mapa (sem pasta/caminho)
        size_t lastSlash = mapName.find_last_of("/\\");
        if (lastSlash != std::string::npos) mapName = mapName.substr(lastSlash + 1);
        
        return mapName;
    }
    catch(...) {
        return "";
    }
}

void UpdateMapVisCheck(Memory& mem, uintptr_t client) {
    if (!autoDetectMap) return;
    if (GetTickCount64() - lastMapCheckTime < 2000) return;
    lastMapCheckTime = GetTickCount64();

    std::string map = GetMapName(mem, client);
    if (!map.empty() && map != currentMapName) {
        currentMapName = map;
        std::string optName = map + ".opt";
        std::string pathInMaps = "maps/" + optName;
        if (std::filesystem::exists(pathInMaps)) {
            vCheck = VisCheck(pathInMaps);
            std::cout << "[INFO] Auto-Loaded: " << pathInMaps << std::endl;
        }
        else if (std::filesystem::exists(optName)) {
            vCheck = VisCheck(optName);
            std::cout << "[INFO] Auto-Loaded (Root): " << optName << std::endl;
        }
    }
}

void UpdateKeybind(KeyBind& bind) {
    if (bind.waitingForKey) {
        for (int i = 1; i < 255; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                if (i == VK_ESCAPE) bind.key = 0;
                else bind.key = i;
                bind.waitingForKey = false;
                while (GetAsyncKeyState(i) & 0x8000) { Sleep(5); }
                return;
            }
        }
        return;
    }
    if (bind.key == 0) {
        bind.active = true;
        return;
    }
    bool keyDown = GetAsyncKeyState(bind.key) & 0x8000;
    static std::map<int, bool> keyWasDown;
    if (bind.mode == KeyMode::AlwaysOn) bind.active = true;
    else if (bind.mode == KeyMode::Hold) bind.active = keyDown;
    else if (bind.mode == KeyMode::Toggle) {
        if (keyDown && !keyWasDown[bind.key]) bind.active = !bind.active;
        keyWasDown[bind.key] = keyDown;
    }
}

// --- WEAPON & PROJECTILE HELPERS ---

std::string GetWeaponNameFromID(int weaponID) {
    static std::map<int, std::string> weaponNames = {
        {1, "Desert Eagle"}, {2, "Dual Berettas"}, {3, "Five-SeveN"}, {4, "Glock-18"},
        {7, "AK-47"}, {8, "AUG"}, {9, "AWP"}, {10, "FAMAS"}, {11, "G3SG1"},
        {13, "Galil AR"}, {14, "M249"}, {16, "M4A4"}, {17, "MAC-10"},
        {19, "P90"}, {23, "MP5-SD"}, {24, "UMP-45"}, {25, "XM1014"},
        {26, "PP-Bizon"}, {27, "MAG-7"}, {28, "Negev"}, {29, "Sawed-Off"},
        {30, "Tec-9"}, {32, "P2000"}, {33, "MP7"}, {34, "MP9"},
        {35, "Nova"}, {36, "P250"}, {38, "SCAR-20"}, {39, "SG 553"},
        {40, "SSG 08"}, {60, "M4A1-S"}, {61, "USP-S"}, {63, "CZ75-Auto"},
        {64, "R8 Revolver"}
    };
    auto it = weaponNames.find(weaponID);
    return (it != weaponNames.end()) ? it->second : "Unknown";
}

std::string GetProjectileNameFromClass(const std::string& className) {
    if (className.find("smokegrenade") != std::string::npos) return "Smoke";
    if (className.find("flashbang") != std::string::npos) return "Flash";
    if (className.find("hegrenade") != std::string::npos) return "HE Grenade";
    if (className.find("molotov") != std::string::npos || className.find("incgrenade") != std::string::npos) return "Molotov";
    return "Projectile";
}

int GetProjectileType(const std::string& className) {
    if (className.find("smokegrenade") != std::string::npos) return 0;
    if (className.find("flashbang") != std::string::npos) return 1;
    if (className.find("hegrenade") != std::string::npos) return 2;
    if (className.find("molotov") != std::string::npos || className.find("incgrenade") != std::string::npos) return 3;
    return -1;
}

void InitSnowEffect(int monitorWidth, int monitorHeight) {
    snowFlakes.clear();
    int amount = misc.snow_amount;
    for (int i = 0; i < amount; i++) {
        SnowFlake flake;
        flake.x = static_cast<float>(rand() % monitorWidth);
        flake.y = static_cast<float>(rand() % monitorHeight);
        flake.speed = (0.3f + (rand() % 150) / 100.0f) * misc.snow_speed;
        flake.size = 1.5f + (rand() % 4);
        flake.opacity = 0.4f + (rand() % 60) / 100.0f;
        snowFlakes.push_back(flake);
    }
}

void UpdateSnowEffect(int monitorWidth, int monitorHeight) {
    for (auto& flake : snowFlakes) {
        flake.y += flake.speed;
        // Movimento horizontal suave (vento)
        flake.x += std::sin(flake.y * 0.01f) * 0.5f;
        
        if (flake.y > monitorHeight) {
            flake.y = -10.0f;
            flake.x = static_cast<float>(rand() % monitorWidth);
        }
        if (flake.x < 0) flake.x = static_cast<float>(monitorWidth);
        if (flake.x > monitorWidth) flake.x = 0;
    }
}

// --- DRAWING FUNCTIONS ---

void DrawSkeetText(ImDrawList* drawList, ImFont* font, float fontSize, ImVec2 pos, ImU32 textColor, const char* text, bool centered = false) {
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    if (centered) pos.x -= floorf(textSize.x / 2.0f);
    ImU32 black = IM_COL32(0, 0, 0, 255);
    drawList->AddText(font, fontSize, ImVec2(pos.x - 1, pos.y - 1), black, text);
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y - 1), black, text);
    drawList->AddText(font, fontSize, ImVec2(pos.x - 1, pos.y + 1), black, text);
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y + 1), black, text);
    drawList->AddText(font, fontSize, pos, textColor, text);
}

void DrawNameText(ImDrawList* drawList, ImFont* font, float fontSize, ImVec2 pos, ImU32 textColor, const char* text) {
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    pos.x -= floorf(textSize.x / 2.0f);
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 0, 0, 230), text);
    drawList->AddText(font, fontSize, pos, textColor, text);
}

// --- IMGUI HELPERS ---
void CustomColorEdit(const char* label, float* color) {
    std::string id = "##cp_" + std::string(label);
    ImGui::ColorEdit4(id.c_str(), color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    if (ImGui::BeginPopupContextItem(id.c_str())) {
        if (ImGui::MenuItem("Copy Color")) memcpy(Globals::copiedColor, color, sizeof(float) * 4);
        if (ImGui::MenuItem("Paste Color")) memcpy(color, Globals::copiedColor, sizeof(float) * 4);
        ImGui::EndPopup();
    }
}

void DrawConfigItem(const char* label, ConfigItem& item) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
    CustomColorEdit(label, item.color);
}

void DrawConfigItemWithVisible(const char* label, ConfigItem& item, float* visibleColor, bool showVisible) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    if (showVisible) {
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 52);
        CustomColorEdit((std::string(label) + "_hid").c_str(), item.color);
        ImGui::SameLine();
        CustomColorEdit((std::string(label) + "_vis").c_str(), visibleColor);
    }
    else {
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        CustomColorEdit(label, item.color);
    }
}

void DrawFlagOption(const char* label, FlagItem& item) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
    std::string id = std::string("##flg_") + label;
    CustomColorEdit(id.c_str(), item.color);
}

void DrawKeybindWidget(KeyBind& bind) {
    std::string label = bind.waitingForKey ? "[ ... ]" : "[ " + std::string(GetKeyNameSafe(bind.key)) + " ]";
    if (bind.key == 0 && !bind.waitingForKey) label = "[ - ]";
    if (bind.mode == KeyMode::AlwaysOn) label = "[ Always ]";
    ImVec2 size = ImGui::CalcTextSize(label.c_str());
    if (ImGui::Button(label.c_str(), ImVec2(size.x + 10, 0))) {
        bind.waitingForKey = !bind.waitingForKey;
    }
    if (ImGui::BeginPopupContextItem(0, 1)) {
        if (ImGui::Selectable("Always On", bind.mode == KeyMode::AlwaysOn)) bind.mode = KeyMode::AlwaysOn;
        if (ImGui::Selectable("Hold", bind.mode == KeyMode::Hold)) bind.mode = KeyMode::Hold;
        if (ImGui::Selectable("Toggle", bind.mode == KeyMode::Toggle)) bind.mode = KeyMode::Toggle;
        ImGui::EndPopup();
    }
}

void DrawMasterSwitch(const char* label, ConfigItem& item, KeyBind& bind) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 60);
    std::string bindId = "##bind_" + std::string(label);
    ImGui::PushID(bindId.c_str());
    DrawKeybindWidget(bind);
    ImGui::PopID();
}

// === GEAR ICON SYSTEM FOR SUB-MENUS ===
void DrawGearIcon(ImDrawList* drawList, ImVec2 center, float radius, ImU32 color) {
    const int teeth = 8;
    const float innerRadius = radius * 0.6f;
    const float toothDepth = radius * 0.3f;
    
    for (int i = 0; i < teeth * 2; i++) {
        float angle1 = (float)i / (teeth * 2) * Math::TWO_PI;
        float angle2 = (float)(i + 1) / (teeth * 2) * Math::TWO_PI;
        float r1 = (i % 2 == 0) ? radius : innerRadius;
        float r2 = ((i + 1) % 2 == 0) ? radius : innerRadius;
        
        ImVec2 p1(center.x + cos(angle1) * r1, center.y + sin(angle1) * r1);
        ImVec2 p2(center.x + cos(angle2) * r2, center.y + sin(angle2) * r2);
        
        drawList->AddLine(p1, p2, color, 1.5f);
    }
    
    drawList->AddCircleFilled(center, innerRadius * 0.5f, color, 12);
}

bool DrawGearButton(const char* id, const char* tooltip = nullptr) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    
    bool clicked = ImGui::Button(id, ImVec2(20, 20));
    
    if (ImGui::IsItemHovered() && tooltip) {
        ImGui::SetTooltip("%s", tooltip);
    }
    
    ImVec2 buttonMin = ImGui::GetItemRectMin();
    ImVec2 buttonMax = ImGui::GetItemRectMax();
    ImVec2 center((buttonMin.x + buttonMax.x) * 0.5f, (buttonMin.y + buttonMax.y) * 0.5f);
    
    ImU32 gearColor = ImGui::IsItemHovered() ? IM_COL32(143, 209, 98, 255) : IM_COL32(180, 180, 180, 255);
    DrawGearIcon(ImGui::GetWindowDrawList(), center, 6.0f, gearColor);
    
    ImGui::PopStyleColor(3);
    
    return clicked;
}

// Helper function to draw checkbox with gear settings button
bool DrawConfigItemWithGear(const char* label, bool* enabled, const char* gearId, const char* tooltip = nullptr) {
    ImGui::Checkbox(label, enabled);
    
    if (*enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        return DrawGearButton(gearId, tooltip);
    }
    
    return false;
}

// === NEVERLOSE-STYLE ANIMATED TOGGLE (Simplified) ===
bool DrawAnimatedToggle(const char* label, bool* value, float speed = 10.0f) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    float height = ImGui::GetFrameHeight();
    float width = height * 1.8f;
    float radius = height * 0.5f;
    
    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool clicked = ImGui::IsItemClicked();
    
    if (clicked) {
        *value = !(*value);
    }
    
    bool hovered = ImGui::IsItemHovered();
    
    // Animation
    static std::map<std::string, float> animation_map;
    float& animation = animation_map[std::string(label)];
    float target = *value ? 1.0f : 0.0f;
    float anim_speed = ImGui::GetIO().DeltaTime * speed;
    
    if (animation < target) {
        animation = (std::min)(animation + anim_speed, target);
    }
    else if (animation > target) {
        animation = (std::max)(animation - anim_speed, target);
    }
    // Colors
    ImU32 col_bg = *value ? IM_COL32(143, 209, 98, 255) : IM_COL32(60, 60, 60, 255);
    if (hovered) {
        col_bg = *value ? IM_COL32(156, 227, 109, 255) : IM_COL32(80, 80, 80, 255);
    }
    
    ImU32 col_circle = IM_COL32(255, 255, 255, 255);
    
    // Draw background
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, radius);
    
    // Draw circle (knob)
    float circle_pos_x = p.x + radius + animation * (width - radius * 2.0f);
    draw_list->AddCircleFilled(ImVec2(circle_pos_x, p.y + radius), radius - 2.0f, col_circle, 16);
    
    // Draw label
    ImVec2 label_size = ImGui::CalcTextSize(label);
    if (label_size.x > 0.0f) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemInnerSpacing.x);
        ImGui::Text("%s", label);
    }
    
    return clicked;
}

// === HELPER FUNCTIONS FOR TOGGLE + COLOR + GEAR ===
void DrawToggleWithColor(const char* label, ConfigItem& item) {
    DrawAnimatedToggle(label, &item.enabled);
    if (item.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        CustomColorEdit(label, item.color);
    }
}

void DrawToggleWithColorAndGear(const char* label, ConfigItem& item, const char* gearId, std::function<void()> gearContent) {
    DrawAnimatedToggle(label, &item.enabled);
    
    if (item.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 52);
        CustomColorEdit(label, item.color);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton(gearId, nullptr)) {
            ImGui::OpenPopup(gearId);
        }
    }
    
    if (ImGui::BeginPopup(gearId)) {
        gearContent();
        ImGui::EndPopup();
    }
}

void DrawToggleWithDoubleColor(const char* label, ConfigItem& item, float* visibleColor, bool showVisible) {
    DrawAnimatedToggle(label, &item.enabled);
    
    if (item.enabled) {
        ImGui::SameLine();
        if (showVisible) {
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 52);
            CustomColorEdit((std::string(label) + "_hid").c_str(), item.color);
            ImGui::SameLine();
            CustomColorEdit((std::string(label) + "_vis").c_str(), visibleColor);
        } else {
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
            CustomColorEdit(label, item.color);
        }
    }
}

void DrawToggleWithDoubleColorAndGear(const char* label, ConfigItem& item, float* visibleColor, bool showVisible, const char* gearId, std::function<void()> gearContent) {
    DrawAnimatedToggle(label, &item.enabled);
    
    if (item.enabled) {
        ImGui::SameLine();
        
        if (showVisible) {
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 78);
            CustomColorEdit((std::string(label) + "_hid").c_str(), item.color);
            ImGui::SameLine();
            CustomColorEdit((std::string(label) + "_vis").c_str(), visibleColor);
        } else {
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 52);
            CustomColorEdit(label, item.color);
        }
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton(gearId, nullptr)) {
            ImGui::OpenPopup(gearId);
        }
    }
    
    if (ImGui::BeginPopup(gearId)) {
        gearContent();
        ImGui::EndPopup();
    }
}

// === NEVERLOSE-STYLE THEME ===
void SetupNeverloseStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Spacing & Rounding
    style.Alpha = 1.0f;
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 22.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 8.0f;
    
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    
    style.WindowTitleAlign = ImVec2(0.02f, 0.5f);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    
    // Neverlose Dark Color Scheme
    ImVec4* colors = style.Colors;
    
    // Main
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);         // #1C1C1C
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);          // #232323
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    
    // Title
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.09f, 0.75f);
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    
    // Check/Radio
    colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.82f, 0.38f, 1.00f);        // #8FD162 (Neverlose Green)
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(0.56f, 0.82f, 0.38f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.63f, 0.90f, 0.45f, 1.00f);
    
    // Button
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    
    // Header
    colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    
    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.26f, 0.26f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.36f, 0.36f, 0.36f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
    
    // Tab
    colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    
    // Docking (if enabled) - Commented out for compatibility
    // colors[ImGuiCol_DockingPreview] = ImVec4(0.56f, 0.82f, 0.38f, 0.30f);
    // colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    
    // Plot
    colors[ImGuiCol_PlotLines] = ImVec4(0.56f, 0.82f, 0.38f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.63f, 0.90f, 0.45f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.56f, 0.82f, 0.38f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.63f, 0.90f, 0.45f, 1.00f);
    
    // Table
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    
    // Text Select
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.56f, 0.82f, 0.38f, 0.35f);
    
    // Drag Drop
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.56f, 0.82f, 0.38f, 0.90f);
    
    // Nav
    colors[ImGuiCol_NavHighlight] = ImVec4(0.56f, 0.82f, 0.38f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
}

void DrawBackGroundGrid(ImDrawList* drawList, ImVec2 pos, ImVec2 size) {
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(10, 10, 10, 255));
    ImU32 dotColor = IM_COL32(25, 25, 25, 255);
    float step = 5.0f;
    for (float x = 0; x < size.x; x += step) {
        for (float y = 0; y < size.y; y += step) {
            drawList->AddRectFilled(ImVec2(pos.x + x, pos.y + y), ImVec2(pos.x + x + 1.0f, pos.y + y + 1.0f), dotColor);
        }
    }
}

void DrawDecorations(ImDrawList* drawList, ImVec2 pos, ImVec2 size) {
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(10, 10, 10, 255), 0.0f, 0, 3.0f);
    static float time = 0.0f;
    time += 0.002f; if (time > 1.0f) time = 0.0f;
    int segments = 100;
    float segWidth = size.x / segments;
    float yTop = pos.y + 1.0f;
    for (int i = 0; i < segments; i++) {
        float hue = time + (float)i / (float)segments;
        if (hue > 1.0f) hue -= 1.0f;
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(hue, 0.8f, 1.0f, r, g, b);
        drawList->AddRectFilled(ImVec2(pos.x + i * segWidth + 1, yTop), ImVec2(pos.x + (i + 1) * segWidth - 1, yTop + 1.0f), ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f)));
    }
    drawList->AddRect(ImVec2(pos.x + 4, pos.y + 4), ImVec2(pos.x + size.x - 4, pos.y + size.y - 4), IM_COL32(45, 45, 45, 255), 0.0f, 0, 1.0f);
}

// --- MENUS ---

void DrawESPColumn(const char* title, ESPGroup& group) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild(title, ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), title);
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));
    DrawMasterSwitch("Master Switch", group.master, group.masterKey);
    ImGui::Separator();

    DrawAnimatedToggle("Visual Check", &group.visCheckEnabled);
    ImGui::Dummy(ImVec2(0, 2));

    // Bounding Box
    DrawToggleWithDoubleColor("Bounding Box", group.box, group.boxVisibleColor, group.visCheckEnabled);
    
    // Name
    DrawToggleWithColor("Name", group.name);
    
    // Skeleton
    DrawToggleWithDoubleColor("Skeleton", group.skeleton, group.skeletonVisibleColor, group.visCheckEnabled);

    // Overlay Chams with Gear
    std::string chamsGearId = std::string("##chamsGear_") + title;
    DrawToggleWithDoubleColorAndGear("Overlay Chams", group.chams, group.chamsVisibleColor, group.visCheckEnabled, chamsGearId.c_str(), [&]() {
        ImGui::Text("Chams Settings");
        ImGui::Separator();
        
        ImGui::Text("Mode");
        const char* chamsModes[] = { "Flat", "Gradient", "Wireframe", "Glow Only", "Pulse" };
        ImGui::Combo("##chamsmode", &group.chamsMode, chamsModes, IM_ARRAYSIZE(chamsModes));
        ImGui::Spacing();
        
        if (group.chamsMode == 1) {
            ImGui::Text("Gradient Colors");
            ImGui::ColorEdit4("Top Color", group.chamsGradientTop, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            ImGui::ColorEdit4("Bottom Color", group.chamsGradientBottom, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            ImGui::Spacing();
        }
        
        if (group.chamsMode == 4) {
            ImGui::Text("Pulse Speed");
            ImGui::SliderFloat("##pulsespeed", &group.chamsPulseSpeed, 0.5f, 10.0f, "Speed: %.1f");
            ImGui::Spacing();
        }
        
        ImGui::Text("Transparency");
        ImGui::SliderFloat("##chamsalpha", &group.chamsAlpha, 0.0f, 1.0f, "Alpha: %.2f");
        ImGui::Spacing();
        
        ImGui::Text("Outline");
        DrawAnimatedToggle("Enable Outline", &group.chamsOutline);
        if (group.chamsOutline) {
            ImGui::ColorEdit4("Invisible Color", group.chamsOutlineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            if (group.visCheckEnabled) {
                ImGui::ColorEdit4("Visible Color", group.chamsOutlineVisibleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
        }
        
        ImGui::Spacing();
        ImGui::Text("Glow");
        DrawAnimatedToggle("Enable Glow", &group.chamsGlow);
        if (group.chamsGlow) {
            ImGui::ColorEdit4("Invisible Glow", group.chamsGlowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            if (group.visCheckEnabled) {
                ImGui::ColorEdit4("Visible Glow", group.chamsGlowVisibleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            }
            DrawAnimatedToggle("Glow Only (No Fill)", &group.chamsGlowOnly);
        }
    });

    // Snaplines
    DrawToggleWithColor("Snaplines", group.snaplines);
    
    // Out of FOV Arrows
    DrawAnimatedToggle("Out of FOV", &group.outOfFovEnabled);
    if (group.outOfFovEnabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        std::string offovGearId = std::string("##offovGear_") + title;
        if (DrawGearButton(offovGearId.c_str(), "Out of FOV Settings")) {
            ImGui::OpenPopup(offovGearId.c_str());
        }
        
        if (ImGui::BeginPopup(offovGearId.c_str())) {
            ImGui::Text("Out of FOV Settings");
            ImGui::Separator();
            ImGui::ColorEdit4("Arrow Color", group.outOfFovColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            ImGui::SliderFloat("Arrow Size", &group.outOfFovSize, 10.0f, 50.0f, "%.0f");
            ImGui::SliderFloat("Distance", &group.outOfFovDistance, 50.0f, 200.0f, "%.0f");
            ImGui::EndPopup();
        }
    }
    
    // Ammo Bar
    DrawToggleWithColor("Ammo Bar", group.ammoBar);

    // Weapon with Gear
    std::string weaponGearId = std::string("##weaponGear_") + title;
    DrawAnimatedToggle("Weapon", &group.weapon.enabled);
    if (group.weapon.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton(weaponGearId.c_str(), "Weapon Settings")) {
            ImGui::OpenPopup(weaponGearId.c_str());
        }
    }
    
    if (ImGui::BeginPopup(weaponGearId.c_str())) {
        ImGui::Text("Weapon Settings");
        ImGui::Separator();
        DrawAnimatedToggle("Text", &group.weapon.text_enabled);
        ImGui::SameLine(); CustomColorEdit("##wpntxtcol", group.weapon.text_color);
        DrawAnimatedToggle("Icon", &group.weapon.icon_enabled);
        ImGui::SameLine(); CustomColorEdit("##wpniconcol", group.weapon.icon_color);
        ImGui::EndPopup();
    }

    // Sounds (Steps)
    DrawAnimatedToggle("Sounds (Steps)", &group.sound.enabled);
    if (group.sound.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        CustomColorEdit("##sndcol", group.sound.color);
    }

    // Armor Bar
    DrawAnimatedToggle("Armor Bar", &group.armor.enabled);
    if (group.armor.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        CustomColorEdit("##armorcol", group.armor.static_color);
    }

    // Health Bar with Gear
    std::string healthGearId = std::string("##healthGear_") + title;
    DrawAnimatedToggle("Health Bar", &group.health.enabled);
    if (group.health.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton(healthGearId.c_str(), "Health Bar Settings")) {
            ImGui::OpenPopup(healthGearId.c_str());
        }
    }
    
    if (ImGui::BeginPopup(healthGearId.c_str())) {
        ImGui::Text("Health Bar Settings");
        ImGui::Separator();
        const char* modes[] = { "Static", "Gradient" };
        ImGui::Combo("Mode", &group.health.mode, modes, IM_ARRAYSIZE(modes));
        if (group.health.mode == 0) {
            ImGui::ColorEdit4("Color", group.health.static_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        } else {
            ImGui::ColorEdit4("Low HP", group.health.gradient_low, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            ImGui::ColorEdit4("High HP", group.health.gradient_high, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        }
        ImGui::EndPopup();
    }

    // Flags with Gear
    std::string flagsGearId = std::string("##flagsGear_") + title;
    DrawAnimatedToggle("Flags", &group.flagsMaster.enabled);
    if (group.flagsMaster.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton(flagsGearId.c_str(), "Flags Settings")) {
            ImGui::OpenPopup(flagsGearId.c_str());
        }
    }
    
    if (ImGui::BeginPopup(flagsGearId.c_str())) {
        ImGui::Text("Flags Settings");
        ImGui::Separator();
        DrawFlagOption("MONEY", group.flagMoney);
        DrawFlagOption("ARMOR", group.flagArmor);
        DrawFlagOption("ZOOM", group.flagZoom);
        DrawFlagOption("KIT", group.flagKit);
        DrawFlagOption("BLIND", group.flagBlind);
        ImGui::EndPopup();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawWorldESPColumn() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild("World ESP", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "World ESP");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));

    DrawMasterSwitch("World Switch", world.master, world.masterKey);
    ImGui::Separator();
    ImGui::Spacing();

    DrawToggleWithColor("Bomb Indicator", world.bomb_indicator);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawMiscColumn() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild("Misc", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Misc");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));

    DrawMasterSwitch("Misc Switch", misc.master, misc.masterKey);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "General");
    DrawAnimatedToggle("Spectator List", &misc.spectator_list);
    DrawAnimatedToggle("Stream Proof", &misc.stream_proof);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Crosshair with Gear Settings
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Crosshair");
    DrawAnimatedToggle("Enable##Cross", &misc.crosshair.enabled);
    
    if (misc.crosshair.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 52);
        CustomColorEdit("##crosshaircol", misc.crosshair.color);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton("##crosshairGear", "Crosshair Settings")) {
            ImGui::OpenPopup("##crosshairGear");
        }
    }
    
    if (ImGui::BeginPopup("##crosshairGear")) {
        ImGui::Text("Crosshair Settings");
        ImGui::Separator();
        DrawAnimatedToggle("Outline", &misc.crosshair.outline);
        if (misc.crosshair.outline) {
            ImGui::SameLine(); CustomColorEdit("##xhairoutcol", misc.crosshair.outline_color);
        }
        DrawAnimatedToggle("Sniper Only", &misc.crosshair.sniper_only);
        DrawAnimatedToggle("Show When Scoped", &misc.crosshair.show_when_scoped);
        ImGui::SliderFloat("Length", &misc.crosshair.length, 2.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("Gap", &misc.crosshair.gap, 0.0f, 10.0f, "%.1f");
        ImGui::SliderFloat("Thickness", &misc.crosshair.thickness, 0.5f, 5.0f, "%.1f");
        
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Hitmarker with Gear Settings
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Hitmarker");
    DrawAnimatedToggle("Enable##Hit", &misc.hitmarker.enabled);
    
    if (misc.hitmarker.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 52);
        CustomColorEdit("##hitmarkercol", misc.hitmarker.color);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - 26);
        if (DrawGearButton("##hitmarkerGear", "Hitmarker Settings")) {
            ImGui::OpenPopup("##hitmarkerGear");
        }
    }
    
    if (ImGui::BeginPopup("##hitmarkerGear")) {
        ImGui::Text("Hitmarker Settings");
        ImGui::Separator();
        DrawAnimatedToggle("Outline", &misc.hitmarker.show_outline);
        if (misc.hitmarker.show_outline) {
            ImGui::SameLine(); CustomColorEdit("##hitmarkeroutlinecol", misc.hitmarker.outline_color);
        }
        ImGui::SliderFloat("Size", &misc.hitmarker.size, 5.0f, 25.0f, "%.1f");
        ImGui::SliderFloat("Gap", &misc.hitmarker.gap, 1.0f, 10.0f, "%.1f");
        ImGui::SliderFloat("Thickness", &misc.hitmarker.thickness, 0.5f, 5.0f, "%.1f");
        ImGui::SliderInt("Time(ms)", &misc.hitmarker.duration_ms, 100, 2000);
        if (misc.hitmarker.show_outline) {
            ImGui::SliderFloat("Outline Thick", &misc.hitmarker.outline_thickness, 0.5f, 5.0f, "%.1f");
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Hitsound
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Hitsound");
    DrawAnimatedToggle("Enable##Hitsound", &misc.hitsound_enabled);
    if (misc.hitsound_enabled) {
        ImGui::SliderFloat("Volume", &misc.hitsound_volume, 0.0f, 100.0f, "%.0f%%");
        
        // Seletor de hitsound
        ImGui::Text("Sound File:");
        if (ImGui::BeginCombo("##HitsoundSelect", availableHitsounds[selectedHitsoundIndex].c_str())) {
            for (int i = 0; i < availableHitsounds.size(); i++) {
                bool isSelected = (selectedHitsoundIndex == i);
                if (ImGui::Selectable(availableHitsounds[i].c_str(), isSelected)) {
                    selectedHitsoundIndex = i;
                    if (i == 0) {
                        currentHitsoundPath = "C:\\Windows\\Media\\Windows Ding.wav";
                    } else {
                        currentHitsoundPath = "hitsounds\\" + availableHitsounds[i];
                    }
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        
        if (ImGui::Button("Refresh Hitsounds")) {
            RefreshHitsoundList();
        }
        ImGui::SameLine();
        if (ImGui::Button("Test Sound")) {
            std::wstring wpath(currentHitsoundPath.begin(), currentHitsoundPath.end());
            PlaySound(wpath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Screen Color
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Screen Color");
    DrawAnimatedToggle("Enable##ScreenColor", &misc.screencolor_enabled);
    if (misc.screencolor_enabled) {
        ImGui::ColorEdit4("Color##Screen", misc.screencolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Radar
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Radar");
    DrawAnimatedToggle("Enable##Radar", &misc.radar_enabled);
    if (misc.radar_enabled) {
        ImGui::SliderFloat("Scale", &misc.radar_scale, 0.5f, 2.0f, "%.1f");
        ImGui::SliderFloat("Alpha", &misc.radar_alpha, 0.1f, 1.0f, "%.2f");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Effects
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Effects");
    DrawAnimatedToggle("Snow Effect", &misc.snow_effect);
    if (misc.snow_effect) {
        ImGui::SliderInt("Snow Amount", &misc.snow_amount, 50, 300, "%d flakes");
        ImGui::SliderFloat("Snow Speed", &misc.snow_speed, 0.5f, 3.0f, "%.1fx");
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawAimCategory(const char* categoryName, WeaponCategoryAim& category) {
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), categoryName);
    ImGui::Separator();
    ImGui::Spacing();
    
    // Aimbot
    ImGui::Checkbox("Aimbot", &category.aimbot_enabled);
    if (category.aimbot_enabled) {
        ImGui::Indent(10.0f);
        const char* bones[] = { "Pelvis", "Chest", "Head" };
        int boneIdx = (category.aimbot_bone == 0) ? 0 : (category.aimbot_bone == 4) ? 1 : 2;
        if (ImGui::Combo("Target Bone", &boneIdx, bones, 3)) {
            category.aimbot_bone = (boneIdx == 0) ? 0 : (boneIdx == 1) ? 4 : 6;
        }
        ImGui::SliderFloat("FOV", &category.aimbot_fov, 1.0f, 20.0f, "%.1f°");
        ImGui::SliderFloat("Smooth", &category.aimbot_smooth, 1.0f, 50.0f, "%.1f");
        ImGui::Text("Key: "); ImGui::SameLine();
        DrawKeybindWidget(category.aimbot_key);
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "Advanced:");
        ImGui::Checkbox("Silent Aim", &category.aimbot_silentaim);
        ImGui::Checkbox("Auto Penetration", &category.aimbot_autopenetration);
        ImGui::Checkbox("Auto Scope", &category.aimbot_autoscope);
        ImGui::Checkbox("Auto Stop", &category.aimbot_autostop);
        ImGui::SliderFloat("Hitchance", &category.aimbot_hitchance, 0.0f, 100.0f, "%.0f%%");
        ImGui::SliderFloat("Min Damage", &category.aimbot_mindamage, 1.0f, 120.0f, "%.0fhp");
        
        ImGui::Spacing();
        const char* multipoints[] = { "Off", "Low", "Medium", "High" };
        ImGui::Combo("Multipoint", &category.aimbot_multipoint, multipoints, 4);
        if (category.aimbot_multipoint > 0) {
            ImGui::SliderFloat("Head Scale", &category.aimbot_headscale, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Body Scale", &category.aimbot_bodyscale, 0.0f, 1.0f, "%.2f");
        }
        
        ImGui::Unindent(10.0f);
    }
    
    ImGui::Spacing();
    
    // Backtrack
    ImGui::Checkbox("Backtrack", &category.backtrack_enabled);
    if (category.backtrack_enabled) {
        ImGui::Indent(10.0f);
        ImGui::SliderInt("Ticks", &category.backtrack_ticks, 1, 15);
        ImGui::Unindent(10.0f);
    }
    
    ImGui::Spacing();
    
    // Resolver
    ImGui::Checkbox("Resolver", &category.resolver_enabled);
    if (category.resolver_enabled) {
        ImGui::Indent(10.0f);
        const char* resolvers[] = { "Basic", "Advanced", "Experimental" };
        ImGui::Combo("Mode", &category.resolver_mode, resolvers, 3);
        ImGui::Unindent(10.0f);
    }
    
    ImGui::Spacing();
    
    // Triggerbot
    ImGui::Checkbox("Triggerbot", &category.triggerbot_enabled);
    if (category.triggerbot_enabled) {
        ImGui::Indent(10.0f);
        ImGui::SliderInt("Delay (ms)", &category.triggerbot_delay, 0, 500);
        ImGui::Text("Key: "); ImGui::SameLine();
        DrawKeybindWidget(category.triggerbot_key);
        
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.8f, 0.9f, 1.0f), "Advanced:");
        ImGui::Checkbox("Burst Fire", &category.triggerbot_burstfire);
        if (category.triggerbot_burstfire) {
            ImGui::SliderInt("Burst Shots", &category.triggerbot_burstshots, 1, 10);
        }
        ImGui::Checkbox("Magnet", &category.triggerbot_magnet);
        ImGui::SliderFloat("Hitchance##trig", &category.triggerbot_hitchance, 0.0f, 100.0f, "%.0f%%");
        
        ImGui::Unindent(10.0f);
    }
    
    ImGui::Spacing();
    
    // Autofire
    ImGui::Checkbox("Autofire", &category.autofire_enabled);
    
    ImGui::Spacing();
    
    // RCS
    ImGui::Checkbox("Recoil Control", &category.rcs_enabled);
    if (category.rcs_enabled) {
        ImGui::Indent(10.0f);
        ImGui::SliderFloat("RCS X", &category.rcs_x, 0.0f, 100.0f, "%.0f%%");
        ImGui::SliderFloat("RCS Y", &category.rcs_y, 0.0f, 100.0f, "%.0f%%");
        ImGui::Checkbox("Standalone (works without aimbot)", &category.rcs_standalone);
        ImGui::Unindent(10.0f);
    }
}

void DrawAimTab() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    
    ImGui::BeginChild("AimMain", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "AIM Settings");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));
    
    DrawMasterSwitch("AIM Master", aim.master, aim.masterKey);
    ImGui::Separator();
    ImGui::Spacing();
    
    // 6 categorias em abas
    if (ImGui::BeginTabBar("WeaponCategories", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Pistol")) {
            DrawAimCategory("Pistols", aim.pistol);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("SMG")) {
            DrawAimCategory("SMGs", aim.smg);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Rifle")) {
            DrawAimCategory("Rifles", aim.rifle);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sniper")) {
            DrawAimCategory("Snipers", aim.sniper);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Shotgun")) {
            DrawAimCategory("Shotguns", aim.shotgun);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Heavy")) {
            DrawAimCategory("Heavy Weapons", aim.heavy);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawSettingsColumn() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild("Settings", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Map Settings");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Map Detection");
    DrawAnimatedToggle("Auto Detect Map", &autoDetectMap);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!autoDetectMap) {
        ImGui::Text("Manual Map Selection");
        std::vector<const char*> mapItems;
        for (const auto& m : availableMaps) mapItems.push_back(m.c_str());

        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##MapSelect", &selectedMapIndex, mapItems.data(), (int)mapItems.size());

        ImGui::Dummy(ImVec2(0, 5));

        if (ImGui::Button("Load Map", ImVec2(150, 25))) {
            if (!availableMaps.empty() && availableMaps[0] != "No files found") {
                std::string path = "maps/" + availableMaps[selectedMapIndex];
                if (std::filesystem::exists(path)) {
                    vCheck = VisCheck(path);
                    currentMapName = availableMaps[selectedMapIndex];
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh", ImVec2(100, 25))) RefreshMapList();
    }
    else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Auto Detection Enabled");
        if (!currentMapName.empty()) ImGui::Text("Detected: %s", currentMapName.c_str());
        else ImGui::Text("Waiting for map...");
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

int GetMaxClip(std::string name) {
    if (name.find("negev") != std::string::npos) return 150;
    if (name.find("m249") != std::string::npos) return 100;
    if (name.find("bizon") != std::string::npos) return 64;
    if (name.find("p90") != std::string::npos) return 50;
    if (name.find("galil") != std::string::npos) return 35;
    if (name.find("ak47") != std::string::npos) return 30;
    if (name.find("mac10") != std::string::npos) return 30;
    if (name.find("aug") != std::string::npos) return 30;
    if (name.find("sg556") != std::string::npos) return 30;
    if (name.find("elite") != std::string::npos) return 30;
    if (name.find("mp9") != std::string::npos) return 30;
    if (name.find("mp7") != std::string::npos) return 30;
    if (name.find("mp5") != std::string::npos) return 30;
    if (name.find("silencer") != std::string::npos && name.find("m4a1") != std::string::npos) return 20;
    if (name.find("m4a1") != std::string::npos) return 30;
    if (name.find("famas") != std::string::npos) return 25;
    if (name.find("ump45") != std::string::npos) return 25;
    if (name.find("glock") != std::string::npos) return 20;
    if (name.find("fiveseven") != std::string::npos) return 20;
    if (name.find("scar20") != std::string::npos) return 20;
    if (name.find("g3sg1") != std::string::npos) return 20;
    if (name.find("tec9") != std::string::npos) return 18;
    if (name.find("p2000") != std::string::npos) return 13;
    if (name.find("hkp2000") != std::string::npos) return 13;
    if (name.find("p250") != std::string::npos) return 13;
    if (name.find("usp") != std::string::npos) return 12;
    if (name.find("cz75") != std::string::npos) return 12;
    if (name.find("ssg08") != std::string::npos) return 10;
    if (name.find("revolver") != std::string::npos) return 8;
    if (name.find("nova") != std::string::npos) return 8;
    if (name.find("deagle") != std::string::npos) return 7;
    if (name.find("sawedoff") != std::string::npos) return 7;
    if (name.find("xm1014") != std::string::npos) return 7;
    if (name.find("mag7") != std::string::npos) return 5;
    if (name.find("awp") != std::string::npos) return 5;
    if (name.find("knife") != std::string::npos || name.find("bayonet") != std::string::npos || name.find("taser") != std::string::npos) return 0;
    return 30;
}

void DrawSpectatorList(const std::vector<Entity>& spectators) {
    if (spectators.empty() || !misc.spectator_list) return;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::SetNextWindowSize(ImVec2(250, 200));
    ImGui::Begin("Spectator List", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "SPECTATORS");
    ImGui::Separator();
    for (const Entity& entity : spectators) {
        ImGui::Text(entity.name.c_str());
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void DrawCrosshair(ImDrawList* drawList, int monitorWidth, int monitorHeight, const CrosshairSettings& settings, const std::string& weaponName, bool isScoped) {
    if (!settings.enabled) return;
    if (settings.sniper_only) {
        bool isSniper = (weaponName == "awp" || weaponName == "ssg08" || weaponName == "g3sg1" || weaponName == "scar20");
        if (!isSniper) return;
    }
    if (!settings.show_when_scoped && isScoped) return;
    
    ImVec2 center{ monitorWidth / 2.0f, monitorHeight / 2.0f };
    ImU32 color = ColorToU32(const_cast<float*>(settings.color));
    ImU32 outlineColor = ColorToU32(const_cast<float*>(settings.outline_color));
    
    if (settings.outline) {
        float t = settings.thickness + 2.0f;
        float l = settings.length + 1.0f;
        drawList->AddLine({ center.x - l - settings.gap, center.y }, { center.x - settings.gap, center.y }, outlineColor, t);
        drawList->AddLine({ center.x + settings.gap, center.y }, { center.x + l + settings.gap, center.y }, outlineColor, t);
        drawList->AddLine({ center.x, center.y - l - settings.gap }, { center.x, center.y - settings.gap }, outlineColor, t);
        drawList->AddLine({ center.x, center.y + settings.gap }, { center.x, center.y + l + settings.gap }, outlineColor, t);
    }
    
    drawList->AddLine({ center.x - settings.length - settings.gap, center.y }, { center.x - settings.gap, center.y }, color, settings.thickness);
    drawList->AddLine({ center.x + settings.gap, center.y }, { center.x + settings.length + settings.gap, center.y }, color, settings.thickness);
    drawList->AddLine({ center.x, center.y - settings.length - settings.gap }, { center.x, center.y - settings.gap }, color, settings.thickness);
    drawList->AddLine({ center.x, center.y + settings.gap }, { center.x, center.y + settings.length + settings.gap }, color, settings.thickness);
}

void DrawHitmarker(ImDrawList* drawList, int monitorWidth, int monitorHeight, HitmarkerSettings& settings, uintptr_t bulletServices, Memory& cs2, ULONGLONG& hitMarkerLastTick, int& hitMarkerPrevTotalHits) {
    if (!settings.enabled) return;
    int totalHits = cs2.read<int>(bulletServices + Offsets::m_totalHitsOnServer);
    static float hitMarkerAlpha = 0.0f;
    if (totalHits != hitMarkerPrevTotalHits) {
        if (totalHits > hitMarkerPrevTotalHits) {
            hitMarkerAlpha = 1.0f;
            hitMarkerLastTick = GetTickCount64();
        }
    }
    else if (totalHits == 0 && hitMarkerPrevTotalHits != 0) hitMarkerAlpha = 0.0f;
    hitMarkerPrevTotalHits = totalHits;

    if (hitMarkerAlpha > 0.0f) {
        ULONGLONG currTicks = GetTickCount64();
        if (currTicks - hitMarkerLastTick >= settings.duration_ms) hitMarkerAlpha = 0.0f;
        else hitMarkerAlpha = 1.0f - ((float)(currTicks - hitMarkerLastTick) / settings.duration_ms);

        ImVec2 center{ monitorWidth / 2.0f, monitorHeight / 2.0f };
        ImU32 color = ColorToU32(settings.color);
        color = (color & ~IM_COL32_A_MASK) | (ImU32)(hitMarkerAlpha * 255) << IM_COL32_A_SHIFT;
        ImU32 outlineColor = ColorToU32(settings.outline_color);
        outlineColor = (outlineColor & ~IM_COL32_A_MASK) | (ImU32)(hitMarkerAlpha * 255) << IM_COL32_A_SHIFT;

        if (settings.show_outline && settings.outline_thickness > 0) {
            drawList->AddLine({ center.x - settings.size, center.y - settings.size }, { center.x - settings.gap, center.y - settings.gap }, outlineColor, settings.outline_thickness);
            drawList->AddLine({ center.x - settings.size, center.y + settings.size }, { center.x - settings.gap, center.y + settings.gap }, outlineColor, settings.outline_thickness);
            drawList->AddLine({ center.x + settings.size, center.y - settings.size }, { center.x + settings.gap, center.y - settings.gap }, outlineColor, settings.outline_thickness);
            drawList->AddLine({ center.x + settings.size, center.y + settings.size }, { center.x + settings.gap, center.y + settings.gap }, outlineColor, settings.outline_thickness);
        }
        drawList->AddLine({ center.x - settings.size, center.y - settings.size }, { center.x - settings.gap, center.y - settings.gap }, color, settings.thickness);
        drawList->AddLine({ center.x - settings.size, center.y + settings.size }, { center.x - settings.gap, center.y + settings.gap }, color, settings.thickness);
        drawList->AddLine({ center.x + settings.size, center.y - settings.size }, { center.x + settings.gap, center.y - settings.gap }, color, settings.thickness);
        drawList->AddLine({ center.x + settings.size, center.y + settings.size }, { center.x + settings.gap, center.y + settings.gap }, color, settings.thickness);
    }
}

// === HIGH-FIDELITY ANATOMICAL CHAMS HELPERS ===

inline long long cross_product(ImVec2 a, ImVec2 b, ImVec2 c) {
    return (long long)(b.x - a.x) * (c.y - a.y) - (long long)(b.y - a.y) * (c.x - a.x);
}

std::vector<ImVec2> compute_convex_hull(std::vector<ImVec2>& points) {
    if (points.size() <= 3) return points;
    
    std::sort(points.begin(), points.end(), [](const ImVec2& a, const ImVec2& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    
    std::vector<ImVec2> upper_hull, lower_hull;
    upper_hull.reserve(points.size() / 2);
    lower_hull.reserve(points.size() / 2);
    
    for (const auto& p : points) {
        while (upper_hull.size() >= 2 && cross_product(upper_hull[upper_hull.size() - 2], upper_hull.back(), p) <= 0) {
            upper_hull.pop_back();
        }
        upper_hull.push_back(p);
    }
    
    for (int i = (int)points.size() - 1; i >= 0; i--) {
        const auto& p = points[i];
        while (lower_hull.size() >= 2 && cross_product(lower_hull[lower_hull.size() - 2], lower_hull.back(), p) <= 0) {
            lower_hull.pop_back();
        }
        lower_hull.push_back(p);
    }
    
    std::vector<ImVec2> hull;
    hull.reserve(upper_hull.size() + lower_hull.size());
    hull.insert(hull.end(), upper_hull.begin(), upper_hull.end());
    hull.insert(hull.end(), lower_hull.begin() + 1, lower_hull.end() - 1);
    
    return hull;
}

std::vector<ImVec2> generate_capsule_points(ImVec2 p1, ImVec2 p2, float radius, int segments = 16) {
    std::vector<ImVec2> points;
    points.reserve(segments + 2);
    
    const float dx = p2.x - p1.x;
    const float dy = p2.y - p1.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    
    if (dist < 0.1f) {
        for (int i = 0; i < segments; i++) {
            const float angle = (float)i / segments * Math::TWO_PI;
            points.push_back(ImVec2(p1.x + radius * std::cos(angle), p1.y + radius * std::sin(angle)));
        }
        return points;
    }
    
    const float angle = std::atan2(dy, dx);
    
    for (int i = 0; i <= segments / 2; i++) {
        const float t = (float)i / (segments / 2);
        const float currentAngle = angle + Math::HALF_PI + t * Math::PI;
        points.push_back(ImVec2(p1.x + radius * std::cos(currentAngle), p1.y + radius * std::sin(currentAngle)));
    }
    
    for (int i = 0; i <= segments / 2; i++) {
        const float t = (float)i / (segments / 2);
        const float currentAngle = angle - Math::HALF_PI + t * Math::PI;
        points.push_back(ImVec2(p2.x + radius * std::cos(currentAngle), p2.y + radius * std::sin(currentAngle)));
    }
    
    return points;
}

// === NEW WORLD ESP DRAWING FUNCTIONS ===

// === WORLD ESP FUNCTIONS (Somente Bomb Indicator) ===
// Dropped Weapons, Projectiles e Grenade Radius foram removidos

void DrawWeaponSpread(ImDrawList* drawList, int monitorWidth, int monitorHeight, float spread) {
    // Função removida - spread foi removido
    return;
}

void DrawScreenColorOverlay(ImDrawList* drawList, int monitorWidth, int monitorHeight) {
    if (!misc.screencolor_enabled) return;
    
    // Desenhar ANTES de tudo para não afetar as features
    ImU32 color = ColorToU32(misc.screencolor);
    
    // Criar um drawlist separado que será renderizado primeiro
    ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList();
    bgDrawList->AddRectFilled(ImVec2(0, 0), ImVec2((float)monitorWidth, (float)monitorHeight), color);
}

void DrawSnowEffect(ImDrawList* drawList) {
    if (!misc.snow_effect) return;
    
    for (const auto& flake : snowFlakes) {
        ImU32 color = IM_COL32(255, 255, 255, (int)(flake.opacity * 255));
        drawList->AddCircleFilled(ImVec2(flake.x, flake.y), flake.size, color);
    }
}

void DrawRadar(ImDrawList* drawList, int monitorWidth, int monitorHeight, const Entity& localPlayer, const std::vector<Entity>& entities, Memory& cs2, uintptr_t localPawn) {
    if (!misc.radar_enabled) return;
    
    float radarSize = 180.0f * misc.radar_scale;
    ImVec2 radarPos(monitorWidth - radarSize - 30.0f, 100.0f);
    ImVec2 radarCenter(radarPos.x + radarSize / 2.0f, radarPos.y + radarSize / 2.0f);
    
    ImU32 bgColor = IM_COL32(10, 10, 10, (int)(220 * misc.radar_alpha));
    ImU32 borderColor = IM_COL32(60, 180, 255, (int)(255 * misc.radar_alpha));
    ImU32 gridColor = IM_COL32(255, 255, 255, (int)(30 * misc.radar_alpha));
    
    // Background com sombra
    drawList->AddRectFilled(ImVec2(radarPos.x + 3, radarPos.y + 3), ImVec2(radarPos.x + radarSize + 3, radarPos.y + radarSize + 3), IM_COL32(0, 0, 0, 120), 8.0f);
    drawList->AddRectFilled(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), bgColor, 8.0f);
    drawList->AddRect(radarPos, ImVec2(radarPos.x + radarSize, radarPos.y + radarSize), borderColor, 8.0f, 0, 2.5f);
    
    // Grid
    drawList->AddLine(ImVec2(radarCenter.x, radarPos.y + 5), ImVec2(radarCenter.x, radarPos.y + radarSize - 5), gridColor, 1.0f);
    drawList->AddLine(ImVec2(radarPos.x + 5, radarCenter.y), ImVec2(radarPos.x + radarSize - 5, radarCenter.y), gridColor, 1.0f);
    
    // Círculos de distância
    drawList->AddCircle(radarCenter, radarSize * 0.25f, gridColor, 32, 1.0f);
    drawList->AddCircle(radarCenter, radarSize * 0.5f, gridColor, 32, 1.0f);
    
    // Ler direção do player local
    Vector2 localEyeAngles = cs2.read<Vector2>(localPawn + Offsets::m_angEyeAngles);
    float localYaw = localEyeAngles.y * (3.14159f / 180.0f);
    
    // Desenhar linha de direção do player local (LINHA BRANCA)
    float dirLength = 30.0f;
    float localDirX = radarCenter.x + std::cos(localYaw - 1.5708f) * dirLength;
    float localDirY = radarCenter.y - std::sin(localYaw - 1.5708f) * dirLength;
    drawList->AddLine(radarCenter, ImVec2(localDirX, localDirY), IM_COL32(255, 255, 255, 255), 3.0f);
    
    // Player local (círculo verde no centro)
    drawList->AddCircleFilled(radarCenter, 5.0f, IM_COL32(0, 255, 100, 255), 16);
    drawList->AddCircle(radarCenter, 5.0f, IM_COL32(255, 255, 255, 200), 16, 1.5f);
    
    // Desenhar outros players
    for (const auto& ent : entities) {
        if (ent.health <= 0) continue;
        
        float dx = ent.position.x - localPlayer.position.x;
        float dy = ent.position.y - localPlayer.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance > 4000.0f) continue;
        
        float scale = radarSize / 8000.0f;
        float dotX = radarCenter.x + dx * scale;
        float dotY = radarCenter.y - dy * scale;
        
        // Limitar ao radar
        float maxDist = radarSize / 2.0f - 8.0f;
        float distFromCenter = std::sqrt((dotX - radarCenter.x) * (dotX - radarCenter.x) + (dotY - radarCenter.y) * (dotY - radarCenter.y));
        if (distFromCenter > maxDist) {
            float ratio = maxDist / distFromCenter;
            dotX = radarCenter.x + (dotX - radarCenter.x) * ratio;
            dotY = radarCenter.y + (dotY - radarCenter.y) * ratio;
        }
        
        ImU32 dotColor = (ent.team == localPlayer.team) ? IM_COL32(50, 150, 255, 255) : IM_COL32(255, 50, 50, 255);
        
        // Desenhar círculo do jogador
        drawList->AddCircleFilled(ImVec2(dotX, dotY), 4.5f, dotColor, 12);
        drawList->AddCircle(ImVec2(dotX, dotY), 4.5f, IM_COL32(0, 0, 0, 180), 12, 1.2f);
        
        // Desenhar linha de direção
        try {
            Vector2 eyeAngles = cs2.read<Vector2>(ent.currentPawn + Offsets::m_angEyeAngles);
            float yaw = eyeAngles.y * (3.14159f / 180.0f);
            
            float playerDirLength = 10.0f;
            float dirX = dotX + std::cos(yaw - 1.5708f) * playerDirLength;
            float dirY = dotY - std::sin(yaw - 1.5708f) * playerDirLength;
            
            drawList->AddLine(ImVec2(dotX, dotY), ImVec2(dirX, dirY), dotColor, 2.0f);
        }
        catch(...) {
            // Se falhar ao ler ângulo, apenas desenha o ponto
        }
    }
    
    // Label
    char radarText[32];
    sprintf_s(radarText, "RADAR");
    ImVec2 textSize = ImGui::CalcTextSize(radarText);
    drawList->AddText(ImVec2(radarCenter.x - textSize.x / 2, radarPos.y - 18), IM_COL32(200, 200, 200, 200), radarText);
}


// --- MAIN LOOP ---
int main()
{
    InitKeyNames();
    RefreshMapList();
    RefreshHitsoundList();

    DWORD cs2PID{ getPID(L"cs2.exe") };
    if (cs2PID == 0) { std::cout << "Failed to find cs2 process.\n"; system("pause"); return 1; }

    int offsetsChoice{ 0 };
    while (true) {
        std::cout << "1. Get offsets online.\n2. Dump offsets.\n> ";
        std::cin >> offsetsChoice;
        
        // Limpar buffer de entrada
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        
        if (offsetsChoice == 1 || offsetsChoice == 2) break;
        std::cout << "Opcao invalida! Escolha 1 ou 2.\n";
    }
    
    std::cout << "\n[INFO] Processando offsets...\n";
    
    bool offsetsSuccess = false;
    try {
        if (offsetsChoice == 1) { 
            std::cout << "[INFO] Conectando aos servidores...\n";
            offsetsSuccess = Offsets::getOffsetsOnline();
            if (offsetsSuccess) {
                std::cout << "[SUCCESS] Offsets obtidos online com sucesso!\n";
            } else {
                std::cout << "[ERROR] Falha ao obter offsets online.\n";
                std::cout << "[INFO] Verifique sua conexao com a internet.\n";
            }
        }
        else { 
            std::cout << "[INFO] Baixando dumper...\n";
            offsetsSuccess = Offsets::dumpOffsets();
            if (offsetsSuccess) {
                std::cout << "[SUCCESS] Offsets extraidos com sucesso!\n";
            } else {
                std::cout << "[ERROR] Falha ao extrair offsets.\n";
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "[EXCEPTION] Erro ao processar offsets: " << e.what() << std::endl;
        offsetsSuccess = false;
    }
    catch (...) {
        std::cout << "[EXCEPTION] Erro desconhecido ao processar offsets.\n";
        offsetsSuccess = false;
    }
    
    if (!offsetsSuccess) {
        std::cout << "\n[FATAL] Nao foi possivel carregar os offsets.\n";
        std::cout << "O programa nao pode continuar sem os offsets.\n";
        system("pause");
        return 1;
    }
    
    std::cout << "[INFO] Offsets carregados!\n";
    Sleep(1500);
    system("cls");

    Memory cs2{ cs2PID };
    if (!cs2.m_hProc) {
        std::cout << "[ERROR] Failed to open CS2 process handle.\n";
        system("pause");
        return 1;
    }
    
    uintptr_t client{ getModuleBase(cs2PID, L"client.dll") };
    if (!client) {
        std::cout << "[ERROR] Failed to find client.dll module.\n";
        system("pause");
        return 1;
    }

    int monitorWidth{ GetSystemMetrics(SM_CXSCREEN) };
    int monitorHeight{ GetSystemMetrics(SM_CYSCREEN) };

    Renderer renderer;
    renderer.Init();

    std::cout << "[INFO] Procurando janela do CS2...\n";
    HWND cs2Window = FindWindow(NULL, L"Counter-Strike 2");
    if (!cs2Window) {
        std::cout << "[WARNING] Janela do CS2 nao encontrada. O programa continuara, mas algumas funcoes podem nao funcionar.\n";
    } else {
        std::cout << "[SUCCESS] Janela do CS2 encontrada!\n";
    }
    NtUserInjectMouseInput::InitInjectMouseInput();
    ImGuiIO& imguiIo{ ImGui::GetIO() };

    ImFontConfig font_config;
    font_config.PixelSnapH = true;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.RasterizerMultiply = 1.0f;

    std::cout << "[INFO] Carregando fontes...\n";
    
    imguiIo.Fonts->AddFontDefault();
    imguiIo.Fonts->AddFontFromMemoryTTF(weaponFont, weaponFontSize, 20.0f);
    imguiIo.Fonts->AddFontFromMemoryTTF(firaCodeFont, firaCodeFontSize, 16.0f);

    pixelFont = imguiIo.Fonts->AddFontFromFileTTF("smallest_pixel-7.ttf", 11.0f, &font_config);
    if (!pixelFont) {
        std::cout << "[WARNING] Fonte smallest_pixel-7.ttf nao encontrada, usando fonte padrao.\n";
        pixelFont = imguiIo.Fonts->AddFontDefault();
    }
    
    gamesenseFont = imguiIo.Fonts->AddFontFromFileTTF("smallest_pixel-7.ttf", 11.0f, &font_config);
    if (!gamesenseFont) gamesenseFont = pixelFont;
    
    tahomaBoldFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 13.0f, &font_config);
    if (!tahomaBoldFont) {
        tahomaBoldFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 13.0f, &font_config);
        if (!tahomaBoldFont) {
            std::cout << "[WARNING] Fontes Tahoma nao encontradas, usando fonte padrao.\n";
            tahomaBoldFont = imguiIo.Fonts->AddFontDefault();
        }
    }
    
    tahomaRegularFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 13.0f, &font_config);
    if (!tahomaRegularFont) tahomaRegularFont = tahomaBoldFont;
    
    smallFont = tahomaBoldFont;

    team.box.color[0] = 0.0f; team.box.color[1] = 1.0f; team.box.color[2] = 0.0f; team.box.color[3] = 1.0f;

    SetupNeverloseStyle();

    std::cout << "[INFO] Overlay initialized successfully.\n";
    std::cout << "[INFO] Press INSERT to show/hide menu.\n";
    std::cout << "[INFO] Monitoring CS2 process...\n";
    std::cout << "\n======================================\n";
    std::cout << "  VORTIX PRO CARREGADO COM SUCESSO!\n";
    std::cout << "======================================\n\n";
    Sleep(2000);

    bool wasVisible = false;
    ULONGLONG hitMarkerLastTick{ GetTickCount64() };
    int hitMarkerPrevTotalHits{ 0 };
    std::vector<Entity> spectatingPlrs;

    std::cout << "[INFO] Entrando no loop principal...\n";

    while (true)
    {
        try {
            // Check if CS2 process is still running
            DWORD exitCode = 0;
            if (GetExitCodeProcess(cs2.m_hProc, &exitCode) && exitCode != STILL_ACTIVE) {
                std::cout << "[ERROR] CS2 process has closed. Exiting...\n";
                Sleep(2000);
                break;
            }

            UpdateKeybind(enemy.masterKey);
            UpdateKeybind(team.masterKey);
            UpdateKeybind(world.masterKey);
            UpdateKeybind(misc.masterKey);
            UpdateKeybind(aim.masterKey);

            if (GetAsyncKeyState(VK_INSERT) & 1) Globals::imguiVisible = !Globals::imguiVisible;

            if (misc.stream_proof) SetWindowDisplayAffinity(renderer.hwnd, WDA_EXCLUDEFROMCAPTURE);
            else SetWindowDisplayAffinity(renderer.hwnd, WDA_NONE);

            if (Globals::imguiVisible != wasVisible) {
                if (Globals::imguiVisible) {
                    SetWindowLong(renderer.hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
                    SetForegroundWindow(renderer.hwnd);
                }
                else {
                    LONG style = WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED;
                    SetWindowLong(renderer.hwnd, GWL_EXSTYLE, style);
                    SetForegroundWindow(cs2Window);
                }
                wasVisible = Globals::imguiVisible;
            }

            renderer.StartRender();
        }
        catch (const std::exception& e) {
            std::cout << "[EXCEPTION] Error in main loop: " << e.what() << std::endl;
            Sleep(100);
            continue;
        }
        catch (...) {
            std::cout << "[EXCEPTION] Unknown error in main loop" << std::endl;
            Sleep(100);
            continue;
        }

        if (Globals::imguiVisible)
        {
            ImGui::PushFont(tahomaBoldFont);
            ImGui::SetNextWindowSize(ImVec2(900, 600));
            ImGui::Begin("VORTIX PRO", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            {
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 windowSize = ImGui::GetWindowSize();
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                
                // Neverlose-style Top Bar with Logo
                drawList->AddRectFilled(
                    ImVec2(windowPos.x, windowPos.y), 
                    ImVec2(windowPos.x + windowSize.x, windowPos.y + 45),
                    IM_COL32(16, 16, 16, 255)
                );
                
                // Logo Circle (Neverlose-style)
                drawList->AddCircleFilled(ImVec2(windowPos.x + 25, windowPos.y + 22), 12, IM_COL32(143, 209, 98, 255), 32);
                drawList->AddText(tahomaBoldFont, 16.0f, ImVec2(windowPos.x + 20, windowPos.y + 15), IM_COL32(30, 30, 30, 255), "V");
                
                // Title
                drawList->AddText(tahomaBoldFont, 16.0f, ImVec2(windowPos.x + 48, windowPos.y + 14), IM_COL32(230, 230, 230, 255), "VORTIX PRO");
                drawList->AddText(tahomaRegularFont, 11.0f, ImVec2(windowPos.x + 48, windowPos.y + 30), IM_COL32(130, 130, 130, 255), "Premium CS2 External");
                
                // Top Bar Buttons
                ImGui::SetCursorPos(ImVec2(windowSize.x - 120, 10));
                if (ImGui::Button("Save##TopBar", ImVec2(50, 25))) {
                    // Save config logic
                }
                ImGui::SameLine();
                if (ImGui::Button("Load##TopBar", ImVec2(50, 25))) {
                    // Load config logic
                }
                
                // Separator Line
                drawList->AddLine(
                    ImVec2(windowPos.x, windowPos.y + 45),
                    ImVec2(windowPos.x + windowSize.x, windowPos.y + 45),
                    IM_COL32(40, 40, 40, 255),
                    1.0f
                );
                
                // Content Area
                ImGui::SetCursorPosY(55);
                
                if (ImGui::BeginTabBar("##MainTabBar", ImGuiTabBarFlags_None)) {
                    // AIM TAB (PRIMEIRA)
                    if (ImGui::BeginTabItem("AIM")) {
                        ImGui::Spacing();
                        ImGui::Columns(1, nullptr, false);
                        DrawAimTab();
                        ImGui::Columns(1);
                        ImGui::EndTabItem();
                    }
                    
                    // VISUALS TAB (SEGUNDA)
                    if (ImGui::BeginTabItem("Visuals")) {
                        ImGui::Spacing();
                        ImGui::Columns(4, nullptr, false);
                        
                        DrawESPColumn("Enemy ESP", enemy);
                        ImGui::NextColumn();
                        DrawESPColumn("Team ESP", team);
                        ImGui::NextColumn();
                        DrawWorldESPColumn();
                        ImGui::NextColumn();
                        DrawMiscColumn();
                        
                        ImGui::Columns(1);
                        ImGui::EndTabItem();
                    }
                    
                    // SETTINGS TAB (TERCEIRA)
                    if (ImGui::BeginTabItem("Settings")) {
                        ImGui::Spacing();
                        ImGui::Columns(1, nullptr, false);
                        DrawSettingsColumn();
                        ImGui::Columns(1);
                        ImGui::EndTabItem();
                    }
                    
                    ImGui::EndTabBar();
                }
            }
            ImGui::End();
            ImGui::PopFont();
        }

        ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };
        
        // Screen Color Overlay - DESENHAR PRIMEIRO para não afetar features
        if (misc.master.enabled && misc.masterKey.active && misc.screencolor_enabled) {
            DrawScreenColorOverlay(drawList, monitorWidth, monitorHeight);
        }

        try {
            Matrix viewMatrix = cs2.read<Matrix>(client + Offsets::dwViewMatrix);
            uintptr_t entityList = cs2.read<uintptr_t>(client + Offsets::dwEntityList);
            uintptr_t playerPawnAddress = cs2.read<uintptr_t>(client + Offsets::dwLocalPlayerPawn);
            
            // Validate addresses before proceeding
            if (!playerPawnAddress || !entityList || !client) {
                renderer.EndRender();
                Sleep(10);
                continue;
            }
            
            Entity localPlayerEntity{ cs2, playerPawnAddress };
            bool isLocalPlayerScoped = cs2.read<bool>(playerPawnAddress + FlagOffsets::m_bIsScoped);

            UpdateMapVisCheck(cs2, client);

        spectatingPlrs.clear();

        if (world.master.enabled && world.masterKey.active) {
            if (world.bomb_indicator.enabled) {
                try {
                    uintptr_t globalVars = cs2.read<uintptr_t>(client + BombOffsets::dwGlobalVars);
                    if (!globalVars) goto skip_bomb_indicator;
                    
                    float serverTime = cs2.read<float>(globalVars + 0x30);
                Globals::isPlanting = false;
                for (int i = 0; i < 64; i++) {
                    uintptr_t listEntry = cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16);
                    if (!listEntry) continue;
                    uintptr_t currentController = cs2.read<uintptr_t>(listEntry + 112 * (i & 0x1FF));
                    if (!currentController) continue;
                    int pawnHandle = cs2.read<int>(currentController + Offsets::m_hPlayerPawn);
                    if (pawnHandle == 0) continue;
                    uintptr_t listEntry2 = cs2.read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16);
                    uintptr_t currentPawn = cs2.read<uintptr_t>(listEntry2 + 112 * (pawnHandle & 0x1FF));
                    if (currentPawn == playerPawnAddress) continue;
                    int teamNum = cs2.read<int>(currentPawn + Offsets::m_iTeamNum);
                    if (teamNum != localPlayerEntity.team) {
                        if (cs2.read<bool>(currentPawn + FlagOffsets::m_bIsPlanting)) {
                            Globals::isPlanting = true; break;
                        }
                    }
                }
                if (Globals::isPlanting) {
                    DrawSkeetText(drawList, tahomaBoldFont, 24.0f, { (float)monitorWidth / 2.0f, (float)monitorHeight / 3.0f }, IM_COL32(255, 0, 0, 255), "ENEMY PLANTING", true);
                }
                bool isPlanted = false;
                uintptr_t gameRules = cs2.read<uintptr_t>(client + BombOffsets::dwGameRules);
                if (gameRules) isPlanted = cs2.read<bool>(gameRules + BombOffsets::m_bBombPlanted);
                if (!isPlanted) { Globals::isBombPlanted = false; Globals::bombTimeLeft = 0.0f; }
                else {
                    Globals::isBombPlanted = true;
                    uintptr_t plantedC4Address = cs2.read<uintptr_t>(client + BombOffsets::dwPlantedC4);
                    uintptr_t c4Entity = cs2.read<uintptr_t>(plantedC4Address);
                    if (c4Entity) {
                        float c4BlowTime = cs2.read<float>(c4Entity + BombOffsets::m_flC4Blow);
                        float defuseCountDown = cs2.read<float>(c4Entity + BombOffsets::m_flDefuseCountDown);
                        Globals::isBeingDefused = cs2.read<bool>(c4Entity + BombOffsets::m_bBeingDefused);
                        Globals::bombSite = cs2.read<int>(c4Entity + BombOffsets::m_nBombSite);
                        Globals::bombTimeLeft = c4BlowTime - serverTime;
                        Globals::defuseTimeLeft = Globals::isBeingDefused ? (defuseCountDown - serverTime) : 0.0f;
                        if (Globals::bombTimeLeft <= 0.0f) { Globals::bombTimeLeft = 0.0f; Globals::isBombPlanted = false; }
                        if (Globals::isBombPlanted && GetForegroundWindow() == cs2Window) {
                            ImU32 bombCol = ColorToU32(world.bomb_indicator.color);
                            std::string siteStr = (Globals::bombSite == 1) ? "B" : "A";
                            char timeBuffer[32]; sprintf_s(timeBuffer, "%.1f", Globals::bombTimeLeft);
                            std::string timerText = "SITE " + siteStr + "\n" + std::string(timeBuffer) + " s";
                            if (Globals::isBeingDefused) {
                                char defBuffer[32]; sprintf_s(defBuffer, "%.1f", Globals::defuseTimeLeft);
                                timerText += "\nDEFUSING: " + std::string(defBuffer) + " s";
                                bombCol = (Globals::defuseTimeLeft < Globals::bombTimeLeft) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
                            }
                            DrawSkeetText(drawList, tahomaBoldFont, 18.0f, { 15.0f, (float)monitorHeight / 2.0f }, bombCol, timerText.c_str(), false);
                            uintptr_t gameSceneNode = cs2.read<uintptr_t>(c4Entity + Offsets::m_pGameSceneNode);
                            Vector3 bombPos = cs2.read<Vector3>(gameSceneNode + Offsets::m_vecAbsOrigin);
                            Vector2 bombPosScreen = WorldToScreen(bombPos, viewMatrix);
                            if (bombPosScreen.x != 0) {
                                DrawSkeetText(drawList, tahomaBoldFont, 14.0f, { bombPosScreen.x, bombPosScreen.y }, bombCol, "C4", true);
                                drawList->AddCircle({ bombPosScreen.x, bombPosScreen.y }, 15.0f, bombCol, 0, 2.0f);
                            }
                        }
                    }
                }
                }
                catch (...) {
                    // Skip bomb indicator if error occurs
                }
                
                skip_bomb_indicator:;
            }
        }

        if (misc.master.enabled && misc.masterKey.active) {
            DrawCrosshair(drawList, monitorWidth, monitorHeight, misc.crosshair, localPlayerEntity.weaponName, isLocalPlayerScoped);
            if (misc.hitmarker.enabled) {
                uintptr_t bulletServices = cs2.read<uintptr_t>(playerPawnAddress + Offsets::m_pBulletServices);
                if (bulletServices) DrawHitmarker(drawList, monitorWidth, monitorHeight, misc.hitmarker, bulletServices, cs2, hitMarkerLastTick, hitMarkerPrevTotalHits);
            }
        }

        uintptr_t localBoneMatrix = getBoneMatrix(cs2, playerPawnAddress);
        Vector3 localEyePos{ 0, 0, 0 };
        if (localBoneMatrix) {
            localEyePos = cs2.read<Vector3>(localBoneMatrix + (6 * 32));
        }
        
        // Coletar entidades para o radar
        std::vector<Entity> allEntities;

        for (int i{ 0 }; i < 64; i++)
        {
            try {
                uintptr_t listEntry{ cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16) };
                if (!listEntry) continue;
                uintptr_t currentController{ cs2.read<uintptr_t>(listEntry + 112 * (i & 0x1FF)) };
                if (!currentController) continue;
                int pawnHandle{ cs2.read<int>(currentController + Offsets::m_hPlayerPawn) };
                if (pawnHandle == 0) continue;
                uintptr_t listEntry2{ cs2.read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16) };
                if (!listEntry2) continue;
                uintptr_t currentPawn{ cs2.read<uintptr_t>(listEntry2 + 112 * (pawnHandle & 0x1FF)) };
                if (!currentPawn) continue;

            Entity entity{ cs2, currentPawn, currentController };
            
            // Adicionar à lista para o radar
            if (entity.health > 0) {
                allEntities.push_back(entity);
            }
            
            ESPGroup* currentConfig = (entity.team == localPlayerEntity.team) ? &team : &enemy;

            if (currentConfig->master.enabled && currentConfig->masterKey.active && (GetForegroundWindow() == cs2Window))
            {
                bool check{ true };
                if (currentPawn == playerPawnAddress) check = false;
                if (entity.health <= 0 || (entity.position.x == 0.0f && entity.position.y == 0.0f)) check = false;

                if (check)
                {
                    Vector3 headPos3D = { entity.position.x, entity.position.y, entity.position.z + 72.0f };
                    Vector2 screenPos{ WorldToScreen(entity.position, viewMatrix) };
                    Vector2 headScreenPos{ WorldToScreen(headPos3D, viewMatrix) };

                    if ((screenPos.x != 0.0f && screenPos.y != 0.0f) && (headScreenPos.x != 0.0f && headScreenPos.y != 0.0f))
                    {
                        uintptr_t boneMatrix = getBoneMatrix(cs2, currentPawn);
                        if (!boneMatrix) continue;

                        bool isVisible = false;
                        if (currentConfig->visCheckEnabled) {
                            Vector3 enemyHead = cs2.read<Vector3>(boneMatrix + (6 * 32));
                            if (vCheck.IsPointVisible(enemyHead, localEyePos)) {
                                isVisible = true;
                            }
                            else {
                                Vector3 enemyChest = cs2.read<Vector3>(boneMatrix + (4 * 32));
                                if (vCheck.IsPointVisible(enemyChest, localEyePos)) isVisible = true;
                            }
                        }

                        ImU32 boxCol = ColorToU32(currentConfig->box.color);
                        ImU32 skelCol = ColorToU32(currentConfig->skeleton.color);
                        ImU32 chamsCol = ColorToU32(currentConfig->chams.color);

                        if (currentConfig->visCheckEnabled && isVisible) {
                            boxCol = ColorToU32(currentConfig->boxVisibleColor);
                            skelCol = ColorToU32(currentConfig->skeletonVisibleColor);
                            chamsCol = ColorToU32(currentConfig->chamsVisibleColor);
                        }

                        // IMPROVED CYLINDER CHAMS (Clean and Simple)
                        if (currentConfig->chams.enabled) {
                            float currentModelHeight = std::fabs(headScreenPos.y - screenPos.y);
                            if (currentModelHeight > 5) {
                                // Definir conexões anatômicas SIMPLES
                                struct BoneConnection { 
                                    int bone1; 
                                    int bone2; 
                                    float radius;
                                };
                                
                                const std::vector<BoneConnection> connections = {
                                    // Torso
                                    {0, 2, 0.135f},   // Pelvis -> Spine1
                                    {2, 4, 0.130f},   // Spine1 -> Spine2
                                    {4, 5, 0.120f},   // Spine2 -> Neck
                                    {5, 6, 0.110f},   // Neck -> Head
                                    
                                    // Braço esquerdo
                                    {5, 13, 0.070f},
                                    {13, 14, 0.065f},
                                    {14, 15, 0.055f},
                                    
                                    // Braço direito
                                    {5, 8, 0.070f},
                                    {8, 9, 0.065f},
                                    {9, 10, 0.055f},
                                    
                                    // Perna esquerda
                                    {0, 25, 0.095f},
                                    {25, 26, 0.085f},
                                    {26, 27, 0.065f},
                                    
                                    // Perna direita
                                    {0, 22, 0.095f},
                                    {22, 23, 0.085f},
                                    {23, 24, 0.065f}
                                };

                                // Coletar pontos para glow
                                std::vector<ImVec2> all_points;
                                const int segments = 12;
                                
                                for (const auto& conn : connections) {
                                    Vector3 b1 = cs2.read<Vector3>(boneMatrix + conn.bone1 * 32);
                                    Vector3 b2 = cs2.read<Vector3>(boneMatrix + conn.bone2 * 32);
                                    Vector2 s1 = WorldToScreen(b1, viewMatrix);
                                    Vector2 s2 = WorldToScreen(b2, viewMatrix);
                                    
                                    if (s1.x == 0 || s2.x == 0) continue;
                                    
                                    float r = currentModelHeight * conn.radius;
                                    if (r < 1.5f) r = 1.5f;
                                    
                                    for (int j = 0; j < segments; ++j) {
                                        float a = (float)j / segments * 2.0f * 3.14159265f;
                                        all_points.push_back({s1.x + cos(a) * r, s1.y + sin(a) * r});
                                        all_points.push_back({s2.x + cos(a) * r, s2.y + sin(a) * r});
                                    }
                                }
                                
                                // GLOW global suave
                                if (currentConfig->chamsGlow && all_points.size() > 3) {
                                    std::vector<ImVec2> hull = compute_convex_hull(all_points);
                                    if (hull.size() > 2) {
                                        float* gc = (currentConfig->visCheckEnabled && isVisible) ? 
                                            currentConfig->chamsGlowVisibleColor : currentConfig->chamsGlowColor;
                                        
                                        drawList->AddPolyline(hull.data(), hull.size(), 
                                            GetAlphaModifiedColor(gc, 0.08f * currentConfig->chamsAlpha), ImDrawFlags_Closed, 22.0f);
                                        drawList->AddPolyline(hull.data(), hull.size(), 
                                            GetAlphaModifiedColor(gc, 0.18f * currentConfig->chamsAlpha), ImDrawFlags_Closed, 15.0f);
                                        drawList->AddPolyline(hull.data(), hull.size(), 
                                            GetAlphaModifiedColor(gc, 0.35f * currentConfig->chamsAlpha), ImDrawFlags_Closed, 9.0f);
                                        drawList->AddPolyline(hull.data(), hull.size(), 
                                            GetAlphaModifiedColor(gc, 0.55f * currentConfig->chamsAlpha), ImDrawFlags_Closed, 4.0f);
                                    }
                                }

                                // Renderizar cada conexão individualmente
                                for (const auto& conn : connections) {
                                    Vector3 b1 = cs2.read<Vector3>(boneMatrix + conn.bone1 * 32);
                                    Vector3 b2 = cs2.read<Vector3>(boneMatrix + conn.bone2 * 32);
                                    Vector2 s1 = WorldToScreen(b1, viewMatrix);
                                    Vector2 s2 = WorldToScreen(b2, viewMatrix);
                                    
                                    if (s1.x == 0 || s2.x == 0) continue;

                                    float r = currentModelHeight * conn.radius;
                                    if (r < 1.5f) r = 1.5f;

                                    std::vector<ImVec2> shape;
                                    
                                    // Criar forma suave entre os dois bones
                                    for (int j = 0; j <= segments / 2; ++j) {
                                        float a = (float)j / segments * 2.0f * 3.14159265f;
                                        shape.push_back({s1.x + cos(a) * r, s1.y + sin(a) * r});
                                    }
                                    for (int j = segments / 2; j <= segments; ++j) {
                                        float a = (float)j / segments * 2.0f * 3.14159265f;
                                        shape.push_back({s2.x + cos(a) * r, s2.y + sin(a) * r});
                                    }

                                    if (shape.size() > 2) {
                                        // Calcular cor baseada no modo
                                        ImU32 fillColor = chamsCol;
                                        
                                        // Mode 1: Gradient (interpolação entre top e bottom)
                                        if (currentConfig->chamsMode == 1) {
                                            float gradientFactor = (s1.y + s2.y) / 2.0f / monitorHeight;
                                            float r = currentConfig->chamsGradientTop[0] * (1 - gradientFactor) + currentConfig->chamsGradientBottom[0] * gradientFactor;
                                            float g = currentConfig->chamsGradientTop[1] * (1 - gradientFactor) + currentConfig->chamsGradientBottom[1] * gradientFactor;
                                            float b = currentConfig->chamsGradientTop[2] * (1 - gradientFactor) + currentConfig->chamsGradientBottom[2] * gradientFactor;
                                            fillColor = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), 255);
                                        }
                                        
                                        // Mode 4: Pulse (animação de alpha)
                                        float pulseAlpha = currentConfig->chamsAlpha;
                                        if (currentConfig->chamsMode == 4) {
                                            float time = ImGui::GetTime() * currentConfig->chamsPulseSpeed;
                                            pulseAlpha = currentConfig->chamsAlpha * (0.5f + 0.5f * sin(time));
                                        }
                                        
                                        // Mode 2: Wireframe (apenas outline, sem fill)
                                        // Mode 3: Glow Only (sem fill)
                                        bool shouldFill = (currentConfig->chamsMode != 2 && currentConfig->chamsMode != 3 && !currentConfig->chamsGlowOnly);
                                        
                                        if (shouldFill) {
                                            ImU32 fill = fillColor;
                                            float alpha = (currentConfig->chamsMode == 4 ? pulseAlpha : currentConfig->chamsAlpha) * ((fill >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
                                            fill = (fill & 0x00FFFFFF) | ((ImU32)(alpha * 255.0f) << IM_COL32_A_SHIFT);
                                            drawList->AddConvexPolyFilled(shape.data(), shape.size(), fill);
                                        }
                                        
                                        // Outline (sempre visível em Wireframe e Glow Only)
                                        if (currentConfig->chamsOutline || currentConfig->chamsMode == 2 || currentConfig->chamsMode == 3) {
                                            float* oc = (currentConfig->visCheckEnabled && isVisible) ? 
                                                currentConfig->chamsOutlineVisibleColor : currentConfig->chamsOutlineColor;
                                            float outlineWidth = (currentConfig->chamsMode == 2 || currentConfig->chamsMode == 3) ? 1.8f : 1.2f;
                                            drawList->AddPolyline(shape.data(), shape.size(), 
                                                GetAlphaModifiedColor(oc, 0.65f * currentConfig->chamsAlpha), ImDrawFlags_Closed, outlineWidth);
                                        }
                                    }
                                }
                            }
                        }

                        if (currentConfig->skeleton.enabled) {
                            std::vector<std::pair<int, int>> skeletonLines = {
                               {6, 5}, {5, 4}, {4, 0}, {5, 13}, {13, 14}, {14, 15}, {5, 8}, {8, 9}, {9, 10}, {0, 25}, {25, 26}, {26, 27}, {0, 22}, {22, 23}, {23, 24}
                            };
                            for (const auto& pair : skeletonLines) {
                                Vector3 v1 = cs2.read<Vector3>(boneMatrix + pair.first * 32);
                                Vector3 v2 = cs2.read<Vector3>(boneMatrix + pair.second * 32);
                                Vector2 sv1 = WorldToScreen(v1, viewMatrix);
                                Vector2 sv2 = WorldToScreen(v2, viewMatrix);
                                if (sv1.x != 0 && sv2.x != 0)
                                    drawList->AddLine({ sv1.x, sv1.y }, { sv2.x, sv2.y }, skelCol, 1.0f);
                            }
                        }

                        if (currentConfig->sound.enabled) {
                            Vector3 vel = cs2.read<Vector3>(currentPawn + FlagOffsets::m_vecAbsVelocity);
                            float speed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
                            
                            // Validação: ignorar velocidades impossíveis (provavelmente offset errado)
                            if (speed > 10000.0f) continue;
                            
                            float dist = std::sqrt(
                                pow(localPlayerEntity.position.x - entity.position.x, 2) + 
                                pow(localPlayerEntity.position.y - entity.position.y, 2) + 
                                pow(localPlayerEntity.position.z - entity.position.z, 2)
                            );
                            
                            // Threshold mais baixo para pegar walk (100) e run (250)
                            if (dist < 1100.0f && speed > 100.0f) {
                                if (entityRunStartTime[i] == 0) entityRunStartTime[i] = GetTickCount64();
                                
                                // Delay de 150ms antes de começar a desenhar
                                if (GetTickCount64() - entityRunStartTime[i] > 150) {
                                    // Cooldown de 450ms entre ripples
                                    if (entitySoundCooldown.find(i) == entitySoundCooldown.end() || 
                                        GetTickCount64() - entitySoundCooldown[i] >= 450) {
                                        entitySoundCooldown[i] = GetTickCount64();
                                        
                                        // Criar ripple com raio variável baseado na velocidade
                                        float maxRadius = 30.0f + (speed / 10.0f);
                                        if (maxRadius > 60.0f) maxRadius = 60.0f;
                                        
                                        activeRipples.push_back({ 
                                            entity.position, 
                                            GetTickCount64(), 
                                            maxRadius, 
                                            700.0f, 
                                            ColorToU32(currentConfig->sound.color) 
                                        });
                                    }
                                }
                            }
                            else {
                                entityRunStartTime[i] = 0;
                            }
                        }

                        float boxHeight = floorf(std::fabs(headScreenPos.y - screenPos.y));
                        float boxWidth = floorf(boxHeight * 0.6f);
                        Vector2 topLeft = { floorf(screenPos.x - boxWidth / 2.0f), floorf(headScreenPos.y) };
                        Vector2 bottomRight = { floorf(screenPos.x + boxWidth / 2.0f), floorf(screenPos.y) };

                        float bL = topLeft.x, bT = topLeft.y, bR = bottomRight.x, bB = bottomRight.y;
                        float bH = bB - bT;

                        if (currentConfig->box.enabled) {
                            drawList->AddRect({ bL - 1, bT - 1 }, { bR + 1, bB + 1 }, IM_COL32(0, 0, 0, 180));
                            drawList->AddRect({ bL + 1, bT + 1 }, { bR - 1, bB - 1 }, IM_COL32(0, 0, 0, 180));
                            drawList->AddRect({ bL, bT }, { bR, bB }, boxCol);
                        }

                        if (currentConfig->health.enabled) {
                            float healthPerc = std::clamp((float)entity.health / 100.0f, 0.0f, 1.0f);
                            float barWidth = 2.0f;
                            float x = bL - 5.0f - barWidth;
                            drawList->AddRectFilled({ x - 1.0f, bT - 1.0f }, { x + barWidth + 1.0f, bB + 1.0f }, IM_COL32(0, 0, 0, 150));
                            if (entity.health > 0) {
                                ImU32 healthCol = (currentConfig->health.mode == 0) ? ColorToU32(currentConfig->health.static_color) : GetGradientColor(healthPerc, currentConfig->health.gradient_low, currentConfig->health.gradient_high);
                                float h = floorf(bH * healthPerc);
                                drawList->AddRectFilled({ x, bB - h }, { x + barWidth, bB }, healthCol);
                                if (entity.health < 100) {
                                    char hpText[10]; sprintf_s(hpText, "%d", entity.health);
                                    ImVec2 tSize = gamesenseFont->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, hpText);
                                    float tY = bB - h - 3.0f; if (tY < bT) tY = bT;
                                    float tX = x + (barWidth / 2.0f) - (tSize.x / 2.0f);
                                    DrawSkeetText(drawList, gamesenseFont, 10.0f, { tX, tY }, IM_COL32(255, 255, 255, 255), hpText, false);
                                }
                            }
                        }

                        if (currentConfig->armor.enabled) {
                            float armorPerc = std::clamp((float)entity.armor / 100.0f, 0.0f, 1.0f);
                            float barWidth = 2.0f;
                            float x = bR + 5.0f;
                            drawList->AddRectFilled({ x - 1.0f, bT - 1.0f }, { x + barWidth + 1.0f, bB + 1.0f }, IM_COL32(0, 0, 0, 150));
                            if (entity.armor > 0) {
                                float h = floorf(bH * armorPerc);
                                drawList->AddRectFilled({ x, bB - h }, { x + barWidth, bB }, ColorToU32(currentConfig->armor.static_color));
                                if (entity.armor < 100) {
                                    char arText[10]; sprintf_s(arText, "%d", entity.armor);
                                    ImVec2 tSize = gamesenseFont->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, arText);
                                    float tY = bB - h - 3.0f; if (tY < bT) tY = bT;
                                    float tX = x + (barWidth / 2.0f) - (tSize.x / 2.0f);
                                    DrawSkeetText(drawList, gamesenseFont, 10.0f, { tX, tY }, IM_COL32(255, 255, 255, 255), arText, false);
                                }
                            }
                        }

                        if (currentConfig->name.enabled) {
                            DrawNameText(drawList, tahomaBoldFont, 13.0f, { bL + (bR - bL) * 0.5f, bT - 15.0f }, ColorToU32(currentConfig->name.color), entity.name.c_str());
                        }

                        float centerX = floorf(bL + (bR - bL) * 0.5f);
                        float bottomY = bB + 4.0f;

                        if (currentConfig->ammoBar.enabled) {
                            uintptr_t clippingWeapon = cs2.read<uintptr_t>(entity.currentPawn + Offsets::m_pClippingWeapon);
                            if (clippingWeapon != 0) {
                                int currentAmmo = cs2.read<int>(clippingWeapon + Offsets::m_iClip1);
                                int maxAmmo = GetMaxClip(entity.weaponName);
                                if (maxAmmo > 0) {
                                    float ammoPerc = std::clamp((float)currentAmmo / (float)maxAmmo, 0.0f, 1.0f);
                                    float barHeight = 2.0f; float width = bR - bL;
                                    drawList->AddRectFilled({ bL - 1.0f, bottomY - 1.0f }, { bR + 1.0f, bottomY + barHeight + 1.0f }, IM_COL32(0, 0, 0, 150));
                                    if (currentAmmo > 0) {
                                        float w = floorf(width * ammoPerc);
                                        drawList->AddRectFilled({ bL, bottomY }, { bL + w, bottomY + barHeight }, ColorToU32(currentConfig->ammoBar.color));
                                        if (currentAmmo < maxAmmo) {
                                            char ammoText[10]; sprintf_s(ammoText, "%d", currentAmmo);
                                            DrawSkeetText(drawList, gamesenseFont, 10.0f, { bL + w, bottomY - 3.0f }, IM_COL32(255, 255, 255, 255), ammoText, true);
                                        }
                                    }
                                    bottomY += 5.0f;
                                }
                            }
                        }

                        if (currentConfig->weapon.enabled) {
                            if (currentConfig->weapon.text_enabled) {
                                ImVec2 txtSz = gamesenseFont->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, entity.weaponName.c_str());
                                DrawSkeetText(drawList, gamesenseFont, 10.0f, { centerX, bottomY }, ColorToU32(currentConfig->weapon.text_color), entity.weaponName.c_str(), true);
                                bottomY += txtSz.y + 1.0f;
                            }
                            if (currentConfig->weapon.icon_enabled) {
                                std::string wpnIcon = getWeaponIcon(entity.weaponName);
                                ImFont* iconFont = imguiIo.Fonts->Fonts[1];
                                ImVec2 iconSz = iconFont->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, wpnIcon.c_str());
                                WeaponIconSize iconConfig = weaponIconSizes[entity.weaponName];
                                ImVec2 iconPos = { floorf(centerX - (iconSz.x * 0.5f)), floorf(bottomY + iconConfig.offsetY) };
                                drawList->AddText(iconFont, 13.0f, { iconPos.x + 1, iconPos.y + 1 }, IM_COL32(0, 0, 0, 255), wpnIcon.c_str());
                                drawList->AddText(iconFont, 13.0f, iconPos, ColorToU32(currentConfig->weapon.icon_color), wpnIcon.c_str());
                            }
                        }

                        if (currentConfig->flagsMaster.enabled) {
                            float flagX = bR + 5.0f; if (currentConfig->armor.enabled) flagX += 4.0f;
                            float flagY = bT;
                            auto DrawFlag = [&](const char* txt, float* col) { DrawSkeetText(drawList, gamesenseFont, 10.0f, { flagX, flagY }, ColorToU32(col), txt, false); flagY += 9.0f; };
                            if (currentConfig->flagMoney.enabled) {
                                uintptr_t mS = cs2.read<uintptr_t>(currentController + FlagOffsets::m_pInGameMoneyServices);
                                if (mS) { int money = cs2.read<int>(mS + FlagOffsets::m_iAccount); if (money > 0) DrawFlag(("$" + std::to_string(money)).c_str(), currentConfig->flagMoney.color); }
                            }
                            if (currentConfig->flagArmor.enabled && entity.armor > 0) { bool helmet = cs2.read<bool>(entity.currentPawn + FlagOffsets::m_bHasHelmet); DrawFlag(helmet ? "HK" : "K", currentConfig->flagArmor.color); }
                            if (currentConfig->flagZoom.enabled && cs2.read<bool>(entity.currentPawn + FlagOffsets::m_bIsScoped)) DrawFlag("ZOOM", currentConfig->flagZoom.color);
                            if (currentConfig->flagKit.enabled) { uintptr_t iS = cs2.read<uintptr_t>(entity.currentPawn + FlagOffsets::m_pItemServices); if (iS && cs2.read<bool>(iS + FlagOffsets::m_bHasDefuser)) DrawFlag("KIT", currentConfig->flagKit.color); }
                            if (currentConfig->flagBlind.enabled && IsBlinded(cs2, entity.currentPawn)) DrawFlag("FLASH", currentConfig->flagBlind.color);
                        }

                        // SNAPLINES - linha do fundo da tela até o player
                        if (currentConfig->snaplines.enabled && screenPos.x != 0.0f && screenPos.y != 0.0f) {
                            drawList->AddLine({monitorWidth / 2.0f, (float)monitorHeight}, {screenPos.x, screenPos.y}, ColorToU32(currentConfig->snaplines.color));
                        }
                        
                        // OUT OF FOV ARROWS - Setinhas para inimigos fora da tela
                        if (currentConfig->outOfFovEnabled && (screenPos.x == 0.0f || screenPos.y == 0.0f || 
                            screenPos.x < 0 || screenPos.x > monitorWidth || screenPos.y < 0 || screenPos.y > monitorHeight)) {
                            
                            // Calcular direção do inimigo em relação ao player
                            Vector2 screenCenter = {monitorWidth / 2.0f, monitorHeight / 2.0f};
                            
                            // Usar m_angEyeAngles do local player para determinar onde ele está olhando
                            Vector2 localEyeAngles = cs2.read<Vector2>(playerPawnAddress + Offsets::m_angEyeAngles);
                            float viewYaw = localEyeAngles.y * 3.14159265f / 180.0f; // Converter para radianos
                            
                            // Calcular direção 2D do inimigo relativo ao player
                            Vector3 enemyDir = {
                                entity.position.x - localPlayerEntity.position.x,
                                entity.position.y - localPlayerEntity.position.y,
                                0.0f
                            };
                            
                            // Normalizar
                            float length = sqrt(enemyDir.x * enemyDir.x + enemyDir.y * enemyDir.y);
                            if (length > 0.1f) {
                                enemyDir.x /= length;
                                enemyDir.y /= length;
                                
                                // Calcular ângulo do inimigo em world space
                                float enemyAngle = atan2(enemyDir.y, enemyDir.x);
                                
                                // Ajustar ângulo relativo à direção que o player está olhando
                                float relativeAngle = enemyAngle - viewYaw;
                                
                                // Normalizar ângulo entre -PI e PI
                                while (relativeAngle > 3.14159265f) relativeAngle -= 2.0f * 3.14159265f;
                                while (relativeAngle < -3.14159265f) relativeAngle += 2.0f * 3.14159265f;
                                
                                // Calcular posição da seta na borda da tela
                                float arrowX = screenCenter.x;
                                float arrowY = screenCenter.y;
                                float distance = currentConfig->outOfFovDistance;
                                
                                // Converter ângulo relativo para coordenadas de tela
                                // (ajuste: Y invertido porque tela tem origem no topo)
                                float screenAngle = -relativeAngle - 3.14159265f / 2.0f; // -90° porque "frente" = topo da tela
                                
                                // Determinar qual borda
                                float screenAspect = (float)monitorWidth / (float)monitorHeight;
                                float angleAbs = abs(screenAngle);
                                
                                if (angleAbs < atan(1.0f / screenAspect)) {
                                    // Borda direita
                                    arrowX = monitorWidth - distance;
                                    arrowY = screenCenter.y + (arrowX - screenCenter.x) * tan(screenAngle);
                                } else if (angleAbs > 3.14159f - atan(1.0f / screenAspect)) {
                                    // Borda esquerda
                                    arrowX = distance;
                                    arrowY = screenCenter.y - (screenCenter.x - arrowX) * tan(screenAngle);
                                } else if (screenAngle > 0) {
                                    // Borda inferior
                                    arrowY = monitorHeight - distance;
                                    arrowX = screenCenter.x + (arrowY - screenCenter.y) / tan(screenAngle);
                                } else {
                                    // Borda superior
                                    arrowY = distance;
                                    arrowX = screenCenter.x + (arrowY - screenCenter.y) / tan(screenAngle);
                                }
                                
                                // Desenhar triângulo apontando para o inimigo
                                float size = currentConfig->outOfFovSize;
                                ImVec2 p1 = {arrowX + cos(screenAngle) * size, arrowY + sin(screenAngle) * size};
                                ImVec2 p2 = {arrowX + cos(screenAngle + 2.5f) * (size * 0.5f), arrowY + sin(screenAngle + 2.5f) * (size * 0.5f)};
                                ImVec2 p3 = {arrowX + cos(screenAngle - 2.5f) * (size * 0.5f), arrowY + sin(screenAngle - 2.5f) * (size * 0.5f)};
                                
                                ImU32 arrowColor = ColorToU32(currentConfig->outOfFovColor);
                                drawList->AddTriangleFilled(p1, p2, p3, arrowColor);
                                drawList->AddTriangle(p1, p2, p3, IM_COL32(0, 0, 0, 200), 2.0f);
                            }
                        }
                    }
                }
            }
            if (misc.spectator_list) {
                int ph = cs2.read<int>(currentController + Offsets::m_hPawn);
                if (ph) {
                    uintptr_t e = cs2.read<uintptr_t>(entityList + 0x8 * ((ph & 0x7FFF) >> 9) + 16);
                    if (e) {
                        uintptr_t pawn = cs2.read<uintptr_t>(e + 112 * (ph & 0x1FF));
                        if (pawn) {
                            uintptr_t observerServices = cs2.read<uintptr_t>(pawn + FlagOffsets::m_pObserverServices);
                            if (observerServices) {
                                int observerTarget = cs2.read<int>(observerServices + FlagOffsets::m_hObserverTarget);
                                if (observerTarget) {
                                    uintptr_t entry = cs2.read<uintptr_t>(entityList + 0x8 * ((observerTarget & 0x7FFF) >> 9) + 16);
                                    if (entry) {
                                        uintptr_t observerTargetPawn = cs2.read<uintptr_t>(entry + 112 * (observerTarget & 0x1FF));
                                        if (observerTargetPawn == playerPawnAddress) spectatingPlrs.push_back(entity);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            }
            catch (...) {
                // Skip this entity if any error occurs
                continue;
            }
        }

        if (misc.spectator_list) DrawSpectatorList(spectatingPlrs);
        
        // === WORLD ESP SIMPLIFICADO (Somente Bomb Indicator no código principal) ===
        // Dropped weapons, projectiles e grenade radius foram removidos

        // === AIMBOT / TRIGGERBOT / RCS SYSTEM ===
        if (aim.master.enabled && aim.masterKey.active) {
            // Determinar categoria da arma atual
            WeaponCategoryAim* activeCategory = nullptr;
            std::string weaponName = localPlayerEntity.weaponName;
            
            // Pistols
            if (weaponName.find("USP") != std::string::npos || weaponName.find("P2000") != std::string::npos ||
                weaponName.find("Glock") != std::string::npos || weaponName.find("P250") != std::string::npos ||
                weaponName.find("CZ75") != std::string::npos || weaponName.find("Five-Seven") != std::string::npos ||
                weaponName.find("Tec-9") != std::string::npos || weaponName.find("Dual Berettas") != std::string::npos ||
                weaponName.find("R8") != std::string::npos || weaponName.find("Desert Eagle") != std::string::npos) {
                activeCategory = &aim.pistol;
            }
            // SMGs
            else if (weaponName.find("MP9") != std::string::npos || weaponName.find("MAC-10") != std::string::npos ||
                     weaponName.find("MP7") != std::string::npos || weaponName.find("MP5") != std::string::npos ||
                     weaponName.find("UMP") != std::string::npos || weaponName.find("P90") != std::string::npos ||
                     weaponName.find("PP-Bizon") != std::string::npos) {
                activeCategory = &aim.smg;
            }
            // Rifles
            else if (weaponName.find("M4A4") != std::string::npos || weaponName.find("M4A1") != std::string::npos ||
                     weaponName.find("AK-47") != std::string::npos || weaponName.find("AUG") != std::string::npos ||
                     weaponName.find("SG 553") != std::string::npos || weaponName.find("Galil") != std::string::npos ||
                     weaponName.find("FAMAS") != std::string::npos) {
                activeCategory = &aim.rifle;
            }
            // Snipers
            else if (weaponName.find("AWP") != std::string::npos || weaponName.find("SSG 08") != std::string::npos ||
                     weaponName.find("G3SG1") != std::string::npos || weaponName.find("SCAR-20") != std::string::npos) {
                activeCategory = &aim.sniper;
            }
            // Shotguns
            else if (weaponName.find("Nova") != std::string::npos || weaponName.find("XM1014") != std::string::npos ||
                     weaponName.find("MAG-7") != std::string::npos || weaponName.find("Sawed-Off") != std::string::npos) {
                activeCategory = &aim.shotgun;
            }
            // Heavy
            else if (weaponName.find("Negev") != std::string::npos || weaponName.find("M249") != std::string::npos) {
                activeCategory = &aim.heavy;
            }

            if (activeCategory) {
                // Update keybinds
                UpdateKeybind(activeCategory->aimbot_key);
                UpdateKeybind(activeCategory->triggerbot_key);

                // === RECOIL CONTROL SYSTEM (RCS) ===
                if (activeCategory->rcs_enabled) {
                    static Vector2 lastPunch = {0.0f, 0.0f};
                    Vector2 currentPunch = cs2.read<Vector2>(playerPawnAddress + Offsets::m_aimPunchAngle);
                    
                    int shotsFired = cs2.read<int>(playerPawnAddress + Offsets::m_iShotsFired);
                    if (shotsFired > 1) {
                        // Calcular delta do recoil
                        Vector2 punchDelta = {
                            (currentPunch.x - lastPunch.x) * (activeCategory->rcs_y / 100.0f),
                            (currentPunch.y - lastPunch.y) * (activeCategory->rcs_x / 100.0f)
                        };
                        
                        // Anti-jitter: só aplicar se delta for significativo
                        if (std::abs(punchDelta.x) > 0.01f || std::abs(punchDelta.y) > 0.01f) {
                            Vector2 viewAngles = cs2.read<Vector2>(client + Offsets::dwViewAngles);
                            viewAngles.x -= punchDelta.x;
                            viewAngles.y -= punchDelta.y;
                            
                            // Clamp angles
                            if (viewAngles.x > 89.0f) viewAngles.x = 89.0f;
                            if (viewAngles.x < -89.0f) viewAngles.x = -89.0f;
                            while (viewAngles.y > 180.0f) viewAngles.y -= 360.0f;
                            while (viewAngles.y < -180.0f) viewAngles.y += 360.0f;
                            
                            cs2.write<Vector2>(client + Offsets::dwViewAngles, viewAngles);
                        }
                    }
                    else {
                        // Reset quando não está atirando
                        lastPunch = {0.0f, 0.0f};
                    }
                    lastPunch = currentPunch;
                }

                // === TRIGGERBOT ===
                if (activeCategory->triggerbot_enabled && activeCategory->triggerbot_key.active) {
                    int crosshairID = cs2.read<int>(playerPawnAddress + Offsets::m_iIDEntIndex);
                    if (crosshairID > 0 && crosshairID < 64) {
                        uintptr_t listEntry = cs2.read<uintptr_t>(entityList + 8 * ((crosshairID & 0x7FFF) >> 9) + 16);
                        if (listEntry) {
                            uintptr_t targetPawn = cs2.read<uintptr_t>(listEntry + 112 * (crosshairID & 0x1FF));
                            if (targetPawn) {
                                int targetTeam = cs2.read<int>(targetPawn + Offsets::m_iTeamNum);
                                int targetHealth = cs2.read<int>(targetPawn + Offsets::m_iHealth);
                                
                                if (targetTeam != localPlayerEntity.team && targetHealth > 0) {
                                    static ULONGLONG lastTriggerTime = 0;
                                    ULONGLONG currentTime = GetTickCount64();
                                    
                                    if (currentTime - lastTriggerTime > (ULONGLONG)activeCategory->triggerbot_delay) {
                                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                                        Sleep(10);
                                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                                        lastTriggerTime = currentTime;
                                    }
                                }
                            }
                        }
                    }
                }

                // === AIMBOT ===
                if (activeCategory->aimbot_enabled && activeCategory->aimbot_key.active) {
                    Vector3 localEyePos = localPlayerEntity.position;
                    Vector3 viewOffset = cs2.read<Vector3>(playerPawnAddress + Offsets::m_vecViewOffset);
                    localEyePos.x += viewOffset.x;
                    localEyePos.y += viewOffset.y;
                    localEyePos.z += viewOffset.z;

                    Vector2 currentViewAngles = cs2.read<Vector2>(client + Offsets::dwViewAngles);
                    
                    float bestFov = activeCategory->aimbot_fov;
                    Vector3 bestTargetPos = {0, 0, 0};
                    bool foundTarget = false;
                    int targetsChecked = 0;

                    // Procurar melhor alvo
                    for (int i = 0; i < 64; i++) {
                        uintptr_t listEntry = cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16);
                        if (!listEntry) continue;
                        
                        uintptr_t currentController = cs2.read<uintptr_t>(listEntry + 112 * (i & 0x1FF));
                        if (!currentController) continue;
                        
                        int pawnHandle = cs2.read<int>(currentController + Offsets::m_hPlayerPawn);
                        if (pawnHandle == 0) continue;
                        
                        uintptr_t listEntry2 = cs2.read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16);
                        uintptr_t currentPawn = cs2.read<uintptr_t>(listEntry2 + 112 * (pawnHandle & 0x1FF));
                        if (!currentPawn || currentPawn == playerPawnAddress) continue;
                        
                        int targetHealth = cs2.read<int>(currentPawn + Offsets::m_iHealth);
                        int targetTeam = cs2.read<int>(currentPawn + Offsets::m_iTeamNum);
                        
                        if (targetHealth <= 0 || targetTeam == localPlayerEntity.team) continue;
                        targetsChecked++;

                        uintptr_t boneMatrix = getBoneMatrix(cs2, currentPawn);
                        if (!boneMatrix) continue;

                        Vector3 targetBonePos = cs2.read<Vector3>(boneMatrix + activeCategory->aimbot_bone * 32);
                        
                        // Calcular ângulo necessário
                        Vector3 delta = {
                            targetBonePos.x - localEyePos.x,
                            targetBonePos.y - localEyePos.y,
                            targetBonePos.z - localEyePos.z
                        };
                        
                        float distance = sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
                        if (distance < 1.0f) continue;

                        float targetPitch = asin(delta.z / distance) * (180.0f / 3.14159265f);
                        float targetYaw = atan2(delta.y, delta.x) * (180.0f / 3.14159265f);
                        
                        // Calcular FOV
                        float deltaYaw = targetYaw - currentViewAngles.y;
                        float deltaPitch = targetPitch - currentViewAngles.x;
                        
                        while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
                        while (deltaYaw < -180.0f) deltaYaw += 360.0f;
                        
                        float fov = sqrt(deltaYaw * deltaYaw + deltaPitch * deltaPitch);
                        
                        if (fov < bestFov) {
                            bestFov = fov;
                            bestTargetPos = targetBonePos;
                            foundTarget = true;
                        }
                    }

                    // Aim no alvo
                    if (foundTarget) {
                        Vector3 delta = {
                            bestTargetPos.x - localEyePos.x,
                            bestTargetPos.y - localEyePos.y,
                            bestTargetPos.z - localEyePos.z
                        };
                        
                        float distance = sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
                        float targetPitch = asin(delta.z / distance) * (180.0f / 3.14159265f);
                        float targetYaw = atan2(delta.y, delta.x) * (180.0f / 3.14159265f);
                        
                        // Smoothing
                        float deltaYaw = targetYaw - currentViewAngles.y;
                        float deltaPitch = targetPitch - currentViewAngles.x;
                        
                        while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
                        while (deltaYaw < -180.0f) deltaYaw += 360.0f;
                        
                        float smoothFactor = activeCategory->aimbot_smooth;
                        Vector2 newAngles = {
                            currentViewAngles.x + deltaPitch / smoothFactor,
                            currentViewAngles.y + deltaYaw / smoothFactor
                        };
                        
                        // Clamp
                        if (newAngles.x > 89.0f) newAngles.x = 89.0f;
                        if (newAngles.x < -89.0f) newAngles.x = -89.0f;
                        while (newAngles.y > 180.0f) newAngles.y -= 360.0f;
                        while (newAngles.y < -180.0f) newAngles.y += 360.0f;
                        
                        cs2.write<Vector2>(client + Offsets::dwViewAngles, newAngles);
                    }
                }
            }
        }

        if (misc.master.enabled && misc.masterKey.active) {
            // Hitsound
            if (misc.hitsound_enabled) {
                uintptr_t bulletServices = cs2.read<uintptr_t>(playerPawnAddress + Offsets::m_pBulletServices);
                if (bulletServices) {
                    int totalHits = cs2.read<int>(bulletServices + Offsets::m_totalHitsOnServer);
                    if (totalHits > lastHitCount) {
                        lastHitCount = totalHits;
                        lastHitTime = GetTickCount64();
                        
                        // Tocar som customizado
                        std::wstring wpath(currentHitsoundPath.begin(), currentHitsoundPath.end());
                        PlaySound(wpath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
                    }
                }
            }
            
            // Radar - Coletar entidades já foi feito no loop acima
            DrawRadar(drawList, monitorWidth, monitorHeight, localPlayerEntity, allEntities, cs2, playerPawnAddress);
            
            // Snow Effect
            if (misc.snow_effect) {
                if (snowFlakes.empty()) InitSnowEffect(monitorWidth, monitorHeight);
                UpdateSnowEffect(monitorWidth, monitorHeight);
                DrawSnowEffect(drawList);
            }
        }
        
        }
        catch (const std::exception& e) {
            std::cout << "[EXCEPTION] Error reading game memory: " << e.what() << std::endl;
            // Continue to next iteration
        }
        catch (...) {
            std::cout << "[EXCEPTION] Unknown error reading game memory" << std::endl;
            // Continue to next iteration
        }
        
        renderer.EndRender();
        Sleep(1);
    }
    renderer.Shutdown();
    return 0;
}