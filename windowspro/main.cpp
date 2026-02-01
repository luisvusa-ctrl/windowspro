#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#pragma comment(lib, "winmm.lib")

#include "inc/Renderer.hpp"
#include "imgui/ImAnimate/ImAnimate.h"
#include "imgui/ImToggle/imgui_toggle.h"
#include "inc/VisCheck.h"
#include "inc/mem.hpp"
#include "inc/utils.hpp"
#include "inc/input.hpp"
#include "inc/offsets.hpp"

#include "inc/firaCode.hpp"
#include "inc/weaponFont.hpp"

ImFont* espNameFont = nullptr;
ImFont* tahomaBoldFont = nullptr;
ImFont* smallFont = nullptr;


namespace FlagOffsets {

    constexpr ptrdiff_t m_pInGameMoneyServices = 0x808;
    constexpr ptrdiff_t m_iAccount = 0x40;
    constexpr ptrdiff_t m_vecAbsVelocity = 0x404;

    // Pawn
    constexpr ptrdiff_t m_pItemServices = 0x13E0;
    constexpr ptrdiff_t m_bHasDefuser = 0x48;
    constexpr ptrdiff_t m_bIsScoped = 0x26F8;
    constexpr ptrdiff_t m_bHasHelmet = 0x49;
    constexpr ptrdiff_t m_ArmorValue = 0x272C;
    constexpr ptrdiff_t m_flFlashDuration = 0x15F8;


    // Inventory / Weapon
    constexpr ptrdiff_t m_pWeaponServices = 0x13D8;
    constexpr ptrdiff_t m_hMyWeapons = 0x48;

    constexpr ptrdiff_t m_AttributeManager = 0x13B8;
    constexpr ptrdiff_t m_Item = 0x50;
    constexpr ptrdiff_t m_iItemDefinitionIndex = 0x1BA;
}

// Features Namespace
namespace Globals
{
    bool imguiVisible{ false };
    bool streamproofEnabled{ false };
    bool spectatorListEnabled{ false };
}

// --- ESTRUTURAS DE SOM (RIPPLES) ---
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

// --- UI STRUCTS ---
struct ConfigItem
{
    bool enabled;
    bool active;
    float color[4];

    ConfigItem() {
        enabled = false;
        active = false;
        for (int i = 0; i < 4; i++) color[i] = 1.0f;
    }
};

struct WeaponSettings
{
    bool enabled = false;
    bool text_enabled = false; float text_color[4] = { 1, 1, 1, 1 };
    bool icon_enabled = false; float icon_color[4] = { 1, 1, 1, 1 };
};

struct SoundSettings
{
    bool enabled = false;
    float color[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
};

struct HealthSettings
{
    bool enabled = false;
    int mode = 1;
    float static_color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float gradient_low[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    float gradient_high[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
};

struct ArmorSettings
{
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

struct ESPGroup
{
    ConfigItem master, box, name, skeleton, snaplines, ammoBar;
    SoundSettings sound;
    WeaponSettings weapon;
    HealthSettings health;
    ArmorSettings armor;

    // Flags---------------------------------------------------------------------------------------------------------------------------------------------------
    ConfigItem flagsMaster;
    FlagItem flagMoney{ 0.45f, 0.7f, 0.2f };
    FlagItem flagArmor{ 1.0f, 1.0f, 1.0f };
    FlagItem flagZoom{ 0.0f, 0.7f, 1.0f };
    FlagItem flagKit{ 0.2f, 0.8f, 0.2f };
    FlagItem flagBlind{ 1.0f, 0.8f, 0.0f };
    FlagItem flagC4{ 1.0f, 0.0f, 0.0f };

    ESPGroup()
    {
        master.enabled = true;
        box.color[0] = 1.f; box.color[1] = 0.f; box.color[2] = 0.f; box.color[3] = 1.f;
        skeleton.color[0] = 1.f; skeleton.color[1] = 1.f; skeleton.color[2] = 1.f; skeleton.color[3] = 1.f;
        snaplines.color[0] = 1.f; snaplines.color[1] = 0.f; snaplines.color[2] = 1.f; snaplines.color[3] = 1.f;
        ammoBar.color[0] = 1.f; ammoBar.color[1] = 0.8f; ammoBar.color[2] = 0.f; ammoBar.color[3] = 1.f;


        weapon.text_enabled = true;
        weapon.icon_enabled = false;
    }
};

struct WorldSettings
{
    ConfigItem master;
    bool spread_enabled = false; float spread_col[4] = { 1, 1, 1, 1 };
    bool recoil_cross_enabled = false; float recoil_col[4] = { 1, 1, 1, 1 };
    bool hitmarker_enabled = false;
    bool dropped_master = false;
    bool grenade_radius = false;
    bool grenade_trajectory = false; float traj_col[4] = { 1, 1, 1, 1 };
    bool radar_enabled = false;
};

// Globals Instances---------------------------------------------------------------------------------------------------------------------------------------------------
static ESPGroup enemy, team;
static WorldSettings world;

// Helpers---------------------------------------------------------------------------------------------------------------------------------------------------
ImU32 ColorToU32(float color[4])
{
    return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3]));
}

ImU32 GetGradientColor(float percentage, float lowColor[4], float highColor[4])
{
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 1.0f) percentage = 1.0f;

