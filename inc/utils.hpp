#pragma once

// tipos vetoriais e helpers de world-to-screen e ossos

#include <Windows.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

#if __has_include("../imgui/imgui.h")
#include "../imgui/imgui.h"
#elif __has_include("imgui/imgui.h")
#include "imgui/imgui.h"
#endif

#include "mem.hpp"
#include "offsets.hpp"

// vetor 4d com operacoes basicas
struct Vector4
{
    float x;
    float y;
    float z;
    float w;

    Vector4 operator+(const Vector4& other) const
    {
        return { x + other.x, y + other.y, z + other.z, w + other.w };
    }

    Vector4 operator-(const Vector4& other) const
    {
        return { x - other.x, y - other.y, z - other.z, w - other.w };
    }

    Vector4 operator*(const Vector4& other) const
    {
        return { x * other.x, y * other.y, z * other.z, w * other.w };
    }

    Vector4 operator/(const Vector4& other) const
    {
        return { x / other.x, y / other.y, z / other.z, w / other.w };
    }
};

// vetor 3d com operacoes basicas
struct Vector3
{
	float x;
	float y;
	float z;

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vector3 operator+(const Vector3& other) const
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vector3 operator-(const Vector3& other) const
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    Vector3 operator*(const Vector3& other) const
    {
        return { x * other.x, y * other.y, z * other.z };
    }

    Vector3 operator/(const Vector3& other) const
    {
        return { x / other.x, y / other.y, z / other.z };
    }

    Vector3 operator+(const float other) const
    {
        return { x + other, y + other, z + other };
    }

    Vector3 operator-(const float other) const
    {
        return { x - other, y - other, z - other };
    }

    Vector3 operator*(const float other) const
    {
        return { x * other, y * other, z * other };
    }

    Vector3 operator/(const float other) const
    {
        return { x / other, y / other, z / other };
    }

