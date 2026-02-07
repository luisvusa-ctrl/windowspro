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

// dependencias do sistema e do projeto
#include <iostream>
#define NOMINMAX
#include <Windows.h>
#include <mmsystem.h>
#include <vector>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <string>
#include <cmath>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <array>
#include <mutex>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#include <fstream>
#include <functional>
#include <commdlg.h>

#pragma comment(lib, "winmm.lib")

#define JSON_HAS_FILESYSTEM 0
#define JSON_HAS_EXPERIMENTAL_FILESYSTEM 1
#include "inc/Renderer.hpp"
#include "inc/VisCheck.h"
#include "inc/mem.hpp"
#include "inc/utils.hpp"
#include "inc/input.hpp"
#include "inc/offsets.hpp"
#include "inc/dependencies/json.hpp"

#include "inc/firaCode.hpp"
#include "inc/weaponFont.hpp"

#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

// === MATHEMATICAL CONSTANTS ===
namespace Math {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 6.28318530717958647692f;
    constexpr float HALF_PI = 1.57079632679489661923f;
    constexpr float DEG_TO_RAD = 0.01745329251994329576f;
    constexpr float RAD_TO_DEG = 57.2957795130823208767f;
}

// --- VARIÁVEIS GLOBAIS ---
// fontes usadas pelo overlay
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

// carrega nomes amigaveis das teclas do teclado e mouse
// isso aqui inicializa a tabela de nomes de teclas para o menu
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

// retorna o nome da tecla com validacao simples
// isso aqui evita acessar indice invalido na tabela de nomes
const char* GetKeyNameSafe(int id) {
    if (id < 0 || id > 255) return "Unknown";
    return VirtualKeyNames[id].c_str();
}

// estado global usado pelo esp e pela interface
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

// dados de ripple para visualizar sons no mundo
struct SoundRipple {
    Vector3 position;
    ULONGLONG startTime;
    float maxRadius = 30.0f;
    float duration = 700.0f;
    ImU32 color;
};

// listas e caches para efeitos e animacoes
std::vector<SoundRipple> activeRipples;
std::map<int, ULONGLONG> entityLastRippleTime;
std::map<int, bool> entityWasOnGround;
std::map<int, ULONGLONG> entityRunStartTime;
std::map<int, Vector3> entityRunStartPos;
std::map<int, float> healthBarAnimValues;
std::map<int, float> armorBarAnimValues;
std::map<int, float> ammoBarAnimValues;
ULONGLONG lastHitTime = 0;
int lastHitCount = 0;

inline float ClampFloat(float value, float minValue, float maxValue);

// modo de ativacao para binds
enum KeyMode { AlwaysOn = 0, Hold, Toggle };

// configuracao de atalho de teclado para features
struct KeyBind {
    int key = 0;
    int mode = 2;
    bool active = true;
    bool waitingForKey = false;
};

// item simples com toggle e cor
struct ConfigItem {
    bool enabled;
    float color[4];
    ConfigItem() {
        enabled = false;
        for (int i = 0; i < 4; i++) color[i] = 1.0f;
    }
};

// configuracao de exibicao da arma
struct WeaponSettings {
    bool enabled = false;
    bool text_enabled = false; float text_color[4] = { 1, 1, 1, 1 };
    bool icon_enabled = false; float icon_color[4] = { 1, 1, 1, 1 };
};