    float r = lowColor[0] + (highColor[0] - lowColor[0]) * percentage;
    float g = lowColor[1] + (highColor[1] - lowColor[1]) * percentage;
    float b = lowColor[2] + (highColor[2] - lowColor[2]) * percentage;
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
}

bool HasC4InInventory(Memory& mem, uintptr_t pawnAddress, uintptr_t entityList)
{
    uintptr_t weaponServices = mem.read<uintptr_t>(pawnAddress + FlagOffsets::m_pWeaponServices);
    if (!weaponServices) return false;

    for (int i = 0; i < 32; i++) {
        int weaponHandle = mem.read<int>(weaponServices + FlagOffsets::m_hMyWeapons + (i * 4));
        if (weaponHandle == -1) continue;

        uintptr_t listEntry = mem.read<uintptr_t>(entityList + 8 * ((weaponHandle & 0x7FFF) >> 9) + 16);
        if (!listEntry) continue;

        uintptr_t weaponEntity = mem.read<uintptr_t>(listEntry + 112 * (weaponHandle & 0x1FF));
        if (!weaponEntity) continue;

        uintptr_t attributeManager = mem.read<uintptr_t>(weaponEntity + FlagOffsets::m_AttributeManager);
        if (!attributeManager) continue;

        uintptr_t item = mem.read<uintptr_t>(attributeManager + FlagOffsets::m_Item);
        if (!item) continue;

        short defIndex = mem.read<short>(item + FlagOffsets::m_iItemDefinitionIndex);

        if (defIndex == 49) return true;
    }
    return false;
}