    float dot(const Vector3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const
    {
        return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
    }
    
    float length() const 
    {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    float distance(const Vector3& other) const
    {
        return (*this - other).length();
    }
    
    Vector3 normalize() const
    {
        float len = length();
        return len > 0.0f ? Vector3(x / len, y / len, z / len) : Vector3(0, 0, 0);
    }
};

// vetor 2d com conversao para ImVec2
struct Vector2
{
	float x;
	float y;

    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
    
    operator ImVec2() const { return ImVec2(x, y); }
    
    Vector2(const ImVec2& vec) : x(vec.x), y(vec.y) {}

    Vector2 operator+(const Vector2& other) const
    {
        return { x + other.x, y + other.y };
    }

    Vector2 operator-(const Vector2& other) const
    {
        return { x - other.x, y - other.y };
    }

    Vector2 operator*(const Vector2& other) const
    {
        return { x * other.x, y * other.y };
    }

    Vector2 operator/(const Vector2& other) const
    {
        return { x / other.x, y / other.y };
    }

    Vector2 operator+(const float other) const
    {
        return { x + other, y + other };
    }

    Vector2 operator-(const float other) const
    {
        return { x - other, y - other };
    }

    Vector2 operator*(const float other) const
    {
        return { x * other, y * other };
    }

    Vector2 operator/(const float other) const
    {
        return { x / other, y / other };
    }
    
    float length() const 
    {
        return std::sqrt(x * x + y * y);
    }
    
    Vector2 normalize() const
    {
        float len = length();
        return len > 0.0f ? Vector2(x / len, y / len) : Vector2(0, 0);
    }
};

// matriz 4x4 usada no view matrix
struct Matrix
{
	float matrix[4][4];
};

// pares de ossos para desenhar skeleton
std::vector<Vector2> boneConnections{
	{ 6, 5 },
	{ 5, 4 },
	{ 4, 0 },
	{ 4, 8 },
	{ 8, 9 },
	{ 9, 11 },
	{ 4, 13 },
	{ 13, 14 },
	{ 14, 16 },
	{ 4, 2 },
	{ 0, 22 },
	{ 0, 25 },
	{ 22, 23 },
	{ 23, 24 },
	{ 25, 26 },
	{ 26, 27 }
};

// indices de ossos usados no skeleton
enum BoneIndex : int
{
    HEAD = 6,
    NECK = 5,
    CHEST = 4,
    STOMACH = 2,
    COCK = 0,
    LSHOULDER = 8,
    LARM = 9,
    LHAND = 10,
    RSHOULDER = 13,
    RARM = 14,
    RHAND = 15,
    LTHIGH = 22,
    LCALF = 23,
    LFOOT = 24,
    RTHIGH = 25,
    RCALF = 26,
    RFOOT = 27,
};

// converte coordenada 3d para tela 2d
Vector2 WorldToScreen(const Vector3& worldPos, const Matrix& viewMatrix)
{
	float clip_x{ worldPos.x * viewMatrix.matrix[0][0] + worldPos.y * viewMatrix.matrix[0][1] + worldPos.z * viewMatrix.matrix[0][2] + viewMatrix.matrix[0][3] };
	float clip_y{ worldPos.x * viewMatrix.matrix[1][0] + worldPos.y * viewMatrix.matrix[1][1] + worldPos.z * viewMatrix.matrix[1][2] + viewMatrix.matrix[1][3] };
	float clip_z{ worldPos.x * viewMatrix.matrix[2][0] + worldPos.y * viewMatrix.matrix[2][1] + worldPos.z * viewMatrix.matrix[2][2] + viewMatrix.matrix[2][3] };
	float clip_w{ worldPos.x * viewMatrix.matrix[3][0] + worldPos.y * viewMatrix.matrix[3][1] + worldPos.z * viewMatrix.matrix[3][2] + viewMatrix.matrix[3][3] };

	Vector2 screenPos{ 0.0f, 0.0f };

	if (clip_w < 0.1f)
	{
		return screenPos;
	}

	float ndc_x{ clip_x / clip_w };
	float ndc_y{ clip_y / clip_w };

	int monitorWidth{ GetSystemMetrics(SM_CXSCREEN) };
	int monitorHeight{ GetSystemMetrics(SM_CYSCREEN) };

	screenPos.x = (monitorWidth / 2 * ndc_x) + (ndc_x + monitorWidth / 2);
	screenPos.y = -(monitorHeight / 2 * ndc_y) + (ndc_y + monitorHeight / 2);

	return screenPos;
}

// retorna ponteiro da matriz de ossos do pawn
uintptr_t getBoneMatrix(Memory& mem, uintptr_t pawn)
{
	uintptr_t node{ mem.read<uintptr_t>(pawn + Offsets::m_pGameSceneNode) };
	return mem.read<uintptr_t>(node + Offsets::m_modelState + 128);
}

static const std::unordered_map<std::string, const char*> weaponIcons{
    {"ct_knife", "]"},
    {"t_knife", "["},
    {"deagle", "A"},
    {"elite", "B"},
    {"fiveseven", "C"},
    {"glock", "D"},
    {"revolver", "J"},
    {"p2000", "E"},
    {"p250", "F"},
    {"usp", "G"},
    {"tec9", "H"},
    {"cz75a", "I"},
    {"mac10", "K"},
    {"ump45", "L"},
    {"bizon", "M"},
    {"mp7", "N"},
    {"mp9", "P"},
    {"p90", "O"},
    {"galilar", "Q"},
    {"famas", "R"},
    {"m4a1", "T"},
    {"m4a4", "S"},
    {"aug", "U"},
    {"sg556", "V"},
    {"ak47", "W"},
    {"g3sg1", "X"},
    {"scar20", "Y"},
    {"awp", "Z"},
    {"ssg08", "a"},
    {"xm1014", "b"},
    {"sawedoff", "c"},
    {"mag7", "d"},
    {"nova", "e"},
    {"negev", "f"},
    {"m249", "g"},
    {"taser", "h"},
    {"flashbang", "i"},
    {"hegrenade", "j"},
    {"smokegrenade", "k"},
    {"molotov", "l"},
    {"decoy", "m"},
    {"incgrenade", "n"},
    {"c4", "o"}
};

struct WeaponIconSize
{
    float width;
    float height;
    float offsetX;
    float offsetY;
};

std::unordered_map<std::string, WeaponIconSize> weaponIconSizes{
    {"t_knife", {13.0f, 13.0f, -5.0f, 0.0f}},
    {"ct_knife", {13.0f, 13.0f, -5.0f, 0.0f}},
    {"deagle", {13.0f, 13.0f, -5.0f, 0.0f}},
    {"elite", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"fiveseven", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"glock", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"revolver", {13.0f, 13.0f, -3.0f, 0.0f}},
    {"p2000", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"p250", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"usp", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"tec9", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"cz75a", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"mac10", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"ump45", {13.0f, 13.0f, -7.0f, 0.0f}},
    {"bizon", {13.0f, 13.0f, -7.0f, 0.0f}},
    {"mp7", {13.0f, 13.0f, -3.0f, 0.0f}},
    {"mp9", {13.0f, 13.0f, -7.0f, 0.0f}},
    {"p90", {13.0f, 13.0f, -7.0f, 0.0f}},
    {"galilar", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"famas", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"m4a4", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"m4a1", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"aug", {13.0f, 13.0f, -7.0f, 0.0f}},
    {"sg556", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"ak47", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"g3sg1", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"scar20", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"awp", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"ssg08", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"xm1014", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"sawedoff", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"mag7", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"nova", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"negev", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"m249", {13.0f, 13.0f, -10.0f, 0.0f}},
    {"taser", {13.0f, 13.0f, 0.0f, 0.0f}},
    {"flashbang", {13.0f, 13.0f, 3.0f, 0.0f}},
    {"hegrenade", {13.0f, 13.0f, 3.0f, 0.0f}},
    {"smokegrenade", {13.0f, 13.0f, 3.0f, 0.0f}},
    {"molotov", {13.0f, 13.0f, 3.0f, 0.0f}},
    {"decoy", {13.0f, 13.0f, 3.0f, 0.0f}},
    {"incgrenade", {13.0f, 13.0f, 3.0f, 0.0f}},
    {"c4", {13.0f, 13.0f, 0.0f, 0.0f}},
};

std::unordered_map<int, std::string> weaponNames{
    {1, "deagle"},
    {2, "elite"},
    {3, "fiveseven"},
    {4, "glock"},
    {7, "ak47"},
    {8, "aug"},
    {9, "awp"},
    {10, "famas"},
    {11, "g3Sg1"},
    {13, "galilar"},
    {14, "m249"},
    {16, "m4a4"},
    {17, "mac10"},
    {19, "p90"},
    {23, "mp5sd"},
    {24, "ump45"},
    {25, "xm1014"},
    {26, "bizon"},
    {27, "mag7"},
    {28, "negev"},
    {29, "sawedoff"},
    {30, "tec9"},
    {31, "zeus"},
    {32, "p2000"},
    {33, "mp7"},
    {34, "mp9"},
    {35, "nova"},
    {36, "p250"},
    {38, "scar20"},
    {39, "sg556"},
    {40, "ssg08"},
    {42, "ct_knife"},
    {43, "flashbang"},
    {44, "hegrenade"},
    {45, "smokegrenade"},
    {46, "molotov"},
    {47, "decoy"},
    {48, "incgrenade"},
    {49, "c4"},
    {59, "t_knife"},
    {60, "m4a1"},
    {61, "usp"},
    {63, "cz75a"},
    {64, "revolver"}
};

std::unordered_map<std::string, std::string> weaponTypes{
    {"weapon_ak47", "AK-47"},
    {"weapon_m4a1", "M4A1"},
    {"weapon_awp", "AWP"},
    {"weapon_elite", "Elite"},
    {"weapon_famas", "Famas"},
    {"weapon_flashbang", "Flashbang"},
    {"weapon_g3sg1", "G3SG1"},
    {"weapon_galilar", "Galil AR"},
    {"weapon_healthshot", "Health Shot"},
    {"weapon_hegrenade", "HE Grenade"},
    {"weapon_incgrenade", "Incendiary Grenade"},
    {"weapon_m249", "M249"},
    {"weapon_m4a1_silencer", "M4A1-S"},
    {"weapon_mac10", "MAC-10"},
    {"weapon_mag7", "MAG-7"},
    {"weapon_molotov", "Molotov"},
    {"weapon_mp5sd", "MP5-SD"},
    {"weapon_mp7", "MP7"},
    {"weapon_mp9", "MP9"},
    {"weapon_negev", "Negev"},
    {"weapon_nova", "Nova"},
    {"weapon_p90", "P90"},
    {"weapon_sawedoff", "Sawed-Off"},
    {"weapon_scar20", "SCAR-20"},
    {"weapon_sg556", "SG 553"},
    {"weapon_smokegrenade", "Smoke Grenade"},
    {"weapon_ssg08", "SSG 08"},
    {"weapon_tagrenade", "TA Grenade"},
    {"weapon_taser", "Taser"},
    {"weapon_ump45", "UMP-45"},
    {"weapon_xm1014", "XM1014"},
    {"weapon_aug", "AUG"},
    {"weapon_bizon", "PP-Bizon"},
    {"weapon_decoy", "Decoy Grenade"},
    {"weapon_fiveseven", "Five-Seven"},
    {"weapon_hkp2000", "P2000"},
    {"weapon_usp_silencer", "USP-S"},
    {"weapon_p250", "P250"},
    {"weapon_tec9", "Tec-9"},
    {"weapon_cz75a", "CZ75-Auto"},
    {"weapon_deagle", "Desert Eagle"},
    {"weapon_revolver", "R8 Revolver"},
    {"weapon_glock", "Glock-18"}
};

std::unordered_map<std::string, std::string> projectileTypes{
    {"smokegrenade_projectile", "Smoke Grenade"},
    {"flashbang_projectile", "Flashbang"},
    {"hegrenade_projectile", "HE Grenade"},
    {"molotov_projectile", "Molotov"},
    {"incendiarygrenade_projectile", "Incendiary Grenade"},
    {"decoy_projectile", "Decoy Grenade"},
    {"inferno", "Molotov"}
};

const char* getWeaponIcon(const std::string weapon)
{
    auto it{ weaponIcons.find(weapon) };
    if (it != weaponIcons.end())
    {
        return it->second;
    }

    return "";
}

std::string getWeaponType(std::string identifier)
{
    auto it{ weaponTypes.find(identifier) };
    if (it != weaponTypes.end())
    {
        return it->second;
    }

    return "unknown";
}

std::string getProjectileType(std::string identifier)
{
    auto it{ projectileTypes.find(identifier) };
    if (it != projectileTypes.end())
    {
        return it->second;
    }

    return "unknown";
}

class Entity
{
public:
    uintptr_t currentPawn;
    uintptr_t currentController;

    int health;
    int armor;
    int team;
    int shotsFired;
    bool spotted;
    bool isScoped;
    std::string name;
    std::string weaponName;
    Vector3 position;
    Vector2 aimPunchAngle;

    Entity(Memory& mem, uintptr_t currPawn = 0, uintptr_t currController = 0) : currentPawn(currPawn), currentController(currController)
    {
        health = mem.read<int>(currentPawn + Offsets::m_iHealth);
        armor = mem.read<int>(currentPawn + Offsets::m_ArmorValue);
        team = mem.read<int>(currentPawn + Offsets::m_iTeamNum);
        shotsFired = mem.read<int>(currentPawn + Offsets::m_iShotsFired);
        spotted = mem.read<bool>(currentPawn + Offsets::m_entitySpottedState + Offsets::m_bSpotted);
        isScoped = mem.read<bool>(currentPawn + Offsets::m_bIsScoped);
        name = mem.readstr(currentController + Offsets::m_iszPlayerName);
        position = mem.read<Vector3>(currentPawn + Offsets::m_vOldOrigin);
        aimPunchAngle = mem.read<Vector2>(currentPawn + Offsets::m_aimPunchAngle);

        // weapon name
        {
            DWORD64 currentWeapon{ mem.read<DWORD64>(currentPawn + Offsets::m_pClippingWeapon) };

            short weaponIndex{ mem.read<short>(currentWeapon + Offsets::m_AttributeManager + Offsets::m_Item + Offsets::m_iItemDefinitionIndex) };
            auto it{ weaponNames.find(weaponIndex) };
            weaponName = (it != weaponNames.end()) ? it->second : "Weapon_None";
        }
    }
};

static const char* KeyNames[]{
    "OFF",
    "VK_LBUTTON",
    "VK_RBUTTON",
    "VK_CANCEL",
    "VK_MBUTTON",
    "VK_XBUTTON1",
    "VK_XBUTTON2",
    "Unknown",
    "VK_BACK",
    "VK_TAB",
    "Unknown",
    "Unknown",
    "VK_CLEAR",
    "VK_RETURN",
    "Unknown",
    "Unknown",
    "VK_SHIFT",
    "VK_CONTROL",
    "VK_MENU",
    "VK_PAUSE",
    "VK_CAPITAL",
    "VK_KANA",
    "Unknown",
    "VK_JUNJA",
    "VK_FINAL",
    "VK_KANJI",
    "Unknown",
    "VK_ESCAPE",
    "VK_CONVERT",
    "VK_NONCONVERT",
    "VK_ACCEPT",
    "VK_MODECHANGE",
    "VK_SPACE",
    "VK_PRIOR",
    "VK_NEXT",
    "VK_END",
    "VK_HOME",
    "VK_LEFT",
    "VK_UP",
    "VK_RIGHT",
    "VK_DOWN",
    "VK_SELECT",
    "VK_PRINT",
    "VK_EXECUTE",
    "VK_SNAPSHOT",
    "VK_INSERT",
    "VK_DELETE",
    "VK_HELP",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "VK_LWIN",
    "VK_RWIN",
    "VK_APPS",
    "Unknown",
    "VK_SLEEP",
    "VK_NUMPAD0",
    "VK_NUMPAD1",
    "VK_NUMPAD2",
    "VK_NUMPAD3",
    "VK_NUMPAD4",
    "VK_NUMPAD5",
    "VK_NUMPAD6",
    "VK_NUMPAD7",
    "VK_NUMPAD8",
    "VK_NUMPAD9",
    "VK_MULTIPLY",
    "VK_ADD",
    "VK_SEPARATOR",
    "VK_SUBTRACT",
    "VK_DECIMAL",
    "VK_DIVIDE",
    "VK_F1",
    "VK_F2",
    "VK_F3",
    "VK_F4",
    "VK_F5",
    "VK_F6",
    "VK_F7",
    "VK_F8",
    "VK_F9",
    "VK_F10",
    "VK_F11",
    "VK_F12",
    "VK_F13",
    "VK_F14",
    "VK_F15",
    "VK_F16",
    "VK_F17",
    "VK_F18",
    "VK_F19",
    "VK_F20",
    "VK_F21",
    "VK_F22",
    "VK_F23",
    "VK_F24",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "VK_NUMLOCK",
    "VK_SCROLL",
    "VK_OEM_NEC_EQUAL",
    "VK_OEM_FJ_MASSHOU",
    "VK_OEM_FJ_TOUROKU",
    "VK_OEM_FJ_LOYA",
    "VK_OEM_FJ_ROYA",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "VK_LSHIFT",
    "VK_RSHIFT",
    "VK_LCONTROL",
    "VK_RCONTROL",
    "VK_LMENU",
    "VK_RMENU"
};

static const int KeyCodes[]{
    0x0,  //Undefined
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07, //Undefined
    0x08,
    0x09,
    0x0A, //Reserved
    0x0B, //Reserved
    0x0C,
    0x0D,
    0x0E, //Undefined
    0x0F, //Undefined
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16, //IME On
    0x17,
    0x18,
    0x19,
    0x1A, //IME Off
    0x1B,
    0x1C,
    0x1D,
    0x1E,
    0x1F,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x2B,
    0x2C,
    0x2D,
    0x2E,
    0x2F,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A, //Undefined
    0x3B, //Undefined
    0x3C, //Undefined
    0x3D, //Undefined
    0x3E, //Undefined
    0x3F, //Undefined
    0x40, //Undefined
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x4B,
    0x4C,
    0x4B,
    0x4E,
    0x4F,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x5B,
    0x5C,
    0x5D,
    0x5E, //Rservered
    0x5F,
    0x60, //Numpad1
    0x61, //Numpad2
    0x62, //Numpad3
    0x63, //Numpad4
    0x64, //Numpad5
    0x65, //Numpad6
    0x66, //Numpad7
    0x67, //Numpad8
    0x68, //Numpad8
    0x69, //Numpad9
    0x6A,
    0x6B,
    0x6C,
    0x6D,
    0x6E,
    0x6F,
    0x70, //F1
    0x71, //F2
    0x72, //F3
    0x73, //F4
    0x74, //F5
    0x75, //F6
    0x76, //F7
    0x77, //F8
    0x78, //F9
    0x79, //F10
    0x7A, //F11
    0x7B, //F12
    0x7C, //F13
    0x7D, //F14
    0x7E, //F15
    0x7F, //F16
    0x80, //F17
    0x81, //F18
    0x82, //F19
    0x83, //F20
    0x84, //F21
    0x85, //F22
    0x86, //F23
    0x87, //F24
    0x88, //Unkown
    0x89, //Unkown
    0x8A, //Unkown
    0x8B, //Unkown
    0x8C, //Unkown
    0x8D, //Unkown
    0x8E, //Unkown
    0x8F, //Unkown
    0x90,
    0x91,
    0x92, //OEM Specific
    0x93, //OEM Specific
    0x94, //OEM Specific
    0x95, //OEM Specific
    0x96, //OEM Specific
    0x97, //Unkown
    0x98, //Unkown
    0x99, //Unkown
    0x9A, //Unkown
    0x9B, //Unkown
    0x9C, //Unkown
    0x9D, //Unkown
    0x9E, //Unkown 
    0x9F, //Unkown
    0xA0,
    0xA1,
    0xA2,
    0xA3,
    0xA4,
    0xA5
};

namespace ImGui
{
    void Hotkey(int* k, const ImVec2& size_arg = ImVec2(0, 0))
    {
        ImGui::PushID(k);

        static std::unordered_map<ImGuiID, bool> waitingforkey;
        bool& waiting{ waitingforkey[ImGui::GetID("")] };
        if (!waiting) {
            if (ImGui::Button(KeyNames[*k], size_arg))
                waiting = true;
        }
        else
        {
            ImGui::Button("...", size_arg);
            Sleep(20);
            for (auto& Key : KeyCodes)
            {
                if (GetAsyncKeyState(Key) & 0x8000) {
                    *k = Key;
                    waiting = false;
                }
            }
        }

        ImGui::PopID();
    }
}

void DrawGlow(ImDrawList* drawList, ImVec2 start, ImVec2 end, ImVec4 color, int layers, float fadingPerLayer, float rounding)
{
    ImVec4 currentColor{ color };

    for (int i{ 0 }; i < layers; i++)
    {
        drawList->AddRect(start, end, ImColor(currentColor), rounding);
        drawList->AddRect({ start.x - i, start.y - i }, { end.x + i, end.y + i }, ImColor(currentColor), rounding + i);
        currentColor.w = (std::max)(0.0f, currentColor.w - fadingPerLayer);
    }
}