// configuracao do esp de som
struct SoundSettings {
    bool enabled = false;
    float color[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
};

// configuracao da barra de vida
struct HealthSettings {
    bool enabled = false;
    int mode = 1;
    float static_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float gradient_low[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float gradient_high[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    bool show_numbers_below_max = true;
    bool animate_drop = true;
};

// configuracao da barra de armadura
struct ArmorSettings {
    bool enabled = false;
    float static_color[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
    bool show_numbers_below_max = true;
    bool animate_drop = true;
};

// configuracao de uma flag lateral (ex: money, armor)
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

    ConfigItem snaplines;
    
    // Out of FOV arrows
    bool outOfFovEnabled = false;
    float outOfFovColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float outOfFovSize = 20.0f;
    float outOfFovDistance = 100.0f; // Distância da borda da tela
    
    ConfigItem ammoBar;
    bool ammo_show_numbers_below_max = true;
    bool ammo_animate_drop = true;

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
    
    MiscSettings() {
        master.enabled = true;
        masterKey.mode = KeyMode::AlwaysOn;
    }
};

struct AimbotSettings {
    bool enabled = false;
    bool vis_check_enabled = false;
    int lock_part = 0;
    bool visualise_target = false;
    bool target_tracer = false;
    bool fov_enabled = false;
    bool team_check = true;
    float fov_size = 100.0f;
    float smoothing = 1.0f;
    KeyBind key;
    AimbotSettings() {
        key.key = VK_RBUTTON;
        key.mode = KeyMode::Hold;
    }
};

struct TriggerbotSettings {
    bool enabled = false;
    bool team_check = true;
    KeyBind key;
    TriggerbotSettings() {
        key.key = VK_LMENU;
        key.mode = KeyMode::Hold;
    }
};

struct RcsSettings {
    bool enabled = false;
    Vector2 xy{ 2.0f, 2.0f };
    int shots_fired = 1;
};

struct AimSettings {
    AimbotSettings aimbot;
    TriggerbotSettings trigger;
    RcsSettings rcs;
};

// Weapon Visuals removido - World ESP simplificado

struct ProjectileTypeSettings {
    bool text = true;
    bool icon = false;
    bool timer = false;
    bool timer_bar = false;
    bool distance = false;
};

struct WorldSettings {
    ConfigItem master;
    KeyBind masterKey;
    ConfigItem bomb_indicator;
    ConfigItem dropped_weapons;
    ConfigItem dropped_projectiles;
    bool dropped_weapons_name = true;
    bool dropped_weapons_icon = false;
    bool dropped_weapons_ammo = false;
    bool dropped_weapons_distance = false;
    float dropped_weapons_name_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float dropped_weapons_icon_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float dropped_weapons_ammo_color[4] = { 0.3f, 0.9f, 0.3f, 1.0f };
    float dropped_weapons_distance_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<ProjectileTypeSettings, 5> dropped_projectiles_types;
    int dropped_projectiles_selected = 0;
    float dropped_projectiles_name_color[4] = { 1.0f, 0.9f, 0.9f, 1.0f };
    float dropped_projectiles_icon_color[4] = { 1.0f, 0.9f, 0.9f, 1.0f };
    float dropped_projectiles_timer_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float dropped_projectiles_distance_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    WorldSettings() {
        master.enabled = true;
        masterKey.mode = KeyMode::AlwaysOn;
        bomb_indicator.color[0] = 1.0f; bomb_indicator.color[1] = 0.0f; bomb_indicator.color[2] = 0.0f;
        dropped_weapons.color[0] = 1.0f; dropped_weapons.color[1] = 1.0f; dropped_weapons.color[2] = 1.0f; dropped_weapons.color[3] = 1.0f;
        dropped_projectiles.color[0] = 1.0f; dropped_projectiles.color[1] = 0.2f; dropped_projectiles.color[2] = 0.2f; dropped_projectiles.color[3] = 1.0f;
    }
};

static ESPGroup enemy, team;
static WorldSettings world;
static MiscSettings misc;
static AimSettings aim;
static KeyBind menuKey;
static Vector2 rcsOldPunch{ 0.0f, 0.0f };
static const char* kAimbotLockParts[] = { "Head", "Chest", "Left Arm", "Right Arm", "Left Leg", "Right Leg" };

static const std::pair<int, int> kSkeletonLines[] = {
    {6, 5}, {5, 4}, {4, 0}, {5, 13}, {13, 14}, {14, 15}, {5, 8}, {8, 9}, {9, 10}, {0, 25}, {25, 26}, {26, 27}, {0, 22}, {22, 23}, {23, 24}
};


// --- FUNÇÕES AUXILIARES ---
// utilitarios gerais usados em varias partes do esp

// isso aqui limita valores para evitar overflow de percentuais
inline float ClampFloat(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

// ajusta o volume do hitsound com base em porcentagem
void ApplyHitsoundVolume(float volumePercent) {
    float volume = ClampFloat(volumePercent, 0.0f, 100.0f);
    DWORD level = (DWORD)(0xFFFF * (volume / 100.0f));
    DWORD packed = (level & 0xFFFF) | (level << 16);
    waveOutSetVolume(nullptr, packed);
}

// snapshot de estilo para restaurar janela apos dialogo
struct WindowStyleState {
    LONG exStyle;
};

// prepara janela para abrir dialogo sem ficar sempre no topo
WindowStyleState BeginFileDialog(HWND hwnd) {
    WindowStyleState state{ GetWindowLong(hwnd, GWL_EXSTYLE) };
    LONG newStyle = state.exStyle | WS_EX_TRANSPARENT;
    newStyle &= ~WS_EX_TOPMOST;
    SetWindowLong(hwnd, GWL_EXSTYLE, newStyle);
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return state;
}

// restaura o estilo original da janela
void EndFileDialog(HWND hwnd, const WindowStyleState& state) {
    SetWindowLong(hwnd, GWL_EXSTYLE, state.exStyle);
    if (state.exStyle & WS_EX_TOPMOST) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else {
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

// abre dialogo de selecao de arquivo json
std::string OpenJsonFileDialog(HWND owner) {
    char file[MAX_PATH] = { 0 };
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "json";
    if (GetOpenFileNameA(&ofn)) {
        return std::string(file);
    }
    return "";
}

// abre dialogo para salvar arquivo json
std::string SaveJsonFileDialog(HWND owner) {
    char file[MAX_PATH] = { 0 };
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "json";
    if (GetSaveFileNameA(&ofn)) {
        return std::string(file);
    }
    return "";
}

// converte cor float[4] para json
nlohmann::json ColorToJson(const float* color) {
    return nlohmann::json::array({ color[0], color[1], color[2], color[3] });
}

// aplica cor armazenada no json ao float[4]
void JsonToColor(const nlohmann::json& j, float* color) {
    if (!j.is_array() || j.size() < 4) return;
    for (int i = 0; i < 4; i++) color[i] = j[i].get<float>();
}

nlohmann::json Vector2ToJson(const Vector2& v) {
    return nlohmann::json::array({ v.x, v.y });
}

void JsonToVector2(const nlohmann::json& j, Vector2& v) {
    if (!j.is_array() || j.size() < 2) return;
    v.x = j[0].get<float>();
    v.y = j[1].get<float>();
}

nlohmann::json ConfigItemToJson(const ConfigItem& item) {
    nlohmann::json j;
    j["enabled"] = item.enabled;
    j["color"] = ColorToJson(item.color);
    return j;
}

void JsonToConfigItem(const nlohmann::json& j, ConfigItem& item) {
    if (j.contains("enabled")) item.enabled = j["enabled"].get<bool>();
    if (j.contains("color")) JsonToColor(j["color"], item.color);
}

nlohmann::json KeyBindToJson(const KeyBind& bind) {
    nlohmann::json j;
    j["key"] = bind.key;
    j["mode"] = bind.mode;
    return j;
}

void JsonToKeyBind(const nlohmann::json& j, KeyBind& bind) {
    if (j.contains("key")) bind.key = j["key"].get<int>();
    if (j.contains("mode")) bind.mode = j["mode"].get<int>();
}

nlohmann::json FlagItemToJson(const FlagItem& item) {
    nlohmann::json j;
    j["enabled"] = item.enabled;
    j["color"] = ColorToJson(item.color);
    return j;
}

void JsonToFlagItem(const nlohmann::json& j, FlagItem& item) {
    if (j.contains("enabled")) item.enabled = j["enabled"].get<bool>();
    if (j.contains("color")) JsonToColor(j["color"], item.color);
}

nlohmann::json WeaponSettingsToJson(const WeaponSettings& w) {
    nlohmann::json j;
    j["enabled"] = w.enabled;
    j["text_enabled"] = w.text_enabled;
    j["text_color"] = ColorToJson(w.text_color);
    j["icon_enabled"] = w.icon_enabled;
    j["icon_color"] = ColorToJson(w.icon_color);
    return j;
}

void JsonToWeaponSettings(const nlohmann::json& j, WeaponSettings& w) {
    if (j.contains("enabled")) w.enabled = j["enabled"].get<bool>();
    if (j.contains("text_enabled")) w.text_enabled = j["text_enabled"].get<bool>();
    if (j.contains("text_color")) JsonToColor(j["text_color"], w.text_color);
    if (j.contains("icon_enabled")) w.icon_enabled = j["icon_enabled"].get<bool>();
    if (j.contains("icon_color")) JsonToColor(j["icon_color"], w.icon_color);
}

nlohmann::json SoundSettingsToJson(const SoundSettings& s) {
    nlohmann::json j;
    j["enabled"] = s.enabled;
    j["color"] = ColorToJson(s.color);
    return j;
}

void JsonToSoundSettings(const nlohmann::json& j, SoundSettings& s) {
    if (j.contains("enabled")) s.enabled = j["enabled"].get<bool>();
    if (j.contains("color")) JsonToColor(j["color"], s.color);
}

nlohmann::json HealthSettingsToJson(const HealthSettings& h) {
    nlohmann::json j;
    j["enabled"] = h.enabled;
    j["mode"] = h.mode;
    j["static_color"] = ColorToJson(h.static_color);
    j["gradient_low"] = ColorToJson(h.gradient_low);
    j["gradient_high"] = ColorToJson(h.gradient_high);
    j["show_numbers_below_max"] = h.show_numbers_below_max;
    j["animate_drop"] = h.animate_drop;
    return j;
}

void JsonToHealthSettings(const nlohmann::json& j, HealthSettings& h) {
    if (j.contains("enabled")) h.enabled = j["enabled"].get<bool>();
    if (j.contains("mode")) h.mode = j["mode"].get<int>();
    if (j.contains("static_color")) JsonToColor(j["static_color"], h.static_color);
    if (j.contains("gradient_low")) JsonToColor(j["gradient_low"], h.gradient_low);
    if (j.contains("gradient_high")) JsonToColor(j["gradient_high"], h.gradient_high);
    if (j.contains("show_numbers_below_max")) h.show_numbers_below_max = j["show_numbers_below_max"].get<bool>();
    if (j.contains("animate_drop")) h.animate_drop = j["animate_drop"].get<bool>();
}

nlohmann::json ArmorSettingsToJson(const ArmorSettings& a) {
    nlohmann::json j;
    j["enabled"] = a.enabled;
    j["static_color"] = ColorToJson(a.static_color);
    j["show_numbers_below_max"] = a.show_numbers_below_max;
    j["animate_drop"] = a.animate_drop;
    return j;
}

void JsonToArmorSettings(const nlohmann::json& j, ArmorSettings& a) {
    if (j.contains("enabled")) a.enabled = j["enabled"].get<bool>();
    if (j.contains("static_color")) JsonToColor(j["static_color"], a.static_color);
    if (j.contains("show_numbers_below_max")) a.show_numbers_below_max = j["show_numbers_below_max"].get<bool>();
    if (j.contains("animate_drop")) a.animate_drop = j["animate_drop"].get<bool>();
}

nlohmann::json CrosshairToJson(const CrosshairSettings& c) {
    nlohmann::json j;
    j["enabled"] = c.enabled;
    j["color"] = ColorToJson(c.color);
    j["outline"] = c.outline;
    j["outline_color"] = ColorToJson(c.outline_color);
    j["thickness"] = c.thickness;
    j["length"] = c.length;
    j["gap"] = c.gap;
    j["sniper_only"] = c.sniper_only;
    j["show_when_scoped"] = c.show_when_scoped;
    return j;
}

void JsonToCrosshair(const nlohmann::json& j, CrosshairSettings& c) {
    if (j.contains("enabled")) c.enabled = j["enabled"].get<bool>();
    if (j.contains("color")) JsonToColor(j["color"], c.color);
    if (j.contains("outline")) c.outline = j["outline"].get<bool>();
    if (j.contains("outline_color")) JsonToColor(j["outline_color"], c.outline_color);
    if (j.contains("thickness")) c.thickness = j["thickness"].get<float>();
    if (j.contains("length")) c.length = j["length"].get<float>();
    if (j.contains("gap")) c.gap = j["gap"].get<float>();
    if (j.contains("sniper_only")) c.sniper_only = j["sniper_only"].get<bool>();
    if (j.contains("show_when_scoped")) c.show_when_scoped = j["show_when_scoped"].get<bool>();
}

nlohmann::json HitmarkerToJson(const HitmarkerSettings& h) {
    nlohmann::json j;
    j["enabled"] = h.enabled;
    j["color"] = ColorToJson(h.color);
    j["outline_color"] = ColorToJson(h.outline_color);
    j["thickness"] = h.thickness;
    j["outline_thickness"] = h.outline_thickness;
    j["duration_ms"] = h.duration_ms;
    j["show_outline"] = h.show_outline;
    j["size"] = h.size;
    j["gap"] = h.gap;
    return j;
}

void JsonToHitmarker(const nlohmann::json& j, HitmarkerSettings& h) {
    if (j.contains("enabled")) h.enabled = j["enabled"].get<bool>();
    if (j.contains("color")) JsonToColor(j["color"], h.color);
    if (j.contains("outline_color")) JsonToColor(j["outline_color"], h.outline_color);
    if (j.contains("thickness")) h.thickness = j["thickness"].get<float>();
    if (j.contains("outline_thickness")) h.outline_thickness = j["outline_thickness"].get<float>();
    if (j.contains("duration_ms")) h.duration_ms = j["duration_ms"].get<int>();
    if (j.contains("show_outline")) h.show_outline = j["show_outline"].get<bool>();
    if (j.contains("size")) h.size = j["size"].get<float>();
    if (j.contains("gap")) h.gap = j["gap"].get<float>();
}

nlohmann::json EspGroupToJson(const ESPGroup& g) {
    nlohmann::json j;
    j["master"] = ConfigItemToJson(g.master);
    j["masterKey"] = KeyBindToJson(g.masterKey);
    j["box"] = ConfigItemToJson(g.box);
    j["boxVisibleColor"] = ColorToJson(g.boxVisibleColor);
    j["name"] = ConfigItemToJson(g.name);
    j["skeleton"] = ConfigItemToJson(g.skeleton);
    j["skeletonVisibleColor"] = ColorToJson(g.skeletonVisibleColor);
    j["snaplines"] = ConfigItemToJson(g.snaplines);
    j["outOfFovEnabled"] = g.outOfFovEnabled;
    j["outOfFovColor"] = ColorToJson(g.outOfFovColor);
    j["outOfFovSize"] = g.outOfFovSize;
    j["outOfFovDistance"] = g.outOfFovDistance;
    j["ammoBar"] = ConfigItemToJson(g.ammoBar);
    j["ammo_show_numbers_below_max"] = g.ammo_show_numbers_below_max;
    j["ammo_animate_drop"] = g.ammo_animate_drop;
    j["weapon"] = WeaponSettingsToJson(g.weapon);
    j["health"] = HealthSettingsToJson(g.health);
    j["armor"] = ArmorSettingsToJson(g.armor);
    j["flagsMaster"] = ConfigItemToJson(g.flagsMaster);
    j["flagMoney"] = FlagItemToJson(g.flagMoney);
    j["flagArmor"] = FlagItemToJson(g.flagArmor);
    j["flagZoom"] = FlagItemToJson(g.flagZoom);
    j["flagKit"] = FlagItemToJson(g.flagKit);
    j["flagBlind"] = FlagItemToJson(g.flagBlind);
    j["visCheckEnabled"] = g.visCheckEnabled;
    j["sound"] = SoundSettingsToJson(g.sound);
    return j;
}

void JsonToEspGroup(const nlohmann::json& j, ESPGroup& g) {
    if (j.contains("master")) JsonToConfigItem(j["master"], g.master);
    if (j.contains("masterKey")) JsonToKeyBind(j["masterKey"], g.masterKey);
    if (j.contains("box")) JsonToConfigItem(j["box"], g.box);
    if (j.contains("boxVisibleColor")) JsonToColor(j["boxVisibleColor"], g.boxVisibleColor);
    if (j.contains("name")) JsonToConfigItem(j["name"], g.name);
    if (j.contains("skeleton")) JsonToConfigItem(j["skeleton"], g.skeleton);
    if (j.contains("skeletonVisibleColor")) JsonToColor(j["skeletonVisibleColor"], g.skeletonVisibleColor);
    if (j.contains("snaplines")) JsonToConfigItem(j["snaplines"], g.snaplines);
    if (j.contains("outOfFovEnabled")) g.outOfFovEnabled = j["outOfFovEnabled"].get<bool>();
    if (j.contains("outOfFovColor")) JsonToColor(j["outOfFovColor"], g.outOfFovColor);
    if (j.contains("outOfFovSize")) g.outOfFovSize = j["outOfFovSize"].get<float>();
    if (j.contains("outOfFovDistance")) g.outOfFovDistance = j["outOfFovDistance"].get<float>();
    if (j.contains("ammoBar")) JsonToConfigItem(j["ammoBar"], g.ammoBar);
    if (j.contains("ammo_show_numbers_below_max")) g.ammo_show_numbers_below_max = j["ammo_show_numbers_below_max"].get<bool>();
    if (j.contains("ammo_animate_drop")) g.ammo_animate_drop = j["ammo_animate_drop"].get<bool>();
    if (j.contains("weapon")) JsonToWeaponSettings(j["weapon"], g.weapon);
    if (j.contains("health")) JsonToHealthSettings(j["health"], g.health);
    if (j.contains("armor")) JsonToArmorSettings(j["armor"], g.armor);
    if (j.contains("flagsMaster")) JsonToConfigItem(j["flagsMaster"], g.flagsMaster);
    if (j.contains("flagMoney")) JsonToFlagItem(j["flagMoney"], g.flagMoney);
    if (j.contains("flagArmor")) JsonToFlagItem(j["flagArmor"], g.flagArmor);
    if (j.contains("flagZoom")) JsonToFlagItem(j["flagZoom"], g.flagZoom);
    if (j.contains("flagKit")) JsonToFlagItem(j["flagKit"], g.flagKit);
    if (j.contains("flagBlind")) JsonToFlagItem(j["flagBlind"], g.flagBlind);
    if (j.contains("visCheckEnabled")) g.visCheckEnabled = j["visCheckEnabled"].get<bool>();
    if (j.contains("sound")) JsonToSoundSettings(j["sound"], g.sound);
}

nlohmann::json WorldToJson(const WorldSettings& w) {
    nlohmann::json j;
    j["master"] = ConfigItemToJson(w.master);
    j["masterKey"] = KeyBindToJson(w.masterKey);
    j["bomb_indicator"] = ConfigItemToJson(w.bomb_indicator);
    j["dropped_weapons"] = ConfigItemToJson(w.dropped_weapons);
    j["dropped_projectiles"] = ConfigItemToJson(w.dropped_projectiles);
    j["dropped_weapons_name"] = w.dropped_weapons_name;
    j["dropped_weapons_icon"] = w.dropped_weapons_icon;
    j["dropped_weapons_ammo"] = w.dropped_weapons_ammo;
    j["dropped_weapons_distance"] = w.dropped_weapons_distance;
    j["dropped_weapons_name_color"] = ColorToJson(w.dropped_weapons_name_color);
    j["dropped_weapons_icon_color"] = ColorToJson(w.dropped_weapons_icon_color);
    j["dropped_weapons_ammo_color"] = ColorToJson(w.dropped_weapons_ammo_color);
    j["dropped_weapons_distance_color"] = ColorToJson(w.dropped_weapons_distance_color);
    nlohmann::json projTypes = nlohmann::json::array();
    for (const auto& t : w.dropped_projectiles_types) {
        nlohmann::json entry;
        entry["text"] = t.text;
        entry["icon"] = t.icon;
        entry["timer"] = t.timer;
        entry["timer_bar"] = t.timer_bar;
        entry["distance"] = t.distance;
        projTypes.push_back(entry);
    }
    j["dropped_projectiles_types"] = projTypes;
    j["dropped_projectiles_selected"] = w.dropped_projectiles_selected;
    j["dropped_projectiles_name_color"] = ColorToJson(w.dropped_projectiles_name_color);
    j["dropped_projectiles_icon_color"] = ColorToJson(w.dropped_projectiles_icon_color);
    j["dropped_projectiles_timer_color"] = ColorToJson(w.dropped_projectiles_timer_color);
    j["dropped_projectiles_distance_color"] = ColorToJson(w.dropped_projectiles_distance_color);
    return j;
}

void JsonToWorld(const nlohmann::json& j, WorldSettings& w) {
    if (j.contains("master")) JsonToConfigItem(j["master"], w.master);
    if (j.contains("masterKey")) JsonToKeyBind(j["masterKey"], w.masterKey);
    if (j.contains("bomb_indicator")) JsonToConfigItem(j["bomb_indicator"], w.bomb_indicator);
    if (j.contains("dropped_weapons")) JsonToConfigItem(j["dropped_weapons"], w.dropped_weapons);
    if (j.contains("dropped_projectiles")) JsonToConfigItem(j["dropped_projectiles"], w.dropped_projectiles);
    if (j.contains("dropped_weapons_name")) w.dropped_weapons_name = j["dropped_weapons_name"].get<bool>();
    if (j.contains("dropped_weapons_icon")) w.dropped_weapons_icon = j["dropped_weapons_icon"].get<bool>();
    if (j.contains("dropped_weapons_ammo")) w.dropped_weapons_ammo = j["dropped_weapons_ammo"].get<bool>();
    if (j.contains("dropped_weapons_distance")) w.dropped_weapons_distance = j["dropped_weapons_distance"].get<bool>();
    if (j.contains("dropped_weapons_name_color")) JsonToColor(j["dropped_weapons_name_color"], w.dropped_weapons_name_color);
    if (j.contains("dropped_weapons_icon_color")) JsonToColor(j["dropped_weapons_icon_color"], w.dropped_weapons_icon_color);
    if (j.contains("dropped_weapons_ammo_color")) JsonToColor(j["dropped_weapons_ammo_color"], w.dropped_weapons_ammo_color);
    if (j.contains("dropped_weapons_distance_color")) JsonToColor(j["dropped_weapons_distance_color"], w.dropped_weapons_distance_color);
    bool hasProjectileTypes = j.contains("dropped_projectiles_types") && j["dropped_projectiles_types"].is_array();
    if (hasProjectileTypes) {
        auto& arr = j["dropped_projectiles_types"];
        size_t count = std::min(arr.size(), w.dropped_projectiles_types.size());
        for (size_t i = 0; i < count; i++) {
            const auto& entry = arr[i];
            if (entry.contains("text")) w.dropped_projectiles_types[i].text = entry["text"].get<bool>();
            if (entry.contains("icon")) w.dropped_projectiles_types[i].icon = entry["icon"].get<bool>();
            if (entry.contains("timer")) w.dropped_projectiles_types[i].timer = entry["timer"].get<bool>();
            if (entry.contains("timer_bar")) w.dropped_projectiles_types[i].timer_bar = entry["timer_bar"].get<bool>();
            if (entry.contains("distance")) w.dropped_projectiles_types[i].distance = entry["distance"].get<bool>();
        }
    }
    if (!hasProjectileTypes) {
        bool legacyName = j.contains("dropped_projectiles_name") ? j["dropped_projectiles_name"].get<bool>() : w.dropped_projectiles_types[0].text;
        bool legacyIcon = j.contains("dropped_projectiles_icon") ? j["dropped_projectiles_icon"].get<bool>() : w.dropped_projectiles_types[0].icon;
        bool legacyTimer = j.contains("dropped_projectiles_timer") ? j["dropped_projectiles_timer"].get<bool>() : w.dropped_projectiles_types[0].timer;
        bool legacyDistance = j.contains("dropped_projectiles_distance") ? j["dropped_projectiles_distance"].get<bool>() : w.dropped_projectiles_types[0].distance;
        if (j.contains("dropped_projectile_icon")) legacyIcon = j["dropped_projectile_icon"].get<bool>();
        for (auto& t : w.dropped_projectiles_types) {
            t.text = legacyName;
            t.icon = legacyIcon;
            t.timer = legacyTimer;
            t.distance = legacyDistance;
        }
    }
    if (j.contains("dropped_projectiles_selected")) w.dropped_projectiles_selected = j["dropped_projectiles_selected"].get<int>();
    if (w.dropped_projectiles_selected < 0 || w.dropped_projectiles_selected >= (int)w.dropped_projectiles_types.size()) {
        w.dropped_projectiles_selected = 0;
    }
    if (j.contains("dropped_projectiles_name_color")) JsonToColor(j["dropped_projectiles_name_color"], w.dropped_projectiles_name_color);
    if (j.contains("dropped_projectiles_icon_color")) JsonToColor(j["dropped_projectiles_icon_color"], w.dropped_projectiles_icon_color);
    if (j.contains("dropped_projectiles_timer_color")) JsonToColor(j["dropped_projectiles_timer_color"], w.dropped_projectiles_timer_color);
    if (j.contains("dropped_projectiles_distance_color")) JsonToColor(j["dropped_projectiles_distance_color"], w.dropped_projectiles_distance_color);
}

nlohmann::json MiscToJson(const MiscSettings& m) {
    nlohmann::json j;
    j["master"] = ConfigItemToJson(m.master);
    j["masterKey"] = KeyBindToJson(m.masterKey);
    j["spectator_list"] = m.spectator_list;
    j["stream_proof"] = m.stream_proof;
    j["crosshair"] = CrosshairToJson(m.crosshair);
    j["hitmarker"] = HitmarkerToJson(m.hitmarker);
    j["hitsound_enabled"] = m.hitsound_enabled;
    j["hitsound_volume"] = m.hitsound_volume;
    j["screencolor_enabled"] = m.screencolor_enabled;
    j["screencolor"] = ColorToJson(m.screencolor);
    return j;
}

void JsonToMisc(const nlohmann::json& j, MiscSettings& m) {
    if (j.contains("master")) JsonToConfigItem(j["master"], m.master);
    if (j.contains("masterKey")) JsonToKeyBind(j["masterKey"], m.masterKey);
    if (j.contains("spectator_list")) m.spectator_list = j["spectator_list"].get<bool>();
    if (j.contains("stream_proof")) m.stream_proof = j["stream_proof"].get<bool>();
    if (j.contains("crosshair")) JsonToCrosshair(j["crosshair"], m.crosshair);
    if (j.contains("hitmarker")) JsonToHitmarker(j["hitmarker"], m.hitmarker);
    if (j.contains("hitsound_enabled")) m.hitsound_enabled = j["hitsound_enabled"].get<bool>();
    if (j.contains("hitsound_volume")) m.hitsound_volume = j["hitsound_volume"].get<float>();
    if (j.contains("screencolor_enabled")) m.screencolor_enabled = j["screencolor_enabled"].get<bool>();
    if (j.contains("screencolor")) JsonToColor(j["screencolor"], m.screencolor);
}

nlohmann::json AimToJson(const AimSettings& a) {
    nlohmann::json j;
    j["aimbot_enabled"] = a.aimbot.enabled;
    j["aimbot_vis_check"] = a.aimbot.vis_check_enabled;
    j["aimbot_lock_part"] = a.aimbot.lock_part;
    j["aimbot_visualise_target"] = a.aimbot.visualise_target;
    j["aimbot_target_tracer"] = a.aimbot.target_tracer;
    j["aimbot_fov_enabled"] = a.aimbot.fov_enabled;
    j["aimbot_team_check"] = a.aimbot.team_check;
    j["aimbot_fov_size"] = a.aimbot.fov_size;
    j["aimbot_smoothing"] = a.aimbot.smoothing;
    j["aimbot_key"] = KeyBindToJson(a.aimbot.key);
    j["trigger_enabled"] = a.trigger.enabled;
    j["trigger_team_check"] = a.trigger.team_check;
    j["trigger_key"] = KeyBindToJson(a.trigger.key);
    j["rcs_enabled"] = a.rcs.enabled;
    j["rcs_xy"] = Vector2ToJson(a.rcs.xy);
    j["rcs_shots_fired"] = a.rcs.shots_fired;
    return j;
}

void JsonToAim(const nlohmann::json& j, AimSettings& a) {
    if (j.contains("aimbot_enabled")) a.aimbot.enabled = j["aimbot_enabled"].get<bool>();
    if (j.contains("aimbot_vis_check")) a.aimbot.vis_check_enabled = j["aimbot_vis_check"].get<bool>();
    if (j.contains("aimbot_lock_part")) a.aimbot.lock_part = j["aimbot_lock_part"].get<int>();
    if (j.contains("aimbot_visualise_target")) a.aimbot.visualise_target = j["aimbot_visualise_target"].get<bool>();
    if (j.contains("aimbot_target_tracer")) a.aimbot.target_tracer = j["aimbot_target_tracer"].get<bool>();
    if (j.contains("aimbot_fov_enabled")) a.aimbot.fov_enabled = j["aimbot_fov_enabled"].get<bool>();
    if (j.contains("aimbot_team_check")) a.aimbot.team_check = j["aimbot_team_check"].get<bool>();
    if (j.contains("aimbot_fov_size")) a.aimbot.fov_size = j["aimbot_fov_size"].get<float>();
    if (j.contains("aimbot_smoothing")) a.aimbot.smoothing = j["aimbot_smoothing"].get<float>();
    if (j.contains("aimbot_key")) JsonToKeyBind(j["aimbot_key"], a.aimbot.key);
    if (j.contains("trigger_enabled")) a.trigger.enabled = j["trigger_enabled"].get<bool>();
    if (j.contains("trigger_team_check")) a.trigger.team_check = j["trigger_team_check"].get<bool>();
    if (j.contains("trigger_key")) JsonToKeyBind(j["trigger_key"], a.trigger.key);
    if (j.contains("rcs_enabled")) a.rcs.enabled = j["rcs_enabled"].get<bool>();
    if (j.contains("rcs_xy")) JsonToVector2(j["rcs_xy"], a.rcs.xy);
    if (j.contains("rcs_shots_fired")) a.rcs.shots_fired = j["rcs_shots_fired"].get<int>();
}

bool SaveConfigFile(const std::string& path) {
    nlohmann::json j;
    j["enemy"] = EspGroupToJson(enemy);
    j["team"] = EspGroupToJson(team);
    j["world"] = WorldToJson(world);
    j["misc"] = MiscToJson(misc);
    j["aim"] = AimToJson(aim);
    j["menuKey"] = KeyBindToJson(menuKey);
    j["hitsound_index"] = selectedHitsoundIndex;
    try {
        std::ofstream out(path);
        if (!out.is_open()) return false;
        out << j.dump(4);
        return true;
    }
    catch (...) {
        return false;
    }
}

void RefreshHitsoundList();

bool LoadConfigFile(const std::string& path) {
    try {
        std::ifstream in(path);
        if (!in.is_open()) return false;
        nlohmann::json j;
        in >> j;
        if (j.contains("enemy")) JsonToEspGroup(j["enemy"], enemy);
        if (j.contains("team")) JsonToEspGroup(j["team"], team);
        if (j.contains("world")) JsonToWorld(j["world"], world);
        if (j.contains("misc")) JsonToMisc(j["misc"], misc);
        if (j.contains("aim")) JsonToAim(j["aim"], aim);
        if (j.contains("menuKey")) JsonToKeyBind(j["menuKey"], menuKey);
        if (j.contains("hitsound_index")) selectedHitsoundIndex = j["hitsound_index"].get<int>();
        RefreshHitsoundList();
        return true;
    }
    catch (...) {
        return false;
    }
}

void RefreshMapList() {
    availableMaps.clear();
    if (!fs::exists("maps")) {
        try { fs::create_directory("maps"); }
        catch (...) {}
    }
    if (fs::exists("maps")) {
        for (const auto& entry : fs::directory_iterator("maps")) {
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
    if (!fs::exists("hitsounds")) {
        try { 
            fs::create_directory("hitsounds"); 
            std::cout << "[INFO] Created hitsounds folder. Place your .wav files there!\n";
        }
        catch (...) {}
    }
    if (fs::exists("hitsounds")) {
        for (const auto& entry : fs::directory_iterator("hitsounds")) {
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

ImU32 MultiplyU32Color(ImU32 color, float rgbMult, float alphaMult) {
    int r = (color >> IM_COL32_R_SHIFT) & 0xFF;
    int g = (color >> IM_COL32_G_SHIFT) & 0xFF;
    int b = (color >> IM_COL32_B_SHIFT) & 0xFF;
    int a = (color >> IM_COL32_A_SHIFT) & 0xFF;
    r = (int)ClampFloat(r * rgbMult, 0.0f, 255.0f);
    g = (int)ClampFloat(g * rgbMult, 0.0f, 255.0f);
    b = (int)ClampFloat(b * rgbMult, 0.0f, 255.0f);
    a = (int)ClampFloat(a * alphaMult, 0.0f, 255.0f);
    return IM_COL32(r, g, b, a);
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
    float flashDuration = mem.read<float>(pawnAddress + Offsets::m_flFlashDuration);
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
        if (fs::exists(pathInMaps)) {
            vCheck = VisCheck(pathInMaps);
            std::cout << "[INFO] Auto-Loaded: " << pathInMaps << std::endl;
        }
        else if (fs::exists(optName)) {
            vCheck = VisCheck(optName);
            std::cout << "[INFO] Auto-Loaded (Root): " << optName << std::endl;
        }
    }
}

// processa o estado do bind (hold, toggle, always on)
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

// controla a visibilidade do menu com o bind configurado
void UpdateMenuKey(KeyBind& bind) {
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
    if (bind.key == 0) return;
    bool keyDown = GetAsyncKeyState(bind.key) & 0x8000;
    static std::map<int, bool> keyWasDown;
    if (keyDown && !keyWasDown[bind.key]) {
        Globals::imguiVisible = !Globals::imguiVisible;
    }
    keyWasDown[bind.key] = keyDown;
}

// --- WEAPON & PROJECTILE HELPERS ---
// mapeia ids e classes para nomes legiveis

// converte id de arma para nome amigavel
std::string GetWeaponNameFromID(int weaponID) {
    // tabela fixa de ids para nomes usados no hud
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
    // procura o id e usa fallback quando nao encontra
    auto it = weaponNames.find(weaponID);
    // retorna o nome ou um texto padrao para ids desconhecidos
    return (it != weaponNames.end()) ? it->second : "Unknown";
}

// converte classe de projetil para nome curto
std::string GetProjectileNameFromClass(const std::string& className) {
    // mapeia por substring porque o nome vem da classe do jogo
    if (className.find("smokegrenade") != std::string::npos) return "Smoke";
    if (className.find("flashbang") != std::string::npos) return "Flash";
    if (className.find("hegrenade") != std::string::npos) return "HE Grenade";
    if (className.find("molotov") != std::string::npos || className.find("incgrenade") != std::string::npos) return "Molotov";
    // fallback para classes nao mapeadas
    return "Projectile";
}

// define tipo de projetil para UI e filtros
int GetProjectileType(const std::string& className) {
    // retorna indice padrao para arrays de configuracao
    if (className.find("smokegrenade") != std::string::npos) return 0;
    if (className.find("flashbang") != std::string::npos) return 1;
    if (className.find("hegrenade") != std::string::npos) return 2;
    if (className.find("molotov") != std::string::npos || className.find("incgrenade") != std::string::npos) return 3;
    // -1 significa que nao e projetil suportado
    return -1;
}

// --- DRAWING FUNCTIONS ---
// helpers de texto e formas para o esp

// desenha texto com contorno para melhor leitura
void DrawSkeetText(ImDrawList* drawList, ImFont* font, float fontSize, ImVec2 pos, ImU32 textColor, const char* text, bool centered = false) {
    // calcula tamanho para centralizar quando preciso
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    if (centered) pos.x -= floorf(textSize.x / 2.0f);
    // usa quatro sombras para criar contorno
    ImU32 black = IM_COL32(0, 0, 0, 255);
    drawList->AddText(font, fontSize, ImVec2(pos.x - 1, pos.y - 1), black, text);
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y - 1), black, text);
    drawList->AddText(font, fontSize, ImVec2(pos.x - 1, pos.y + 1), black, text);
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y + 1), black, text);
    drawList->AddText(font, fontSize, pos, textColor, text);
}

// desenha texto centralizado com sombra leve
void DrawNameText(ImDrawList* drawList, ImFont* font, float fontSize, ImVec2 pos, ImU32 textColor, const char* text) {
    // centraliza horizontalmente o texto
    ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
    pos.x -= floorf(textSize.x / 2.0f);
    // aplica sombra para contraste
    drawList->AddText(font, fontSize, ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 0, 0, 230), text);
    drawList->AddText(font, fontSize, pos, textColor, text);
}

// --- IMGUI HELPERS ---
// widgets utilitarios para o menu
float GetRightAlignedX(float offset);
// editor de cor com copiar/colar
void CustomColorEdit(const char* label, float* color) {
    std::string id = "##cp_" + std::string(label);
    ImGui::ColorEdit4(id.c_str(), color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    if (ImGui::BeginPopupContextItem(id.c_str())) {
        if (ImGui::MenuItem("Copy Color")) memcpy(Globals::copiedColor, color, sizeof(float) * 4);
        if (ImGui::MenuItem("Paste Color")) memcpy(color, Globals::copiedColor, sizeof(float) * 4);
        ImGui::EndPopup();
    }
}

// checkbox + seletor de cor padrao
void DrawConfigItem(const char* label, ConfigItem& item) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(GetRightAlignedX(26));
    CustomColorEdit(label, item.color);
}

// checkbox + duas cores (visivel/oculto)
void DrawConfigItemWithVisible(const char* label, ConfigItem& item, float* visibleColor, bool showVisible) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    if (showVisible) {
        ImGui::SetCursorPosX(GetRightAlignedX(52));
        CustomColorEdit((std::string(label) + "_hid").c_str(), item.color);
        ImGui::SameLine();
        CustomColorEdit((std::string(label) + "_vis").c_str(), visibleColor);
    }
    else {
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        CustomColorEdit(label, item.color);
    }
}

// desenha opcao de flag com cor editavel
void DrawFlagOption(const char* label, FlagItem& item) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(GetRightAlignedX(26));
    std::string id = std::string("##flg_") + label;
    CustomColorEdit(id.c_str(), item.color);
}

// widget de bind com modo e captura de tecla
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

// botao compacto de bind para o menu principal
void DrawMenuKeyButton(KeyBind& bind, ImVec2 size) {
    std::string label = bind.waitingForKey ? "[ ... ]" : "[ " + std::string(GetKeyNameSafe(bind.key)) + " ]";
    if (ImGui::Button(label.c_str(), size)) {
        bind.waitingForKey = !bind.waitingForKey;
    }
}

// switch master com bind integrado
void DrawMasterSwitch(const char* label, ConfigItem& item, KeyBind& bind) {
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(GetRightAlignedX(60));
    std::string bindId = "##bind_" + std::string(label);
    ImGui::PushID(bindId.c_str());
    DrawKeybindWidget(bind);
    ImGui::PopID();
}

// === GEAR ICON SYSTEM FOR SUB-MENUS ===
// icone de engrenagem para abrir submenus de configuracao
// desenha engrenagem com linhas e circulo central
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

// botao invisivel com desenho da engrenagem
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

// checkbox com botao de engrenagem para abrir ajustes
bool DrawConfigItemWithGear(const char* label, bool* enabled, const char* gearId, const char* tooltip = nullptr) {
    ImGui::Checkbox(label, enabled);
    
    if (*enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        return DrawGearButton(gearId, tooltip);
    }
    
    return false;
}

// === NEVERLOSE-STYLE ANIMATED TOGGLE (Simplified) ===
// toggle animado com interpolacao de estado
// desenha toggle animado no estilo neverlose
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
    static std::map<ImGuiID, float> animation_map;
    ImGuiID animId = ImGui::GetItemID();
    float& animation = animation_map[animId];
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

// calcula posicao alinhada a direita dentro do conteudo
float GetRightAlignedX(float offset) {
    return ImGui::GetContentRegionMax().x - offset;
}

// anima o valor para queda suave em barras (vida/armadura)
float GetAnimatedDropValue(std::map<int, float>& cache, int key, float target, bool enabled) {
    if (!enabled) {
        cache[key] = target;
        return target;
    }
    float& value = cache[key];
    if (value <= 0.0f || value > 1.0f) value = target;
    if (target > value) {
        value = target;
    } else {
        float dropSpeed = 2.5f;
        value -= dropSpeed * ImGui::GetIO().DeltaTime;
        if (value < target) value = target;
    }
    return value;
}

// === HELPER FUNCTIONS FOR TOGGLE + COLOR + GEAR ===
// combinacoes de toggle com cores e engrenagem
// toggle com uma cor
void DrawToggleWithColor(const char* label, ConfigItem& item) {
    DrawAnimatedToggle(label, &item.enabled);
    if (item.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        CustomColorEdit(label, item.color);
    }
}

// toggle com cor e submenu de engrenagem
void DrawToggleWithColorAndGear(const char* label, ConfigItem& item, const char* gearId, std::function<void()> gearContent) {
    DrawAnimatedToggle(label, &item.enabled);
    
    if (item.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(52));
        CustomColorEdit(label, item.color);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        if (DrawGearButton(gearId, nullptr)) {
            ImGui::OpenPopup(gearId);
        }
    }
    
    if (ImGui::BeginPopup(gearId)) {
        gearContent();
        ImGui::EndPopup();
    }
}

// toggle com duas cores (oculto/visivel)
void DrawToggleWithDoubleColor(const char* label, ConfigItem& item, float* visibleColor, bool showVisible) {
    DrawAnimatedToggle(label, &item.enabled);
    
    if (item.enabled) {
        ImGui::SameLine();
        if (showVisible) {
            ImGui::SetCursorPosX(GetRightAlignedX(52));
            CustomColorEdit((std::string(label) + "_hid").c_str(), item.color);
            ImGui::SameLine();
            CustomColorEdit((std::string(label) + "_vis").c_str(), visibleColor);
        } else {
            ImGui::SetCursorPosX(GetRightAlignedX(26));
            CustomColorEdit(label, item.color);
        }
    }
}

// toggle com duas cores e submenu
void DrawToggleWithDoubleColorAndGear(const char* label, ConfigItem& item, float* visibleColor, bool showVisible, const char* gearId, std::function<void()> gearContent) {
    DrawAnimatedToggle(label, &item.enabled);
    
    if (item.enabled) {
        ImGui::SameLine();
        
        if (showVisible) {
            ImGui::SetCursorPosX(GetRightAlignedX(78));
            CustomColorEdit((std::string(label) + "_hid").c_str(), item.color);
            ImGui::SameLine();
            CustomColorEdit((std::string(label) + "_vis").c_str(), visibleColor);
        } else {
            ImGui::SetCursorPosX(GetRightAlignedX(52));
            CustomColorEdit(label, item.color);
        }
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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
// configura tema visual do menu
// aplica cores e espacamentos do tema
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

// fundo pontilhado do menu para dar textura
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

// bordas e faixa colorida do menu
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
// renderiza colunas e paginas do menu principal

// coluna de configuracao do esp para time ou inimigo
void DrawESPColumn(const char* title, ESPGroup& group) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild(title, ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", title);
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

    // Snaplines
    DrawToggleWithColor("Snaplines", group.snaplines);
    
    // Out of FOV Arrows
    DrawAnimatedToggle("Out of FOV", &group.outOfFovEnabled);
    if (group.outOfFovEnabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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
    std::string ammoGearId = std::string("##ammoGear_") + title;
    DrawToggleWithColorAndGear("Ammo Bar", group.ammoBar, ammoGearId.c_str(), [&]() {
        ImGui::Text("Ammo Bar Settings");
        ImGui::Separator();
        DrawAnimatedToggle("Show Numbers", &group.ammo_show_numbers_below_max);
        DrawAnimatedToggle("Animate Drop", &group.ammo_animate_drop);
    });

    // Weapon with Gear
    std::string weaponGearId = std::string("##weaponGear_") + title;
    DrawAnimatedToggle("Weapon", &group.weapon.enabled);
    if (group.weapon.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        CustomColorEdit("##sndcol", group.sound.color);
    }

    // Armor Bar
    std::string armorGearId = std::string("##armorGear_") + title;
    DrawAnimatedToggle("Armor Bar", &group.armor.enabled);
    if (group.armor.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(52));
        CustomColorEdit("##armorcol", group.armor.static_color);
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        if (DrawGearButton(armorGearId.c_str(), "Armor Bar Settings")) {
            ImGui::OpenPopup(armorGearId.c_str());
        }
    }
    if (ImGui::BeginPopup(armorGearId.c_str())) {
        ImGui::Text("Armor Bar Settings");
        ImGui::Separator();
        DrawAnimatedToggle("Show Numbers", &group.armor.show_numbers_below_max);
        DrawAnimatedToggle("Animate Drop", &group.armor.animate_drop);
        ImGui::EndPopup();
    }

    // Health Bar with Gear
    std::string healthGearId = std::string("##healthGear_") + title;
    DrawAnimatedToggle("Health Bar", &group.health.enabled);
    if (group.health.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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
        DrawAnimatedToggle("Show Numbers", &group.health.show_numbers_below_max);
        DrawAnimatedToggle("Animate Drop", &group.health.animate_drop);
        ImGui::EndPopup();
    }

    // Flags with Gear
    std::string flagsGearId = std::string("##flagsGear_") + title;
    DrawAnimatedToggle("Flags", &group.flagsMaster.enabled);
    if (group.flagsMaster.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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

// coluna de configuracoes do world esp
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
    DrawAnimatedToggle("Dropped Weapons", &world.dropped_weapons.enabled);
    if (world.dropped_weapons.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        if (DrawGearButton("##droppedWeaponsGear", "Dropped Weapons Settings")) {
            ImGui::OpenPopup("##droppedWeaponsGear");
        }
    }
    if (ImGui::BeginPopup("##droppedWeaponsGear")) {
        ImGui::Text("Dropped Weapons");
        ImGui::Separator();
        ImGui::PushID("DroppedWeaponsSettings");
        DrawAnimatedToggle("Name", &world.dropped_weapons_name);
        if (world.dropped_weapons_name) { ImGui::SameLine(); CustomColorEdit("##dwNameCol", world.dropped_weapons_name_color); }
        DrawAnimatedToggle("Icon", &world.dropped_weapons_icon);
        if (world.dropped_weapons_icon) { ImGui::SameLine(); CustomColorEdit("##dwIconCol", world.dropped_weapons_icon_color); }
        DrawAnimatedToggle("Ammo", &world.dropped_weapons_ammo);
        if (world.dropped_weapons_ammo) { ImGui::SameLine(); CustomColorEdit("##dwAmmoCol", world.dropped_weapons_ammo_color); }
        DrawAnimatedToggle("Distance", &world.dropped_weapons_distance);
        if (world.dropped_weapons_distance) { ImGui::SameLine(); CustomColorEdit("##dwDistCol", world.dropped_weapons_distance_color); }
        ImGui::PopID();
        ImGui::EndPopup();
    }

    DrawAnimatedToggle("Dropped Projectiles", &world.dropped_projectiles.enabled);
    if (world.dropped_projectiles.enabled) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        if (DrawGearButton("##droppedProjectilesGear", "Dropped Projectiles Settings")) {
            ImGui::OpenPopup("##droppedProjectilesGear");
        }
    }
    if (ImGui::BeginPopup("##droppedProjectilesGear")) {
        ImGui::Text("Dropped Projectiles");
        ImGui::Separator();
        ImGui::PushID("DroppedProjectilesSettings");
        const char* projectileLabels[] = { "Smoke", "Flash", "HE", "Molotov", "Decoy" };
        ImGui::Text("Type");
        ImGui::ListBox("##dpTypeList", &world.dropped_projectiles_selected, projectileLabels, IM_ARRAYSIZE(projectileLabels), 5);
        ImGui::Separator();
        ProjectileTypeSettings& selected = world.dropped_projectiles_types[world.dropped_projectiles_selected];
        DrawAnimatedToggle("Text", &selected.text);
        if (selected.text) { ImGui::SameLine(); CustomColorEdit("##dpNameCol", world.dropped_projectiles_name_color); }
        DrawAnimatedToggle("Icon", &selected.icon);
        if (selected.icon) { ImGui::SameLine(); CustomColorEdit("##dpIconCol", world.dropped_projectiles_icon_color); }
        DrawAnimatedToggle("Timer", &selected.timer);
        if (selected.timer) { ImGui::SameLine(); CustomColorEdit("##dpTimerCol", world.dropped_projectiles_timer_color); }
        DrawAnimatedToggle("Timer Bar", &selected.timer_bar);
        DrawAnimatedToggle("Distance", &selected.distance);
        if (selected.distance) { ImGui::SameLine(); CustomColorEdit("##dpDistCol", world.dropped_projectiles_distance_color); }
        ImGui::PopID();
        ImGui::EndPopup();
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// coluna de configuracoes diversas
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
        ImGui::SetCursorPosX(GetRightAlignedX(52));
        CustomColorEdit("##crosshaircol", misc.crosshair.color);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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
        ImGui::SetCursorPosX(GetRightAlignedX(52));
        CustomColorEdit("##hitmarkercol", misc.hitmarker.color);
        
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
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
        ImGui::SameLine();
        ImGui::SetCursorPosX(GetRightAlignedX(26));
        if (DrawGearButton("##hitsoundGear", "Hitsound Settings")) {
            ImGui::OpenPopup("##hitsoundGear");
        }
    }
    
    if (ImGui::BeginPopup("##hitsoundGear")) {
        ImGui::Text("Hitsound Settings");
        ImGui::Separator();
        ImGui::SliderFloat("Volume", &misc.hitsound_volume, 0.0f, 100.0f, "%.0f%%");
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
            ApplyHitsoundVolume(misc.hitsound_volume);
            std::wstring wpath(currentHitsoundPath.begin(), currentHitsoundPath.end());
            PlaySoundW(wpath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        }
        
        ImGui::EndPopup();
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
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawAimbotColumn() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild("Aimbot", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Aimbot");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));

    DrawAnimatedToggle("Enabled", &aim.aimbot.enabled);
    DrawAnimatedToggle("Visual Check", &aim.aimbot.vis_check_enabled);
    ImGui::Text("Body Part");
    ImGui::Combo("##aim_part", &aim.aimbot.lock_part, kAimbotLockParts, IM_ARRAYSIZE(kAimbotLockParts));
    DrawAnimatedToggle("Visualise Target", &aim.aimbot.visualise_target);
    DrawAnimatedToggle("Target Tracer", &aim.aimbot.target_tracer);
    DrawAnimatedToggle("Show FOV", &aim.aimbot.fov_enabled);
    DrawAnimatedToggle("Teamcheck", &aim.aimbot.team_check);
    ImGui::SliderFloat("FOV Size", &aim.aimbot.fov_size, 1.0f, 1000.0f, "%.0f");
    ImGui::SliderFloat("Smoothing", &aim.aimbot.smoothing, 1.0f, 30.0f, "%.2f");
    ImGui::Text("Keybind");
    DrawKeybindWidget(aim.aimbot.key);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawTriggerbotColumn() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild("Triggerbot", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Triggerbot");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));

    DrawAnimatedToggle("Enabled", &aim.trigger.enabled);
    DrawAnimatedToggle("Teamcheck", &aim.trigger.team_check);
    ImGui::Text("Keybind");
    DrawKeybindWidget(aim.trigger.key);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void DrawRcsColumn() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGui::BeginChild("RCS", ImVec2(0, 0), true, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "RCS");
        ImGui::EndMenuBar();
    }
    ImGui::Dummy(ImVec2(0, 3));

    DrawAnimatedToggle("Enabled", &aim.rcs.enabled);
    ImGui::Text("X");
    ImGui::SliderFloat("##rcs_x", &aim.rcs.xy.x, 0.0f, 2.0f, "%.1f");
    ImGui::Text("Y");
    ImGui::SliderFloat("##rcs_y", &aim.rcs.xy.y, 0.0f, 2.0f, "%.1f");
    ImGui::Text("Shots Fired");
    ImGui::SliderInt("##rcs_shots", &aim.rcs.shots_fired, 0, 10);

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

// coluna de configuracoes de mapa e arquivos
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
                if (fs::exists(path)) {
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

// renderiza lista de espectadores na tela
void DrawSpectatorList(const std::vector<Entity>& spectators) {
    if (spectators.empty() || !misc.spectator_list) return;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::SetNextWindowSize(ImVec2(250, 200));
    ImGui::Begin("Spectator List", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "SPECTATORS");
    ImGui::Separator();
    for (const Entity& entity : spectators) {
        ImGui::Text("%s", entity.name.c_str());
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

// desenha crosshair customizada
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

// desenha hitmarker com base no total de hits do servidor
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
        const float t = (float)i / (segments * 0.5f);
        const float currentAngle = angle + Math::HALF_PI + t * Math::PI;
        points.push_back(ImVec2(p1.x + radius * std::cos(currentAngle), p1.y + radius * std::sin(currentAngle)));
    }
    
    for (int i = 0; i <= segments / 2; i++) {
        const float t = (float)i / (segments * 0.5f);
        const float currentAngle = angle - Math::HALF_PI + t * Math::PI;
        points.push_back(ImVec2(p2.x + radius * std::cos(currentAngle), p2.y + radius * std::sin(currentAngle)));
    }
    
    return points;
}

// === NEW WORLD ESP DRAWING FUNCTIONS ===
// funcoes de renderizacao do world esp

// placeholder de spread (desativado)
void DrawWeaponSpread(ImDrawList* drawList, int monitorWidth, int monitorHeight, float spread) {
    // Função removida - spread foi removido
    return;
}

// overlay de cor na tela inteira
void DrawScreenColorOverlay(ImDrawList* drawList, int monitorWidth, int monitorHeight) {
    if (!misc.screencolor_enabled) return;
    
    // Desenhar ANTES de tudo para não afetar as features
    ImU32 color = ColorToU32(misc.screencolor);
    
    // Criar um drawlist separado que será renderizado primeiro
    ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList();
    bgDrawList->AddRectFilled(ImVec2(0, 0), ImVec2((float)monitorWidth, (float)monitorHeight), color);
}


// --- MAIN LOOP ---
// fluxo principal: init, offsets, janela, loop de render
int main()
{
    InitKeyNames();
    RefreshMapList();
    RefreshHitsoundList();
    menuKey.key = VK_INSERT;
    menuKey.mode = KeyMode::Toggle;

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
    HWND cs2Window = FindWindowW(NULL, L"Counter-Strike 2");
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
    std::cout << "[INFO] Press " << GetKeyNameSafe(menuKey.key) << " to show/hide menu.\n";
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
            UpdateKeybind(aim.aimbot.key);
            UpdateKeybind(aim.trigger.key);
            UpdateMenuKey(menuKey);

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
            static ImVec2 menuSize = ImVec2(900, 680);
            ImGui::SetNextWindowSize(menuSize, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(700, 520), ImVec2(1500, 1000));
            ImGui::Begin("VORTIX PRO", nullptr, ImGuiWindowFlags_NoCollapse);
            {
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 windowSize = ImGui::GetWindowSize();
                menuSize = windowSize;
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                
                float topBarHeight = 60.0f;
                drawList->AddRectFilled(
                    ImVec2(windowPos.x, windowPos.y), 
                    ImVec2(windowPos.x + windowSize.x, windowPos.y + topBarHeight),
                    IM_COL32(16, 16, 16, 255)
                );
                
                // Logo Circle (Neverlose-style)
                drawList->AddCircleFilled(ImVec2(windowPos.x + 25, windowPos.y + 26), 12, IM_COL32(143, 209, 98, 255), 32);
                drawList->AddText(tahomaBoldFont, 16.0f, ImVec2(windowPos.x + 20, windowPos.y + 19), IM_COL32(30, 30, 30, 255), "V");
                
                // Title
                drawList->AddText(tahomaBoldFont, 16.0f, ImVec2(windowPos.x + 48, windowPos.y + 18), IM_COL32(230, 230, 230, 255), "VORTIX PRO");
                drawList->AddText(tahomaRegularFont, 11.0f, ImVec2(windowPos.x + 48, windowPos.y + 34), IM_COL32(130, 130, 130, 255), "Premium CS2 External");
                
                ImGui::SetCursorPos(ImVec2(windowSize.x - 220, 18));
                DrawMenuKeyButton(menuKey, ImVec2(90, 25));
                ImGui::SameLine();
                if (ImGui::Button("Save##TopBar", ImVec2(50, 25))) {
                    WindowStyleState state = BeginFileDialog(renderer.hwnd);
                    std::string path = SaveJsonFileDialog(renderer.hwnd);
                    EndFileDialog(renderer.hwnd, state);
                    if (!path.empty()) SaveConfigFile(path);
                }
                ImGui::SameLine();
                if (ImGui::Button("Load##TopBar", ImVec2(50, 25))) {
                    WindowStyleState state = BeginFileDialog(renderer.hwnd);
                    std::string path = OpenJsonFileDialog(renderer.hwnd);
                    EndFileDialog(renderer.hwnd, state);
                    if (!path.empty()) LoadConfigFile(path);
                }
                
                // Separator Line
                drawList->AddLine(
                    ImVec2(windowPos.x, windowPos.y + topBarHeight),
                    ImVec2(windowPos.x + windowSize.x, windowPos.y + topBarHeight),
                    IM_COL32(40, 40, 40, 255),
                    1.0f
                );
                
                // Content Area
                ImGui::SetCursorPosY(topBarHeight + 10.0f);
                
                if (ImGui::BeginTabBar("##MainTabBar", ImGuiTabBarFlags_None)) {
                    if (ImGui::BeginTabItem("Aim")) {
                        ImGui::Spacing();
                        ImGui::Columns(3, nullptr, false);
                        DrawAimbotColumn();
                        ImGui::NextColumn();
                        DrawTriggerbotColumn();
                        ImGui::NextColumn();
                        DrawRcsColumn();
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
            bool isForeground = (GetForegroundWindow() == cs2Window);
            uintptr_t playerPawnAddress = cs2.read<uintptr_t>(client + Offsets::dwLocalPlayerPawn);
            
            // Validate addresses before proceeding
            if (!playerPawnAddress || !entityList || !client) {
                renderer.EndRender();
                Sleep(10);
                continue;
            }
            
            Entity localPlayerEntity{ cs2, playerPawnAddress };
            bool isLocalPlayerScoped = cs2.read<bool>(playerPawnAddress + Offsets::m_bIsScoped);

            UpdateMapVisCheck(cs2, client);

        spectatingPlrs.clear();

        if (world.master.enabled && world.masterKey.active) {
            if (world.bomb_indicator.enabled) {
                try {
                    uintptr_t globalVars = cs2.read<uintptr_t>(client + Offsets::dwGlobalVars);
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
                        uintptr_t currentWeapon = cs2.read<uintptr_t>(currentPawn + Offsets::m_pClippingWeapon);
                        if (!currentWeapon) {
                            continue;
                        }
                        short weaponIndex = cs2.read<short>(currentWeapon + Offsets::m_AttributeManager + Offsets::m_Item + Offsets::m_iItemDefinitionIndex);
                        if (weaponIndex == 49) {
                            if (cs2.read<bool>(currentWeapon + Offsets::m_bIsPlanting)) {
                                Globals::isPlanting = true;
                                break;
                            }
                        }
                    }
                }
                if (Globals::isPlanting) {
                    DrawSkeetText(drawList, tahomaBoldFont, 24.0f, { (float)monitorWidth / 2.0f, (float)monitorHeight / 3.0f }, IM_COL32(255, 0, 0, 255), "ENEMY PLANTING", true);
                }
                bool isPlanted = false;
                uintptr_t gameRules = cs2.read<uintptr_t>(client + Offsets::dwGameRules);
                if (gameRules) isPlanted = cs2.read<bool>(gameRules + Offsets::m_bBombPlanted);
                if (!isPlanted) { Globals::isBombPlanted = false; Globals::bombTimeLeft = 0.0f; }
                else {
                    Globals::isBombPlanted = true;
                    uintptr_t plantedC4Address = cs2.read<uintptr_t>(client + Offsets::dwPlantedC4);
                    uintptr_t c4Entity = cs2.read<uintptr_t>(plantedC4Address);
                    if (c4Entity) {
                        float c4BlowTime = cs2.read<float>(c4Entity + Offsets::m_flC4Blow);
                        float defuseCountDown = cs2.read<float>(c4Entity + Offsets::m_flDefuseCountDown);
                        Globals::isBeingDefused = cs2.read<bool>(c4Entity + Offsets::m_bBeingDefused);
                        Globals::bombSite = cs2.read<int>(c4Entity + Offsets::m_nBombSite);
                        Globals::bombTimeLeft = c4BlowTime - serverTime;
                        Globals::defuseTimeLeft = Globals::isBeingDefused ? (defuseCountDown - serverTime) : 0.0f;
                        if (Globals::bombTimeLeft <= 0.0f) { Globals::bombTimeLeft = 0.0f; Globals::isBombPlanted = false; }
                        if (Globals::isBombPlanted && isForeground) {
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

        int entityIndex = cs2.read<int>(playerPawnAddress + Offsets::m_iIDEntIndex);

        if (aim.aimbot.enabled && aim.aimbot.fov_enabled && isForeground) {
            drawList->AddCircle({ (float)monitorWidth / 2.0f, (float)monitorHeight / 2.0f }, aim.aimbot.fov_size, IM_COL32(255, 0, 0, 255));
        }

        if (aim.rcs.enabled && isForeground) {
            if (localPlayerEntity.shotsFired > aim.rcs.shots_fired) {
                Vector2 viewAngles{ cs2.read<Vector2>(playerPawnAddress + Offsets::m_angEyeAngles) };
                Vector2 delta{ viewAngles - (viewAngles + (rcsOldPunch - (localPlayerEntity.aimPunchAngle * 2.0f))) };

                float sensitivity{ cs2.read<float>(cs2.read<uintptr_t>(client + Offsets::dwSensitivity) + Offsets::dwSensitivity_sensitivity) };

                int x{ static_cast<int>(std::round((delta.y * aim.rcs.xy.x / sensitivity) / 0.044f)) };
                int y{ static_cast<int>(std::round((delta.x * aim.rcs.xy.y / sensitivity) / 0.044f)) };

                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    NtUserInjectMouseInput::NTInjectInput(x, -y);
                    rcsOldPunch = localPlayerEntity.aimPunchAngle * 2.0f;
                }
            }
            else {
                rcsOldPunch = { 0.0f, 0.0f };
            }
        }

        if (aim.trigger.enabled && aim.trigger.key.active && isForeground && entityIndex > 0) {
            uintptr_t tbEntry{ cs2.read<uintptr_t>(entityList + 0x8 * ((entityIndex & 0x7FFF) >> 9) + 16) };
            if (tbEntry) {
                uintptr_t tbPawn{ cs2.read<uintptr_t>(tbEntry + 112 * (entityIndex & 0x1FF)) };
                if (tbPawn) {
                    Entity pawnEntity{ cs2, tbPawn };
                    bool canShoot = pawnEntity.health > 0;
                    if (aim.trigger.team_check && pawnEntity.team == localPlayerEntity.team) canShoot = false;
                    if (canShoot) {
                        NtUserInjectMouseInput::INJECTED_INPUT_MOUSE_INFO mInfo[1]{};
                        mInfo[0].PixelDeltaX = 0;
                        mInfo[0].PixelDeltaY = 0;
                        mInfo[0].MouseOptions = NtUserInjectMouseInput::IIMO_LeftDown;
                        mInfo[0].ExtraInfo = (ULONG_PTR)GetMessageExtraInfo();
                        NtUserInjectMouseInput::NTInjectMouseInput(mInfo, 1);

                        mInfo[0].MouseOptions = NtUserInjectMouseInput::IIMO_LeftUp;
                        NtUserInjectMouseInput::NTInjectMouseInput(mInfo, 1);
                    }
                }
            }
        }

        uintptr_t localBoneMatrix = getBoneMatrix(cs2, playerPawnAddress);
        Vector3 localEyePos{ 0, 0, 0 };
        if (localBoneMatrix) {
            localEyePos = cs2.read<Vector3>(localBoneMatrix + (6 * 32));
        }
        
        bool aimShouldTarget = aim.aimbot.enabled && isForeground && (aim.aimbot.key.active || aim.aimbot.visualise_target || aim.aimbot.target_tracer);
        std::vector<Entity> aimbotTargets;

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
            
            if (aimShouldTarget) {
                bool check{ true };
                if (currentPawn == playerPawnAddress) check = false;
                if (entity.health <= 0) check = false;
                if (aim.aimbot.team_check && entity.team == localPlayerEntity.team) check = false;
                if (check) aimbotTargets.push_back(entity);
            }

            ESPGroup* currentConfig = (entity.team == localPlayerEntity.team) ? &team : &enemy;

            if (currentConfig->master.enabled && currentConfig->masterKey.active && isForeground)
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
                        ImU32 boxCol = ColorToU32(currentConfig->box.color);
                        ImU32 skelCol = ColorToU32(currentConfig->skeleton.color);
                        bool isVisible = false;
                        bool needBoneData = currentConfig->skeleton.enabled || currentConfig->visCheckEnabled;

                        if (needBoneData) {
                            uintptr_t boneMatrix = getBoneMatrix(cs2, currentPawn);
                            if (boneMatrix) {
                                std::array<Vector3, 28> boneCache{};
                                std::array<bool, 28> boneCached{};
                                auto ReadBone = [&](int index) -> Vector3 {
                                    if (!boneCached[index]) {
                                        boneCache[index] = cs2.read<Vector3>(boneMatrix + index * 32);
                                        boneCached[index] = true;
                                    }
                                    return boneCache[index];
                                };
                                std::array<Vector2, 28> boneScreenCache{};
                                std::array<bool, 28> boneScreenCached{};
                                auto ReadBoneScreen = [&](int index) -> Vector2 {
                                    if (!boneScreenCached[index]) {
                                        boneScreenCache[index] = WorldToScreen(ReadBone(index), viewMatrix);
                                        boneScreenCached[index] = true;
                                    }
                                    return boneScreenCache[index];
                                };

                                if (currentConfig->visCheckEnabled) {
                                    Vector3 enemyHead = ReadBone(6);
                                    if (vCheck.IsPointVisible(enemyHead, localEyePos)) {
                                        isVisible = true;
                                    }
                                    else {
                                        Vector3 enemyChest = ReadBone(4);
                                        if (vCheck.IsPointVisible(enemyChest, localEyePos)) isVisible = true;
                                    }
                                    if (isVisible) {
                                        boxCol = ColorToU32(currentConfig->boxVisibleColor);
                                        skelCol = ColorToU32(currentConfig->skeletonVisibleColor);
                                    }
                                }

                                if (currentConfig->skeleton.enabled) {
                                    for (const auto& pair : kSkeletonLines) {
                                        Vector2 sv1 = ReadBoneScreen(pair.first);
                                        Vector2 sv2 = ReadBoneScreen(pair.second);
                                        if (sv1.x != 0 && sv2.x != 0)
                                            drawList->AddLine({ sv1.x, sv1.y }, { sv2.x, sv2.y }, skelCol, 1.0f);
                                    }
                                }
                            }
                        }
                        
                        if (currentConfig->sound.enabled && Offsets::m_vecAbsVelocity && Offsets::m_fFlags) {
                            Vector3 vel = cs2.read<Vector3>(currentPawn + Offsets::m_vecAbsVelocity);
                            float speed = std::sqrt((vel.x * vel.x) + (vel.y * vel.y));
                            float dx = localPlayerEntity.position.x - entity.position.x;
                            float dy = localPlayerEntity.position.y - entity.position.y;
                            float dz = localPlayerEntity.position.z - entity.position.z;
                            float distSq = dx * dx + dy * dy + dz * dz;
                            int flags = cs2.read<int>(currentPawn + Offsets::m_fFlags);
                            bool onGround = (flags & 1) != 0;
                            bool wasOnGround = entityWasOnGround[i];
                            bool justLanded = (!wasOnGround && onGround);
                            entityWasOnGround[i] = onGround;
                            Vector3 ripplePos = entity.position;
                            if (Offsets::m_pGameSceneNode && Offsets::m_vecAbsOrigin) {
                                uintptr_t pawnNode = cs2.read<uintptr_t>(currentPawn + Offsets::m_pGameSceneNode);
                                if (pawnNode) {
                                    Vector3 nodePos = cs2.read<Vector3>(pawnNode + Offsets::m_vecAbsOrigin);
                                    if (!(nodePos.x == 0.0f && nodePos.y == 0.0f && nodePos.z == 0.0f)) {
                                        ripplePos = nodePos;
                                    }
                                }
                            }
                            ULONGLONG nowTick = GetTickCount64();
                            ImU32 stepColor = ColorToU32(currentConfig->sound.color);
                            ImU32 landColor = (stepColor & 0x00FFFFFF) | (200 << IM_COL32_A_SHIFT);
                            auto addRipple = [&](ImU32 color) {
                                auto lastIt = entityLastRippleTime.find(i);
                                if (lastIt != entityLastRippleTime.end() && nowTick - lastIt->second < 200) return;
                                entityLastRippleTime[i] = nowTick;
                                activeRipples.push_back({
                                    ripplePos,
                                    nowTick,
                                    30.0f,
                                    700.0f,
                                    color
                                });
                            };
                            const float minSpeed = 160.0f;
                            const float maxStepDist = 1100.0f;
                            const float maxLandDist = 1600.0f;
                            if (onGround && speed > minSpeed && distSq < (maxStepDist * maxStepDist)) {
                                addRipple(stepColor);
                            }
                            if (justLanded && distSq < (maxLandDist * maxLandDist)) {
                                addRipple(landColor);
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

                        // codigo do health bar começa aqui
                        if (currentConfig->health.enabled) {
                            float healthPercRaw = ClampFloat((float)entity.health / 100.0f, 0.0f, 1.0f);
                            float healthPerc = GetAnimatedDropValue(healthBarAnimValues, i, healthPercRaw, currentConfig->health.animate_drop);
                            float barWidth = 2.0f;
                            float x = bL - 5.0f - barWidth;
                            drawList->AddRectFilled({ x - 1.0f, bT - 1.0f }, { x + barWidth + 1.0f, bB + 1.0f }, IM_COL32(0, 0, 0, 150));
                            if (entity.health > 0) {
                                ImU32 healthCol = (currentConfig->health.mode == 0) ? ColorToU32(currentConfig->health.static_color) : GetGradientColor(healthPercRaw, currentConfig->health.gradient_low, currentConfig->health.gradient_high);
                                float h = floorf(bH * healthPerc);
                                drawList->AddRectFilled({ x, bB - h }, { x + barWidth, bB }, healthCol);
                                if (currentConfig->health.show_numbers_below_max && entity.health < 100) {
                                    char hpText[10]; sprintf_s(hpText, "%d", entity.health);
                                    ImVec2 tSize = gamesenseFont->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, hpText);
                                    float tY = bB - h - 3.0f; if (tY < bT) tY = bT;
                                    float tX = x + (barWidth / 2.0f) - (tSize.x / 2.0f);
                                    DrawSkeetText(drawList, gamesenseFont, 10.0f, { tX, tY }, IM_COL32(255, 255, 255, 255), hpText, false);
                                }
                            }
                        }
                        // codigo do health bar termina aqui

                        if (currentConfig->armor.enabled) {
                            float armorPercRaw = ClampFloat((float)entity.armor / 100.0f, 0.0f, 1.0f);
                            float armorPerc = GetAnimatedDropValue(armorBarAnimValues, i, armorPercRaw, currentConfig->armor.animate_drop);
                            float barWidth = 2.0f;
                            float x = bR + 5.0f;
                            drawList->AddRectFilled({ x - 1.0f, bT - 1.0f }, { x + barWidth + 1.0f, bB + 1.0f }, IM_COL32(0, 0, 0, 150));
                            if (entity.armor > 0) {
                                float h = floorf(bH * armorPerc);
                                drawList->AddRectFilled({ x, bB - h }, { x + barWidth, bB }, ColorToU32(currentConfig->armor.static_color));
                                if (currentConfig->armor.show_numbers_below_max && entity.armor < 100) {
                                    char arText[10]; sprintf_s(arText, "%d", entity.armor);
                                    ImVec2 tSize = gamesenseFont->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, arText);
                                    float tY = bB - h - 3.0f; if (tY < bT) tY = bT;
                                    float tX = x + (barWidth / 2.0f) - (tSize.x / 2.0f);
                                    DrawSkeetText(drawList, gamesenseFont, 10.0f, { tX, tY }, IM_COL32(255, 255, 255, 255), arText, false);
                                }
                            }
                        }

                        if (currentConfig->name.enabled) {
                            DrawNameText(drawList, gamesenseFont, 12.0f, { bL + (bR - bL) * 0.5f, bT - 15.0f }, ColorToU32(currentConfig->name.color), entity.name.c_str());
                        }

                        float centerX = floorf(bL + (bR - bL) * 0.5f);
                        float bottomY = bB + 4.0f;

                        if (currentConfig->ammoBar.enabled) {
                            uintptr_t clippingWeapon = cs2.read<uintptr_t>(entity.currentPawn + Offsets::m_pClippingWeapon);
                            if (clippingWeapon != 0) {
                                int currentAmmo = cs2.read<int>(clippingWeapon + Offsets::m_iClip1);
                                int maxAmmo = GetMaxClip(entity.weaponName);
                                if (maxAmmo > 0) {
                                    float ammoPercRaw = ClampFloat((float)currentAmmo / (float)maxAmmo, 0.0f, 1.0f);
                                    float ammoPerc = GetAnimatedDropValue(ammoBarAnimValues, i, ammoPercRaw, currentConfig->ammo_animate_drop);
                                    float barHeight = 2.0f; float width = bR - bL;
                                    drawList->AddRectFilled({ bL - 1.0f, bottomY - 1.0f }, { bR + 1.0f, bottomY + barHeight + 1.0f }, IM_COL32(0, 0, 0, 150));
                                    if (currentAmmo > 0) {
                                        float w = floorf(width * ammoPerc);
                                        drawList->AddRectFilled({ bL, bottomY }, { bL + w, bottomY + barHeight }, ColorToU32(currentConfig->ammoBar.color));
                                        if (currentConfig->ammo_show_numbers_below_max && currentAmmo < maxAmmo) {
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
                                uintptr_t mS = cs2.read<uintptr_t>(currentController + Offsets::m_pInGameMoneyServices);
                                if (mS) { int money = cs2.read<int>(mS + Offsets::m_iAccount); if (money > 0) DrawFlag(("$" + std::to_string(money)).c_str(), currentConfig->flagMoney.color); }
                            }
                            if (currentConfig->flagArmor.enabled && entity.armor > 0) { bool helmet = cs2.read<bool>(entity.currentPawn + Offsets::m_bHasHelmet); DrawFlag(helmet ? "HK" : "K", currentConfig->flagArmor.color); }
                            if (currentConfig->flagZoom.enabled && cs2.read<bool>(entity.currentPawn + Offsets::m_bIsScoped)) DrawFlag("ZOOM", currentConfig->flagZoom.color);
                            if (currentConfig->flagKit.enabled) { uintptr_t iS = cs2.read<uintptr_t>(entity.currentPawn + Offsets::m_pItemServices); if (iS && cs2.read<bool>(iS + Offsets::m_bHasDefuser)) DrawFlag("KIT", currentConfig->flagKit.color); }
                            if (currentConfig->flagBlind.enabled && IsBlinded(cs2, entity.currentPawn)) DrawFlag("FLASH", currentConfig->flagBlind.color);
                        }

                        if (currentConfig->snaplines.enabled && screenPos.x != 0.0f && screenPos.y != 0.0f) {
                            drawList->AddLine({monitorWidth / 2.0f, (float)monitorHeight}, {screenPos.x, screenPos.y}, ColorToU32(currentConfig->snaplines.color));
                        }
                        
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
                                float dirX = cosf(screenAngle);
                                float dirY = sinf(screenAngle);
                                float rightX = cosf(screenAngle + 1.5707963f);
                                float rightY = sinf(screenAngle + 1.5707963f);
                                auto buildArrow = [&](float scale, ImVec2* outPts) {
                                    float head = size * scale;
                                    float tail = size * 0.9f * scale;
                                    float body = size * 0.25f * scale;
                                    float halfWidth = size * 0.6f * scale;
                                    outPts[0] = { arrowX + dirX * head, arrowY + dirY * head };
                                    outPts[1] = { arrowX - dirX * body + rightX * halfWidth, arrowY - dirY * body + rightY * halfWidth };
                                    outPts[2] = { arrowX - dirX * tail, arrowY - dirY * tail };
                                    outPts[3] = { arrowX - dirX * body - rightX * halfWidth, arrowY - dirY * body - rightY * halfWidth };
                                };
                                ImU32 arrowColor = ColorToU32(currentConfig->outOfFovColor);
                                ImU32 glowColor1 = (arrowColor & 0x00FFFFFF) | (60 << IM_COL32_A_SHIFT);
                                ImU32 glowColor2 = (arrowColor & 0x00FFFFFF) | (30 << IM_COL32_A_SHIFT);
                                ImVec2 glowPts1[4];
                                ImVec2 glowPts2[4];
                                ImVec2 arrowPts[4];
                                buildArrow(1.6f, glowPts2);
                                buildArrow(1.35f, glowPts1);
                                buildArrow(1.0f, arrowPts);
                                drawList->AddConvexPolyFilled(glowPts2, 4, glowColor2);
                                drawList->AddConvexPolyFilled(glowPts1, 4, glowColor1);
                                drawList->AddConvexPolyFilled(arrowPts, 4, arrowColor);
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
                            uintptr_t observerServices = cs2.read<uintptr_t>(pawn + Offsets::m_pObserverServices);
                            if (observerServices) {
                                int observerTarget = cs2.read<int>(observerServices + Offsets::m_hObserverTarget);
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

        if (aimShouldTarget) {
            Vector3 bestPartPos{ 0.0f, 0.0f, 0.0f };
            Vector2 bestScreenPos{ 0.0f, 0.0f };
            float closestDist = aim.aimbot.fov_size;
            bool found = false;
            bool canVisCheck = aim.aimbot.vis_check_enabled && (localEyePos.x != 0.0f || localEyePos.y != 0.0f || localEyePos.z != 0.0f);

            auto ResolveBoneIndex = [&](int lockPart) {
                switch (lockPart) {
                case 1: return BoneIndex::CHEST;
                case 2: return BoneIndex::LARM;
                case 3: return BoneIndex::RARM;
                case 4: return BoneIndex::LCALF;
                case 5: return BoneIndex::RCALF;
                default: return BoneIndex::HEAD;
                }
            };

            for (const auto& entity : aimbotTargets) {
                uintptr_t boneMatrix = getBoneMatrix(cs2, entity.currentPawn);
                if (!boneMatrix) continue;
                int boneIndex = ResolveBoneIndex(aim.aimbot.lock_part);
                Vector3 partPos = cs2.read<Vector3>(boneMatrix + (boneIndex * 0x20));
                Vector2 screenPos = WorldToScreen(partPos, viewMatrix);
                if (screenPos.x == 0.0f && screenPos.y == 0.0f) continue;

                float dx = screenPos.x - (float)monitorWidth / 2.0f;
                float dy = screenPos.y - (float)monitorHeight / 2.0f;
                float distance = sqrtf((dx * dx) + (dy * dy));

                if (distance < closestDist) {
                    bool visible = true;
                    if (canVisCheck) {
                        if (!vCheck.IsPointVisible(partPos, localEyePos)) visible = false;
                    }
                    if (visible) {
                        closestDist = distance;
                        bestPartPos = partPos;
                        bestScreenPos = screenPos;
                        found = true;
                    }
                }
            }

            if (found) {
                if (aim.aimbot.key.active) {
                    if ((bestScreenPos.x != 0.0f && bestScreenPos.y != 0.0f) && (bestPartPos.x != 0.0f || bestPartPos.y != 0.0f || bestPartPos.z != 0.0f)) {
                        POINT mousePos;
                        GetCursorPos(&mousePos);

                        float dx = bestScreenPos.x - mousePos.x;
                        float dy = bestScreenPos.y - mousePos.y;

                        static float accX{ 0.0f };
                        static float accY{ 0.0f };

                        accX += dx / aim.aimbot.smoothing;
                        accY += dy / aim.aimbot.smoothing;

                        int moveX{ (int)accX };
                        int moveY{ (int)accY };

                        accX -= moveX;
                        accY -= moveY;

                        NtUserInjectMouseInput::InjectMouseMoveRelative(moveX, moveY);
                    }
                }

                if (aim.aimbot.visualise_target) {
                    drawList->AddCircleFilled({ bestScreenPos.x, bestScreenPos.y }, 5.0f, IM_COL32(255, 0, 0, 255));
                }

                if (aim.aimbot.target_tracer) {
                    drawList->AddLine({ (float)monitorWidth / 2.0f, (float)monitorHeight / 2.0f }, { bestScreenPos.x, bestScreenPos.y }, IM_COL32(255, 0, 0, 255));
                }
            }
        }

        ULONGLONG rippleNow = GetTickCount64();
        for (size_t r = 0; r < activeRipples.size(); ) {
            SoundRipple& ripple = activeRipples[r];
            float progress = (float)(rippleNow - ripple.startTime) / ripple.duration;
            if (progress >= 1.0f) {
                activeRipples.erase(activeRipples.begin() + r);
                continue;
            }
            float easeOut = 1.0f - powf(1.0f - progress, 3.0f);
            float radius = ripple.maxRadius * easeOut;
            float alpha = 1.0f - progress;
            ImU32 color = (ripple.color & 0x00FFFFFF) | ((ImU32)(alpha * 255.0f) << IM_COL32_A_SHIFT);
            ImU32 softColor = (ripple.color & 0x00FFFFFF) | ((ImU32)(alpha * 128.0f) << IM_COL32_A_SHIFT);
            const int segments = 80;
            static std::vector<ImVec2> points;
            static std::vector<ImVec2> softPoints;
            points.clear();
            softPoints.clear();
            points.reserve(segments + 1);
            softPoints.reserve(segments + 1);
            bool closed = true;
            for (int s = 0; s <= segments; s++) {
                float angle = (Math::TWO_PI * s) / segments;
                Vector3 worldPos = {
                    ripple.position.x + cosf(angle) * radius,
                    ripple.position.y + sinf(angle) * radius,
                    ripple.position.z - 1.5f
                };
                Vector2 screen = WorldToScreen(worldPos, viewMatrix);
                if (screen.x == 0.0f && screen.y == 0.0f) { closed = false; continue; }
                points.push_back({ screen.x, screen.y });
                softPoints.push_back({ screen.x + 0.5f, screen.y + 0.5f });
            }
            if (points.size() >= 2) {
                drawList->AddPolyline(points.data(), (int)points.size(), color, closed, 1.5f);
                drawList->AddPolyline(softPoints.data(), (int)softPoints.size(), softColor, closed, 1.5f);
            }
            r++;
        }

        if (misc.spectator_list) DrawSpectatorList(spectatingPlrs);

        float serverTime = 0.0f;
        uintptr_t globalVars = cs2.read<uintptr_t>(client + Offsets::dwGlobalVars);
        if (globalVars) {
            serverTime = cs2.read<float>(globalVars + 0x30);
        }

        if (world.master.enabled && world.masterKey.active && isForeground && (world.dropped_weapons.enabled || world.dropped_projectiles.enabled)) {
            static std::unordered_map<uintptr_t, ULONGLONG> projectileSeenTime;
            std::vector<uintptr_t> currentProjectiles;
            auto ResolveEntityFromHandle = [&](int handle) -> uintptr_t {
                if (handle <= 0) {
                    return 0;
                }
                uintptr_t listEntry{ cs2.read<uintptr_t>(entityList + 8 * ((handle & 0x7FFF) >> 9) + 16) };
                if (!listEntry) {
                    return 0;
                }
                return cs2.read<uintptr_t>(listEntry + 112 * (handle & 0x1FF));
            };
            for (int i{ 64 }; i < 2000; i++) {
                uintptr_t itemlistEntry{ cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16) };
                if (!itemlistEntry) {
                    continue;
                }

                uintptr_t itemEntity{ cs2.read<uintptr_t>(itemlistEntry + 112 * (i & 0x1FF)) };
                if (!itemEntity) {
                    continue;
                }

                int ownerHandle = 0;
                if (Offsets::m_hOwnerEntity) {
                    ownerHandle = cs2.read<int>(itemEntity + Offsets::m_hOwnerEntity);
                }
                bool ownerIsPlayer = false;
                Vector3 ownerPos{};
                bool ownerPosValid = false;
                if (ownerHandle > 0) {
                    uintptr_t ownerEntity = ResolveEntityFromHandle(ownerHandle);
                    if (ownerEntity) {
                        int ownerHealth = cs2.read<int>(ownerEntity + Offsets::m_iHealth);
                        int ownerTeam = cs2.read<int>(ownerEntity + Offsets::m_iTeamNum);
                        if (ownerHealth > 0 && ownerTeam >= 1 && ownerTeam <= 3) {
                            ownerIsPlayer = true;
                        }
                        if (Offsets::m_pGameSceneNode && Offsets::m_vecAbsOrigin) {
                            uintptr_t ownerNode = cs2.read<uintptr_t>(ownerEntity + Offsets::m_pGameSceneNode);
                            if (ownerNode) {
                                Vector3 pos = cs2.read<Vector3>(ownerNode + Offsets::m_vecAbsOrigin);
                                if (!(pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f)) {
                                    ownerPos = pos;
                                    ownerPosValid = true;
                                }
                            }
                        }
                    }
                }

                uintptr_t itemNode{ cs2.read<uintptr_t>(itemEntity + Offsets::m_pGameSceneNode) };
                bool hasNode = itemNode != 0;
                Vector3 itemPos{};
                if (hasNode) {
                    itemPos = cs2.read<Vector3>(itemNode + Offsets::m_vecAbsOrigin);
                } else {
                    bool hasPos = false;
                    if (Offsets::m_vOldOrigin) {
                        Vector3 fallback = cs2.read<Vector3>(itemEntity + Offsets::m_vOldOrigin);
                        if (!(fallback.x == 0.0f && fallback.y == 0.0f && fallback.z == 0.0f)) {
                            itemPos = fallback;
                            hasPos = true;
                        }
                    }
                    if (!hasPos) {
                        continue;
                    }
                }
                uintptr_t itemInfo{ cs2.read<uintptr_t>(itemEntity + 0x10) };
                std::string type;
                if (itemInfo) {
                    uintptr_t itemTypePtr{ cs2.read<uintptr_t>(itemInfo + 0x20) };
                    if (itemTypePtr) {
                        type = cs2.readstr(itemTypePtr);
                    }
                    if (type.empty()) {
                        type = cs2.readstr(itemInfo + 0x20);
                    }
                    if (type.empty()) {
                        type = cs2.readstr(itemInfo + 0x18);
                    }
                }
                if (type.empty()) {
                    type = cs2.readstr(itemEntity + 0x10);
                }
                if (type.empty()) {
                    type = cs2.readstr(itemEntity + 0x20);
                }
                if (type.empty()) {
                    type = cs2.readstr(itemEntity + 0x18);
                }

                std::string typeLower = type;
                std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

                std::string weapon;
                if (world.dropped_weapons.enabled) weapon = getWeaponType(typeLower);

                int projType = -1;
                bool isWeaponClass = typeLower.rfind("weapon_", 0) == 0;
                if (typeLower.find("smokegrenade") != std::string::npos) projType = 0;
                else if (typeLower.find("flashbang") != std::string::npos) projType = 1;
                else if (typeLower.find("hegrenade") != std::string::npos) projType = 2;
                else if (typeLower.find("molotov") != std::string::npos || typeLower.find("incendiarygrenade") != std::string::npos || typeLower.find("incgrenade") != std::string::npos || typeLower.find("inferno") != std::string::npos) projType = 3;
                else if (typeLower.find("decoy") != std::string::npos) projType = 4;
                bool isWeapon = weapon != "unknown" && !ownerIsPlayer;
                bool isProjectile = world.dropped_projectiles.enabled && projType >= 0;
                if (isProjectile && ownerIsPlayer && isWeaponClass && ownerPosValid) {
                    float odx = ownerPos.x - itemPos.x;
                    float ody = ownerPos.y - itemPos.y;
                    float odz = ownerPos.z - itemPos.z;
                    float ownerDistSq = (odx * odx) + (ody * ody) + (odz * odz);
                    if (ownerDistSq < (80.0f * 80.0f)) {
                        isProjectile = false;
                    }
                }
                if (!isWeapon && !isProjectile) {
                    continue;
                }
                bool canDrawInfo = false;
                Vector2 sP{};
                if (hasNode) {
                    sP = WorldToScreen(itemPos, viewMatrix);
                    canDrawInfo = !(sP.x == 0.0f && sP.y == 0.0f);
                }
                if (canDrawInfo) {
                    ImFont* worldTextFont = gamesenseFont;
                    const float worldTextSize = 10.0f;
                    float y = sP.y;
                    if (isWeapon && world.dropped_weapons_icon) {
                        std::string iconKey = typeLower;
                        if (iconKey.rfind("weapon_", 0) == 0) iconKey = iconKey.substr(7);
                        if (iconKey == "m4a1_silencer") iconKey = "m4a1";
                        if (iconKey == "usp_silencer") iconKey = "usp";
                        if (iconKey == "hkp2000") iconKey = "p2000";
                        const char* icon = getWeaponIcon(iconKey);
                        if (icon && icon[0] != '\0') {
                            ImFont* iconFont = imguiIo.Fonts->Fonts[1];
                            ImVec2 iconSz = iconFont->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, icon);
                            WeaponIconSize iconConfig{ 13.0f, 13.0f, 0.0f, 0.0f };
                            auto iconIt = weaponIconSizes.find(iconKey);
                            if (iconIt != weaponIconSizes.end()) iconConfig = iconIt->second;
                            ImVec2 iconPos = { floorf(sP.x - (iconSz.x * 0.5f)), floorf(y + iconConfig.offsetY) };
                            ImU32 iconColor = ColorToU32(world.dropped_weapons_icon_color);
                            drawList->AddText(iconFont, 13.0f, { iconPos.x + 1, iconPos.y + 1 }, IM_COL32(0, 0, 0, 255), icon);
                            drawList->AddText(iconFont, 13.0f, iconPos, iconColor, icon);
                            y += iconSz.y + 1.0f;
                        }
                    }
                    if (isWeapon && world.dropped_weapons_name) {
                        DrawSkeetText(drawList, worldTextFont, worldTextSize, { sP.x, y }, ColorToU32(world.dropped_weapons_name_color), weapon.c_str(), true);
                        ImVec2 wSize = worldTextFont->CalcTextSizeA(worldTextSize, FLT_MAX, 0.0f, weapon.c_str());
                        y += wSize.y + 1.0f;
                    }
                    if (isWeapon && world.dropped_weapons_ammo) {
                        int currentAmmo = cs2.read<int>(itemEntity + Offsets::m_iClip1);
                        int maxAmmo = GetMaxClip(typeLower);
                        if (maxAmmo > 0 && currentAmmo >= 0) {
                            float ammoPerc = ClampFloat((float)currentAmmo / (float)maxAmmo, 0.0f, 1.0f);
                            float barWidth = 32.0f;
                            float barHeight = 2.0f;
                            float left = sP.x - (barWidth * 0.5f);
                            float right = sP.x + (barWidth * 0.5f);
                            drawList->AddRectFilled({ left - 1.0f, y - 1.0f }, { right + 1.0f, y + barHeight + 1.0f }, IM_COL32(0, 0, 0, 150));
                            float fill = barWidth * ammoPerc;
                            drawList->AddRectFilled({ left, y }, { left + fill, y + barHeight }, ColorToU32(world.dropped_weapons_ammo_color));
                            y += barHeight + 3.0f;
                        }
                    }
                    if (isWeapon && world.dropped_weapons_distance) {
                        float dx = localPlayerEntity.position.x - itemPos.x;
                        float dy = localPlayerEntity.position.y - itemPos.y;
                        float dz = localPlayerEntity.position.z - itemPos.z;
                        float distMeters = sqrtf((dx * dx) + (dy * dy) + (dz * dz)) * 0.0254f;
                        char distBuffer[32];
                        sprintf_s(distBuffer, "%.1fm", distMeters);
                        DrawSkeetText(drawList, worldTextFont, worldTextSize, { sP.x, y }, ColorToU32(world.dropped_weapons_distance_color), distBuffer, true);
                        ImVec2 dSize = worldTextFont->CalcTextSizeA(worldTextSize, FLT_MAX, 0.0f, distBuffer);
                        y += dSize.y + 1.0f;
                    }

                    if (isProjectile) {
                        ProjectileTypeSettings& projSettings = world.dropped_projectiles_types[projType];
                        const char* projLabels[] = { "Smoke", "Flash", "HE Grenade", "Molotov", "Decoy" };
                        const char* projectileName = projLabels[projType];

                        if (projSettings.text) {
                            DrawSkeetText(drawList, worldTextFont, worldTextSize, { sP.x, y }, ColorToU32(world.dropped_projectiles_name_color), projectileName, true);
                            ImVec2 pSize = worldTextFont->CalcTextSizeA(worldTextSize, FLT_MAX, 0.0f, projectileName);
                            y += pSize.y + 1.0f;
                        }

                        if (projSettings.icon) {
                            std::string iconKey;
                        if (projType == 0) iconKey = "smokegrenade";
                        else if (projType == 1) iconKey = "flashbang";
                        else if (projType == 2) iconKey = "hegrenade";
                        else if (projType == 3) {
                            if (typeLower.find("incendiarygrenade") != std::string::npos || typeLower.find("incgrenade") != std::string::npos) iconKey = "incgrenade";
                            else iconKey = "molotov";
                        }
                        else if (projType == 4) iconKey = "decoy";

                        if (!iconKey.empty()) {
                            const char* icon = getWeaponIcon(iconKey);
                            if (icon && icon[0] != '\0') {
                                ImFont* iconFont = imguiIo.Fonts->Fonts[1];
                                ImVec2 iconSz = iconFont->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, icon);
                                WeaponIconSize iconConfig{ 13.0f, 13.0f, 0.0f, 0.0f };
                                auto iconIt = weaponIconSizes.find(iconKey);
                                if (iconIt != weaponIconSizes.end()) iconConfig = iconIt->second;
                                ImVec2 iconPos = { floorf(sP.x - (iconSz.x * 0.5f)), floorf(y + iconConfig.offsetY) };
                                ImU32 iconColor = ColorToU32(world.dropped_projectiles_icon_color);
                                drawList->AddText(iconFont, 13.0f, { iconPos.x + 1, iconPos.y + 1 }, IM_COL32(0, 0, 0, 255), icon);
                                drawList->AddText(iconFont, 13.0f, iconPos, iconColor, icon);
                                y += iconSz.y + 1.0f;
                            }
                        }
                    }

                    bool wantsTimerData = projSettings.timer || projSettings.timer_bar;
                    if (wantsTimerData) {
                        currentProjectiles.push_back(itemEntity);
                        float remain = -1.0f;
                        float duration = 1.7f;
                        if (projType == 0) duration = 18.0f;
                        else if (projType == 3) duration = 7.0f;
                        else if (projType == 4) duration = 15.0f;
                        bool useSmokeLinger = false;

                        if (Offsets::m_flDetonateTime && serverTime > 0.0f) {
                            float detonateTime = cs2.read<float>(itemEntity + Offsets::m_flDetonateTime);
                            if (detonateTime > 0.0f) {
                                remain = detonateTime - serverTime;
                            }
                        }
                        if (remain < 0.0f && Offsets::m_flSmokeEffectTickBegin && Offsets::m_flSmokeEffectTickEnd && serverTime > 0.0f) {
                            float smokeBegin = cs2.read<float>(itemEntity + Offsets::m_flSmokeEffectTickBegin);
                            float smokeEnd = cs2.read<float>(itemEntity + Offsets::m_flSmokeEffectTickEnd);
                            if (smokeEnd > smokeBegin && smokeBegin > 0.0f) {
                                remain = smokeEnd - serverTime;
                                duration = smokeEnd - smokeBegin;
                                if (projType == 0) useSmokeLinger = true;
                            }
                        }
                        if (remain < 0.0f) {
                            if (!projectileSeenTime.count(itemEntity)) {
                                projectileSeenTime[itemEntity] = GetTickCount64();
                            }
                            ULONGLONG start = projectileSeenTime[itemEntity];
                            float elapsed = (float)(GetTickCount64() - start) / 1000.0f;
                            remain = duration - elapsed;
                            if (projType == 0) useSmokeLinger = true;
                        }
                        if (useSmokeLinger) {
                            float linger = 2.0f;
                            duration += linger;
                            remain += linger;
                        }
                        if (remain < 0.0f) remain = 0.0f;

                        if (projSettings.timer) {
                            char timerBuffer[32];
                            sprintf_s(timerBuffer, "%.1fs", remain);
                            DrawSkeetText(drawList, worldTextFont, worldTextSize, { sP.x, y }, ColorToU32(world.dropped_projectiles_timer_color), timerBuffer, true);
                            ImVec2 tSize = worldTextFont->CalcTextSizeA(worldTextSize, FLT_MAX, 0.0f, timerBuffer);
                            y += tSize.y + 1.0f;
                        }
                        if (projSettings.timer_bar) {
                            float barWidth = 32.0f;
                            float barHeight = 2.0f;
                            float left = sP.x - (barWidth * 0.5f);
                            float right = sP.x + (barWidth * 0.5f);
                            float perc = (duration > 0.0f) ? ClampFloat(remain / duration, 0.0f, 1.0f) : 0.0f;
                            drawList->AddRectFilled({ left - 1.0f, y - 1.0f }, { right + 1.0f, y + barHeight + 1.0f }, IM_COL32(0, 0, 0, 150));
                            drawList->AddRectFilled({ left, y }, { left + (barWidth * perc), y + barHeight }, ColorToU32(world.dropped_projectiles_timer_color));
                            y += barHeight + 3.0f;
                        }
                    }

                    if (projSettings.distance) {
                        float dx = localPlayerEntity.position.x - itemPos.x;
                        float dy = localPlayerEntity.position.y - itemPos.y;
                        float dz = localPlayerEntity.position.z - itemPos.z;
                        float distMeters = sqrtf((dx * dx) + (dy * dy) + (dz * dz)) * 0.0254f;
                        char distBuffer[32];
                        sprintf_s(distBuffer, "%.1fm", distMeters);
                        DrawSkeetText(drawList, worldTextFont, worldTextSize, { sP.x, y }, ColorToU32(world.dropped_projectiles_distance_color), distBuffer, true);
                        ImVec2 dSize = worldTextFont->CalcTextSizeA(worldTextSize, FLT_MAX, 0.0f, distBuffer);
                        y += dSize.y + 1.0f;
                    }
                }
                }

            }

            ULONGLONG cleanupNow = GetTickCount64();
            if (!projectileSeenTime.empty()) {
                for (auto it = projectileSeenTime.begin(); it != projectileSeenTime.end(); ) {
                    if (std::find(currentProjectiles.begin(), currentProjectiles.end(), it->first) == currentProjectiles.end()) {
                        it = projectileSeenTime.erase(it);
                    }
                    else {
                        ++it;
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
                        ApplyHitsoundVolume(misc.hitsound_volume);
                        std::wstring wpath(currentHitsoundPath.begin(), currentHitsoundPath.end());
                        PlaySoundW(wpath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
                    }
                }
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
        Sleep(0);
    }
    renderer.Shutdown();
    return 0;
}