// UI Helpers---------------------------------------------------------------------------------------------------------------------------------------------------
void DrawConfigItem(const char* label, ConfigItem& item)
{
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
    std::string id = std::string("##cp_") + label;
    ImGui::ColorEdit4(id.c_str(), item.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
}

void DrawFlagOption(const char* label, FlagItem& item)
{
    ImGui::Checkbox(label, &item.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
    std::string id = std::string("##flg_cp_") + label;
    ImGui::ColorEdit4(id.c_str(), item.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
}

// Enemy and Team ESP UI---------------------------------------------------------------------------------------------------------------------------------------------------
void DrawESPColumn(const char* title, ESPGroup& group)
{
    ImGui::BeginChild(title, ImVec2(0, 0), true);
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), title);
    ImGui::Separator();

    ImGui::Checkbox("Master Switch", &group.master.enabled);
    ImGui::Separator();

    DrawConfigItem("Bounding Box", group.box);
    DrawConfigItem("Name", group.name);
    DrawConfigItem("Skeleton", group.skeleton);
    DrawConfigItem("Snaplines", group.snaplines);
    DrawConfigItem("Ammo Bar", group.ammoBar);

    // --- WEAPON ------------------------------------------------------------------------------------------------------------------------------------------------------
    ImGui::Checkbox("Weapon", &group.weapon.enabled);
    if (group.weapon.enabled)
    {
        ImGui::Indent(15.0f);
        ImGui::Checkbox("Text", &group.weapon.text_enabled);
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
        ImGui::ColorEdit4("##wpntxtcol", group.weapon.text_color, ImGuiColorEditFlags_NoInputs);

        ImGui::Checkbox("Icon", &group.weapon.icon_enabled);
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
        ImGui::ColorEdit4("##wpniconcol", group.weapon.icon_color, ImGuiColorEditFlags_NoInputs);
        ImGui::Unindent(15.0f);
    }

    // --- SOUNDS ------------------------------------------------------------------------------------------------------------------------------------------------------
    ImGui::Checkbox("Sounds (Steps)", &group.sound.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
    ImGui::ColorEdit4("##sndcol", group.sound.color, ImGuiColorEditFlags_NoInputs);

    // --- ARMOR ------------------------------------------------------------------------------------------------------------------------------------------------------
    ImGui::Checkbox("Armor Bar", &group.armor.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
    ImGui::ColorEdit4("##armorcol", group.armor.static_color, ImGuiColorEditFlags_NoInputs);

    // --- HEALTH ------------------------------------------------------------------------------------------------------------------------------------------------------
    ImGui::Checkbox("Health Bar", &group.health.enabled);
    if (group.health.enabled)
    {
        ImGui::Indent(15.0f);
        const char* modes[] = { "Static", "Gradient" };
        ImGui::SetNextItemWidth(100);
        ImGui::Combo("##hpMode", &group.health.mode, modes, IM_ARRAYSIZE(modes));
        ImGui::SameLine();

        if (group.health.mode == 0)
        {
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
            ImGui::ColorEdit4("##hpstatic", group.health.static_color, ImGuiColorEditFlags_NoInputs);
        }
        else
        {
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 80);
            ImGui::ColorEdit4("##hplow", group.health.gradient_low, ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine();
            ImGui::ColorEdit4("##hphigh", group.health.gradient_high, ImGuiColorEditFlags_NoInputs);
        }
        ImGui::Unindent(15.0f);
    }

    // --- FLAGS --- ---------------------------------------------------------------------------------------------------------------------------------------------------
    ImGui::Checkbox("Flags", &group.flagsMaster.enabled);
    if (group.flagsMaster.enabled)
    {
        ImGui::Indent(15.0f);
        DrawFlagOption("MONEY", group.flagMoney);
        DrawFlagOption("HK (Armor)", group.flagArmor);
        DrawFlagOption("ZOOM", group.flagZoom);
        DrawFlagOption("KIT", group.flagKit);
        DrawFlagOption("C4", group.flagC4);
        DrawFlagOption("BLIND", group.flagBlind);
        ImGui::Unindent(15.0f);
    }

    ImGui::EndChild();
}

// World ESP UI ---------------------------------------------------------------------------------------------------------------------------------------------------
void DrawWorldColumn()
{
    ImGui::BeginChild("World ESP", ImVec2(0, 0), true);
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "World ESP");
    ImGui::Separator();

    ImGui::Checkbox("Master Switch", &world.master.enabled);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Spread", &world.spread_enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
    ImGui::ColorEdit4("##spread", world.spread_col, ImGuiColorEditFlags_NoInputs);

    ImGui::Checkbox("Recoil Crosshair", &world.recoil_cross_enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
    ImGui::ColorEdit4("##recoil", world.recoil_col, ImGuiColorEditFlags_NoInputs);

    ImGui::Checkbox("Hitmarker", &world.hitmarker_enabled);
    ImGui::Checkbox("Dropped Weapons", &world.dropped_master);
    ImGui::Checkbox("Grenade Radius", &world.grenade_radius);

    ImGui::Checkbox("Grenade Trajectory", &world.grenade_trajectory);
    if (world.grenade_trajectory) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
        ImGui::ColorEdit4("##traj", world.traj_col, ImGuiColorEditFlags_NoInputs);
    }

    ImGui::Checkbox("Radar", &world.radar_enabled);

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 100);
    if (ImGui::Button("SAVE CONFIG", ImVec2(240, 35))) { /* Save */ }
    ImGui::Spacing();
    if (ImGui::Button("LOAD CONFIG", ImVec2(240, 35))) { /* Load */ }

    ImGui::EndChild();
}

int GetMaxClip(std::string name)
{
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

// Main Loader ---------------------------------------------------------------------------------------------------------------------------------------------------
int main()
{
    DWORD cs2PID{ getPID(L"cs2.exe") };

    if (cs2PID == 0) {
        std::cout << "Failed to find cs2 process.\n";
        system("pause"); return 1;
    }

    int offsetsChoice{ 0 };
    while (true) {
        std::cout << "1. Get offsets online.\n2. Dump offsets.\n> ";
        std::cin >> offsetsChoice;
        if (offsetsChoice == 1 || offsetsChoice == 2) break;
    }

    system("cls");
    std::cout << "Getting offsets...\n";
    if (offsetsChoice == 1) { if (Offsets::getOffsetsOnline()) system("cls"); else return 1; }
    else { if (Offsets::dumpOffsets()) system("cls"); else return 1; }

    Memory cs2{ cs2PID };
    uintptr_t client{ getModuleBase(cs2PID, L"client.dll") };

    int monitorWidth{ GetSystemMetrics(SM_CXSCREEN) };
    int monitorHeight{ GetSystemMetrics(SM_CYSCREEN) };

    Renderer renderer;
    renderer.Init();

    VisCheck vCheck{ "" };
    NtUserInjectMouseInput::InitInjectMouseInput();
    ImGuiIO& imguiIo{ ImGui::GetIO() };

    // FONTS  ---------------------------------------------------------------------------------------------------------------------------------------------------

    imguiIo.Fonts->AddFontDefault();
    imguiIo.Fonts->AddFontFromMemoryTTF(weaponFont, weaponFontSize, 20.0f);
    imguiIo.Fonts->AddFontFromMemoryTTF(firaCodeFont, firaCodeFontSize, 16.0f);


    ImFont* gamesenseFont = imguiIo.Fonts->AddFontFromFileTTF("smallest_pixel-7.ttf", 11.0f);


    tahomaBoldFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 12.0f);
    if (!tahomaBoldFont) {
        tahomaBoldFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 12.0f);
        if (!tahomaBoldFont) tahomaBoldFont = ImGui::GetFont();
    }

    smallFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 10.0f);
    if (!smallFont) smallFont = tahomaBoldFont;

    // IMGUI ---------------------------------------------------------------------------------------------------------------------------------------------------

    while (true)
    {
        renderer.StartRender();

        if (GetAsyncKeyState(VK_INSERT) & 1) Globals::imguiVisible = !Globals::imguiVisible;

        if (Globals::imguiVisible)
        {
            SetWindowLong(renderer.hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
            ImGui::SetNextWindowSize(ImVec2(880, 580));
            ImGui::Begin("VORTIX MASTER", nullptr, ImGuiWindowFlags_NoDecoration);
            {
                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 120, 8));
                if (ImGui::Button("Save", ImVec2(110, 35))) {}
                if (ImGui::BeginTabBar("MainTabs")) {
                    if (ImGui::BeginTabItem("Visuals")) {
                        ImGui::Columns(3, nullptr, false);
                        DrawESPColumn("Enemy ESP", enemy); ImGui::NextColumn();
                        DrawESPColumn("Team ESP", team); ImGui::NextColumn();
                        DrawWorldColumn(); ImGui::Columns(1);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Misc")) {
                        ImGui::Text("Misc & Settings"); ImGui::Separator();
                        ImGui::Checkbox("Stream Proof", &Globals::streamproofEnabled);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            ImGui::End();
        }
        else {
            SetWindowLong(renderer.hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED);
        }

        ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };

        uintptr_t playerPawnAddress{ cs2.read<uintptr_t>(client + Offsets::dwLocalPlayerPawn) };
        Entity localPlayerEntity{ cs2, playerPawnAddress };
        uintptr_t entityList{ cs2.read<uintptr_t>(client + Offsets::dwEntityList) };
        Matrix viewMatrix{ cs2.read<Matrix>(client + Offsets::dwViewMatrix) };

        // SOUND ESP RIPPLES ---------------------------------------------------------------------------------------------------------------------------------------------------
        ULONGLONG currentTime = GetTickCount64();
        for (int i = activeRipples.size() - 1; i >= 0; i--) {
            float elapsed = (float)(currentTime - activeRipples[i].startTime);
            if (elapsed >= activeRipples[i].duration) {
                activeRipples.erase(activeRipples.begin() + i);
            }
            else {
                
                float progress = elapsed / activeRipples[i].duration;
                
                float easeOut = 1.0f - std::pow(1.0f - progress, 3.0f);
                float currentRadius = easeOut * activeRipples[i].maxRadius;

                
                ImVec4 colVec = ImGui::ColorConvertU32ToFloat4(activeRipples[i].color);
                colVec.w *= (1.0f - progress); 
                ImU32 finalColor = ImGui::ColorConvertFloat4ToU32(colVec);

                
                std::vector<ImVec2> points;
                for (int j = 0; j <= 40; j++) {
                    float theta = (2.0f * 3.14159f * j) / 40.0f;
                    Vector3 worldPos = {
                        activeRipples[i].position.x + currentRadius * std::cos(theta),
                        activeRipples[i].position.y + currentRadius * std::sin(theta),
                        activeRipples[i].position.z
                    };
                    Vector2 sc = WorldToScreen(worldPos, viewMatrix);
                    if (sc.x != 0) points.push_back({ sc.x, sc.y });
                }
                if (points.size() > 1) {
                    for (size_t k = 0; k < points.size() - 1; k++)
                        drawList->AddLine(points[k], points[k + 1], finalColor, 1.5f);
                }
            }
        }

        for (int i{ 0 }; i < 64; i++)
        {
            uintptr_t listEntry{ cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16) };
            if (!listEntry) continue;
            uintptr_t currentController{ cs2.read<uintptr_t>(listEntry + 112 * (i & 0x1FF)) };
            if (!currentController) continue;
            int pawnHandle{ cs2.read<int>(currentController + Offsets::m_hPlayerPawn) };
            if (pawnHandle == 0) continue;
            uintptr_t listEntry2{ cs2.read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16) };
            uintptr_t currentPawn{ cs2.read<uintptr_t>(listEntry2 + 112 * (pawnHandle & 0x1FF)) };

            Entity entity{ cs2, currentPawn, currentController };

            ESPGroup* currentConfig = (entity.team == localPlayerEntity.team) ? &team : &enemy;

            if (currentConfig->master.enabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
            {
                bool check{ true };
                if (currentPawn == playerPawnAddress) check = false;
                if (entity.health <= 0) check = false;
                if (entity.position.x == 0.0f && entity.position.y == 0.0f) check = false;

                if (check)
                {
                    Vector3 headPos = { entity.position.x, entity.position.y, entity.position.z + 72.0f };
                    Vector2 screenPos{ WorldToScreen(entity.position, viewMatrix) };
                    Vector2 headScreenPos{ WorldToScreen(headPos, viewMatrix) };

                    if ((screenPos.x != 0.0f && screenPos.y != 0.0f) && (headScreenPos.x != 0.0f && headScreenPos.y != 0.0f))
                    {
                        // SKELETON ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->skeleton.enabled) {
                            uintptr_t boneMatrix{ getBoneMatrix(cs2, currentPawn) };
                            ImU32 skelColor = ColorToU32(currentConfig->skeleton.color);
                            for (Vector2 connection : boneConnections) {
                                int b1{ (int)connection.x }, b2{ (int)connection.y };
                                Vector3 v1{ cs2.read<Vector3>(boneMatrix + b1 * 32) };
                                Vector3 v2{ cs2.read<Vector3>(boneMatrix + b2 * 32) };
                                Vector2 sv1{ WorldToScreen(v1, viewMatrix) }, sv2{ WorldToScreen(v2, viewMatrix) };
                                if (sv1.x != 0 && sv2.x != 0) drawList->AddLine({ sv1.x, sv1.y }, { sv2.x, sv2.y }, skelColor, 1.5f);
                            }
                        }

                        float boxHeight{ std::fabs(headScreenPos.y - screenPos.y) };
                        float boxWidth{ boxHeight * 0.6f };
                        Vector2 topLeft{ screenPos.x - boxWidth / 2, headScreenPos.y };
                        Vector2 topRight{ screenPos.x + boxWidth / 2 + 12.0f, headScreenPos.y };
                        Vector2 bottomRight{ screenPos.x + boxWidth / 2, screenPos.y };

                        // BOX ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->box.enabled) {
                            ImU32 boxColor = ColorToU32(currentConfig->box.color);
                            drawList->AddRect({ topLeft.x - 1, topLeft.y - 1 }, { bottomRight.x + 1, bottomRight.y + 1 }, IM_COL32(0, 0, 0, 255));
                            drawList->AddRect({ topLeft.x + 1, topLeft.y + 1 }, { bottomRight.x - 1, bottomRight.y - 1 }, IM_COL32(0, 0, 0, 255));
                            drawList->AddRect({ topLeft.x, topLeft.y }, { bottomRight.x, bottomRight.y }, boxColor);
                        }

                        // AMMO BAR ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->ammoBar.enabled) {
                            uintptr_t clippingWeapon = cs2.read<uintptr_t>(entity.currentPawn + Offsets::m_pClippingWeapon);
                            if (clippingWeapon != 0) {
                                int currentAmmo = cs2.read<int>(clippingWeapon + Offsets::m_iClip1);
                                int maxAmmo = GetMaxClip(entity.weaponName);
                                if (maxAmmo > 0) {
                                    float ammoPerc = (float)currentAmmo / (float)maxAmmo;
                                    if (ammoPerc > 1.f) ammoPerc = 1.f;
                                    float topY = bottomRight.y + 2.0f;
                                    float barWidth = (bottomRight.x - topLeft.x) * ammoPerc;
                                    ImU32 ammoColor = ColorToU32(currentConfig->ammoBar.color);
                                    drawList->AddRectFilled({ topLeft.x, topY }, { bottomRight.x, topY + 4.0f }, IM_COL32(0, 0, 0, 255));
                                    drawList->AddRectFilled({ topLeft.x, topY }, { topLeft.x + barWidth, topY + 4.0f }, ammoColor);
                                    drawList->AddRect({ topLeft.x, topY }, { bottomRight.x, topY + 4.0f }, IM_COL32(0, 0, 0, 255));
                                }
                            }
                        }

                        // ARMOR BAR ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->armor.enabled) {
                            float armorPerc = (float)entity.armor / 100.0f;
                            if (armorPerc > 1.f) armorPerc = 1.f;
                            float barLeftX = bottomRight.x + 2.0f;
                            float barHeight = bottomRight.y - topLeft.y + 2.0f;
                            float visualHeight = barHeight * armorPerc;
                            ImU32 armorColor = ColorToU32(currentConfig->armor.static_color);

                            drawList->AddRectFilled({ barLeftX, topLeft.y - 1.0f }, { barLeftX + 4.0f, bottomRight.y + 1.0f }, IM_COL32(0, 0, 0, 255));
                            if (entity.armor > 0)
                                drawList->AddRectFilled({ barLeftX, bottomRight.y - visualHeight + 1.0f }, { barLeftX + 4.0f, bottomRight.y + 1.0f }, armorColor);
                            drawList->AddRect({ barLeftX, topLeft.y - 1.0f }, { barLeftX + 4.0f, bottomRight.y + 1.0f }, IM_COL32(0, 0, 0, 255));
                        }


                        // SOUND ESP ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->sound.enabled) {
                            float dx = localPlayerEntity.position.x - entity.position.x;
                            float dy = localPlayerEntity.position.y - entity.position.y;
                            float dz = localPlayerEntity.position.z - entity.position.z;
                            float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

                            Vector3 velocity = cs2.read<Vector3>(currentPawn + FlagOffsets::m_vecAbsVelocity);
                            float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

                            if (distance < 1100.0f && speed > 138.0f) {

                                if (entityRunStartTime[i] == 0) {
                                    entityRunStartTime[i] = currentTime;
                                }

                                if (currentTime - entityRunStartTime[i] > 150) {
                                    bool canAdd = true;

                                    if (entitySoundCooldown.count(i)) {
                                        if (currentTime - entitySoundCooldown[i] < 450) {
                                            canAdd = false;
                                        }
                                    }

                                    if (canAdd) {
                                        entitySoundCooldown[i] = currentTime;

                                        SoundRipple r;
                                        r.position = entity.position;
                                        r.startTime = currentTime;
                                        r.color = ColorToU32(currentConfig->sound.color);

                                        activeRipples.push_back(r);
                                    }
                                }
                            }
                            else {
                                entityRunStartTime[i] = 0;
                            }
                        }

                        // WEAPON ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->weapon.enabled) {
                            std::string weaponIcon = getWeaponIcon(entity.weaponName);
                            WeaponIconSize iconSize = weaponIconSizes[entity.weaponName];

                            
                            float baseY = bottomRight.y + 2.0f;

                            
                            if (currentConfig->ammoBar.enabled) {
                                baseY += 5.0f;
                            }

                            
                            if (currentConfig->weapon.text_enabled) {
                                ImVec2 txtSz = gamesenseFont->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, entity.weaponName.c_str());
                                ImVec2 textPos{ screenPos.x - txtSz.x / 2, baseY };

                                ImU32 textColor = ColorToU32(currentConfig->weapon.text_color);
                                ImU32 outlineColor = IM_COL32(0, 0, 0, 255);

                                
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x + 1, textPos.y), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x - 1, textPos.y), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x, textPos.y + 1), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x, textPos.y - 1), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x + 1, textPos.y + 1), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x - 1, textPos.y - 1), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x + 1, textPos.y - 1), outlineColor, entity.weaponName.c_str());
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(textPos.x - 1, textPos.y + 1), outlineColor, entity.weaponName.c_str());

                                
                                drawList->AddText(gamesenseFont, 10.0f, textPos, textColor, entity.weaponName.c_str());

                               
                                baseY += 8.0f;
                            }

                            if (currentConfig->weapon.icon_enabled) {
                                // centro da box
                                float centerX = (topLeft.x + bottomRight.x) * 0.5f;

                                // calcula tamanho do ícone
                                ImVec2 iconSizeCalc = imguiIo.Fonts->Fonts[1]->CalcTextSizeA(
                                    15.0f, FLT_MAX, 0.0f, weaponIcon.c_str()
                                );

                                ImVec2 iconPos{
                                    centerX - iconSizeCalc.x * 0.5f,   // centraliza horizontalmente
                                    baseY + iconSize.offsetY           // posição vertical
                                };

                                ImU32 iconColor = ColorToU32(currentConfig->weapon.icon_color);

                                drawList->AddText(imguiIo.Fonts->Fonts[1], 15.0f, iconPos, iconColor, weaponIcon.c_str());
                            }



                        }


                        // HEALTH BAR  ---------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->health.enabled) {
                            float healthPerc = (float)entity.health / 100.0f;
                            float barLeftX = topLeft.x - 6.0f;
                            float barHeight = bottomRight.y - topLeft.y + 2.0f;
                            float visualHeight = barHeight * healthPerc;
                            ImU32 healthCol = (currentConfig->health.mode == 0) ?
                                ColorToU32(currentConfig->health.static_color) :
                                GetGradientColor(healthPerc, currentConfig->health.gradient_low, currentConfig->health.gradient_high);

                            drawList->AddRectFilled({ barLeftX, topLeft.y - 1.0f }, { barLeftX + 4.0f, bottomRight.y + 1.0f }, IM_COL32(0, 0, 0, 255));
                            if (entity.health > 0)
                                drawList->AddRectFilled({ barLeftX, bottomRight.y - visualHeight + 1.0f }, { barLeftX + 4.0f, bottomRight.y + 1.0f }, healthCol);
                            drawList->AddRect({ barLeftX, topLeft.y - 1.0f }, { barLeftX + 4.0f, bottomRight.y + 1.0f }, IM_COL32(0, 0, 0, 255));
                        }

                        // SNAPLINES ----------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->snaplines.enabled) {
                            ImVec2 bottom = { (float)monitorWidth / 2, (float)monitorHeight };
                            drawList->AddLine(bottom, { screenPos.x, screenPos.y }, ColorToU32(currentConfig->snaplines.color), 1.2f);
                        }

                        // NAME ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->name.enabled) {
                            ImGui::PushFont(tahomaBoldFont);
                            ImVec2 textSize = ImGui::CalcTextSize(entity.name.c_str());
                            float centerX = topLeft.x + ((bottomRight.x - topLeft.x) / 2.0f) - (textSize.x / 2.0f);
                            drawList->AddText({ centerX + 1, topLeft.y - textSize.y - 1.0f }, IM_COL32(0, 0, 0, 255), entity.name.c_str());
                            drawList->AddText({ centerX, topLeft.y - textSize.y - 2.0f }, ColorToU32(currentConfig->name.color), entity.name.c_str());
                            ImGui::PopFont();
                        }

                        // FLAGS ---------------------------------------------------------------------------------------------------------------------------------------------------
                        if (currentConfig->flagsMaster.enabled)
                        {
                            
                            float flagX = bottomRight.x + 4.5f;   
                            float flagY = topLeft.y;             

                            
                            if (currentConfig->armor.enabled) {
                                flagX += 5.0f; 
                            }

                            ImGui::PushFont(gamesenseFont);

                            auto DrawFlag = [&](const char* txt, float* col) {
                                ImU32 textColor = ColorToU32(col);
                                ImU32 outlineColor = IM_COL32(0, 0, 0, 255);

                                
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX + 1, flagY), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX - 1, flagY), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX, flagY + 1), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX, flagY - 1), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX + 1, flagY + 1), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX - 1, flagY - 1), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX + 1, flagY - 1), outlineColor, txt);
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX - 1, flagY + 1), outlineColor, txt);

                                
                                drawList->AddText(gamesenseFont, 10.0f, ImVec2(flagX, flagY), textColor, txt);

                                flagY += 11.0f; 
                                };


                            if (currentConfig->flagMoney.enabled) {
                                uintptr_t moneyServices = cs2.read<uintptr_t>(currentController + FlagOffsets::m_pInGameMoneyServices);
                                if (moneyServices) {
                                    int money = cs2.read<int>(moneyServices + FlagOffsets::m_iAccount);
                                    if (money > 0) DrawFlag(("$" + std::to_string(money)).c_str(), currentConfig->flagMoney.color);
                                }
                            }
                            if (currentConfig->flagArmor.enabled) {
                                int armorVal = cs2.read<int>(currentPawn + FlagOffsets::m_ArmorValue);
                                bool hasHelmet = cs2.read<bool>(currentPawn + FlagOffsets::m_bHasHelmet);
                                if (armorVal > 0) DrawFlag(hasHelmet ? "HK" : "K", currentConfig->flagArmor.color);
                            }
                            if (currentConfig->flagZoom.enabled) {
                                if (cs2.read<bool>(currentPawn + FlagOffsets::m_bIsScoped)) DrawFlag("ZOOM", currentConfig->flagZoom.color);
                            }
                            if (currentConfig->flagKit.enabled) {
                                uintptr_t itemServices = cs2.read<uintptr_t>(currentPawn + FlagOffsets::m_pItemServices);
                                if (itemServices && cs2.read<bool>(itemServices + FlagOffsets::m_bHasDefuser)) DrawFlag("KIT", currentConfig->flagKit.color);
                            }
                            if (currentConfig->flagC4.enabled) {
                                if (entity.weaponName.find("c4") != std::string::npos) DrawFlag("C4", currentConfig->flagC4.color);
                                else if (HasC4InInventory(cs2, currentPawn, entityList)) DrawFlag("C4", currentConfig->flagC4.color);
                            }
                            if (currentConfig->flagBlind.enabled) {
                                if (cs2.read<float>(currentPawn + FlagOffsets::m_flFlashDuration) > 1.2f) DrawFlag("BLIND", currentConfig->flagBlind.color);
                            }
                        // ----------------------------------------------------------------------------------------------------------------------------------------------------------
                            ImGui::PopFont();
                        }
                    }
                }
            }
        }
        renderer.EndRender();
        Sleep(1);
    }
    renderer.Shutdown();
    return 0;
}