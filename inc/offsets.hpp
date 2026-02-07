#pragma once

#include <Windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <string>
#include <stdexcept>
#include <fstream>
#include <initializer_list>
#include <cstdlib>

#include "dependencies/json.hpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

// centraliza offsets carregados pelo dumper
// https://github.com/a2x/cs2-dumper

namespace Offsets
{
	// offsets globais
	inline uintptr_t dwLocalPlayerPawn = 0;
	inline uintptr_t dwEntityList = 0;
	inline uintptr_t dwGameRules = 0;
	inline uintptr_t dwPlantedC4 = 0;
	inline uintptr_t dwViewMatrix = 0;
	inline uintptr_t dwSensitivity = 0;
	inline uintptr_t dwSensitivity_sensitivity = 0;
	inline uintptr_t dwGlobalVars = 0;
	inline uintptr_t dwViewAngles = 0;

	// offsets de classes
	inline uintptr_t m_pGameSceneNode = 0;
	inline uintptr_t m_vOldOrigin = 0;
	inline uintptr_t m_fFlags = 0;
	inline uintptr_t m_aimPunchAngle = 0;
	inline uintptr_t m_angEyeAngles = 0;
	inline uintptr_t m_entitySpottedState = 0;
	inline uintptr_t m_pObserverServices = 0;
	inline uintptr_t m_hObserverTarget = 0;
	inline uintptr_t m_pBulletServices = 0;
	inline uintptr_t m_totalHitsOnServer = 0;
	inline uintptr_t m_bSpotted = 0;
	inline uintptr_t m_modelState = 0;
	inline uintptr_t m_iTeamNum = 0;
	inline uintptr_t m_bBombPlanted = 0;
	inline uintptr_t m_nBombSite = 0;
	inline uintptr_t m_hPlayerPawn = 0;
	inline uintptr_t m_hPawn = 0;
	inline uintptr_t m_flFlashBangTime = 0;
	inline uintptr_t m_iIDEntIndex = 0;
	inline uintptr_t m_AttributeManager = 0;
	inline uintptr_t m_Item = 0;
	inline uintptr_t m_iItemDefinitionIndex = 0;
	inline uintptr_t m_pClippingWeapon = 0;
	inline uintptr_t m_iszPlayerName = 0;
	inline uintptr_t m_vecAbsOrigin = 0;
	inline uintptr_t m_Glow = 0;
	inline uintptr_t m_glowColorOverride = 0;
	inline uintptr_t m_bGlowing = 0;
	inline uintptr_t m_ArmorValue = 0;
	inline uintptr_t m_iShotsFired = 0;
	inline uintptr_t m_bIsScoped = 0;
	inline uintptr_t m_iHealth = 0;
	inline uintptr_t m_iClip1 = 0;
	inline uintptr_t m_vecAbsVelocity = 0;
	inline uintptr_t m_pInGameMoneyServices = 0;
	inline uintptr_t m_iAccount = 0;
	inline uintptr_t m_pItemServices = 0;
	inline uintptr_t m_bHasDefuser = 0;
	inline uintptr_t m_bIsDefusing = 0;
	inline uintptr_t m_bIsPlanting = 0;
	inline uintptr_t m_bHasHelmet = 0;
	inline uintptr_t m_flFlashDuration = 0;
	inline uintptr_t m_flFlashMaxAlpha = 0;
	inline uintptr_t m_pWeaponServices = 0;
	inline uintptr_t m_hMyWeapons = 0;
	inline uintptr_t m_vecViewOffset = 0;
	inline uintptr_t m_flC4Blow = 0;
	inline uintptr_t m_flDefuseCountDown = 0;
	inline uintptr_t m_bBeingDefused = 0;
	inline uintptr_t m_flDetonateTime = 0;
	inline uintptr_t m_flSmokeEffectTickBegin = 0;
	inline uintptr_t m_flSmokeEffectTickEnd = 0;
	inline uintptr_t m_hOwnerEntity = 0;

	// offsets de input
	inline uintptr_t jump = 0;

	// funcoes publicas para carregar offsets
	inline bool getOffsetsOnline();
	inline bool dumpOffsets();

	// funcoes internas de carregamento e validacao
	namespace detail
	{
		inline uintptr_t RequireOffset(const nlohmann::json& root, std::initializer_list<const char*> path, const char* label)
		{
			const nlohmann::json* current = &root;
			for (const char* key : path)
			{
				if (!current->contains(key))
				{
					return 0;
				}
				current = &((*current)[key]);
			}
			if (!current->is_number())
			{
				return 0;
			}
			return current->get<uintptr_t>();
		}

		inline bool TryGetOffset(const nlohmann::json& root, std::initializer_list<const char*> path, uintptr_t& outValue)
		{
			const nlohmann::json* current = &root;
			for (const char* key : path)
			{
				if (!current->contains(key))
				{
					return false;
				}
				current = &((*current)[key]);
			}
			if (!current->is_number())
			{
				return false;
			}
			outValue = current->get<uintptr_t>();
			return true;
		}

		inline uintptr_t RequireOffsetWithFallback(const nlohmann::json& root,
			std::initializer_list<const char*> primaryPath, const char* primaryLabel,
			std::initializer_list<const char*> fallbackPath, const char* fallbackLabel)
		{
			uintptr_t value = 0;
			if (TryGetOffset(root, primaryPath, value))
			{
				return value;
			}
			if (TryGetOffset(root, fallbackPath, value))
			{
				return value;
			}
			return 0;
		}

		inline uintptr_t RequireOffsetAny(const nlohmann::json& root, const char* label,
			std::initializer_list<std::initializer_list<const char*>> paths)
		{
			uintptr_t value = 0;
			for (const auto& path : paths)
			{
				if (TryGetOffset(root, path, value))
				{
					return value;
				}
			}
			return 0;
		}

		inline bool TryGetOffsetAny(const nlohmann::json& root,
			std::initializer_list<std::initializer_list<const char*>> paths, uintptr_t& outValue)
		{
			for (const auto& path : paths)
			{
				if (TryGetOffset(root, path, outValue))
				{
					return true;
				}
			}
			return false;
		}

		inline void LoadGlobalOffsets(const nlohmann::json& offsetsJ)
		{
			dwLocalPlayerPawn = RequireOffset(offsetsJ, { "client.dll", "dwLocalPlayerPawn" }, "dwLocalPlayerPawn");
			dwEntityList = RequireOffset(offsetsJ, { "client.dll", "dwEntityList" }, "dwEntityList");
			dwGameRules = RequireOffset(offsetsJ, { "client.dll", "dwGameRules" }, "dwGameRules");
			dwPlantedC4 = RequireOffset(offsetsJ, { "client.dll", "dwPlantedC4" }, "dwPlantedC4");
			dwViewMatrix = RequireOffset(offsetsJ, { "client.dll", "dwViewMatrix" }, "dwViewMatrix");
			dwSensitivity = RequireOffset(offsetsJ, { "client.dll", "dwSensitivity" }, "dwSensitivity");
			dwSensitivity_sensitivity = RequireOffset(offsetsJ, { "client.dll", "dwSensitivity_sensitivity" }, "dwSensitivity_sensitivity");
			dwGlobalVars = RequireOffset(offsetsJ, { "client.dll", "dwGlobalVars" }, "dwGlobalVars");
			dwViewAngles = RequireOffset(offsetsJ, { "client.dll", "dwViewAngles" }, "dwViewAngles");
		}

		inline void LoadClientOffsets(const nlohmann::json& clientJ, const nlohmann::json* serverJ)
		{
			m_pGameSceneNode = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseEntity", "fields", "m_pGameSceneNode" }, "m_pGameSceneNode");
			m_iTeamNum = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseEntity", "fields", "m_iTeamNum" }, "m_iTeamNum");
			m_fFlags = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseEntity", "fields", "m_fFlags" }, "m_fFlags");
			m_vOldOrigin = RequireOffset(clientJ, { "client.dll", "classes", "C_BasePlayerPawn", "fields", "m_vOldOrigin" }, "m_vOldOrigin");
			m_pObserverServices = RequireOffset(clientJ, { "client.dll", "classes", "C_BasePlayerPawn", "fields", "m_pObserverServices" }, "m_pObserverServices");
			m_aimPunchAngle = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_aimPunchAngle" }, "m_aimPunchAngle");
			m_angEyeAngles = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_angEyeAngles" }, "m_angEyeAngles");
			m_entitySpottedState = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_entitySpottedState" }, "m_entitySpottedState");
			m_hObserverTarget = RequireOffset(clientJ, { "client.dll", "classes", "CPlayer_ObserverServices", "fields", "m_hObserverTarget" }, "m_hObserverTarget");
			m_bSpotted = RequireOffset(clientJ, { "client.dll", "classes", "EntitySpottedState_t", "fields", "m_bSpotted" }, "m_bSpotted");
			m_modelState = RequireOffset(clientJ, { "client.dll", "classes", "CSkeletonInstance", "fields", "m_modelState" }, "m_modelState");
			m_bBombPlanted = RequireOffset(clientJ, { "client.dll", "classes", "C_CSGameRules", "fields", "m_bBombPlanted" }, "m_bBombPlanted");
			m_nBombSite = RequireOffset(clientJ, { "client.dll", "classes", "C_PlantedC4", "fields", "m_nBombSite" }, "m_nBombSite");
			m_hPlayerPawn = RequireOffset(clientJ, { "client.dll", "classes", "CCSPlayerController", "fields", "m_hPlayerPawn" }, "m_hPlayerPawn");
			m_hPawn = RequireOffset(clientJ, { "client.dll", "classes", "CBasePlayerController", "fields", "m_hPawn" }, "m_hPawn");
			m_flFlashBangTime = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawnBase", "fields", "m_flFlashBangTime" }, "m_flFlashBangTime");
			m_iIDEntIndex = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_iIDEntIndex" }, "m_iIDEntIndex");
			m_AttributeManager = RequireOffset(clientJ, { "client.dll", "classes", "C_EconEntity", "fields", "m_AttributeManager" }, "m_AttributeManager");
			m_Item = RequireOffset(clientJ, { "client.dll", "classes", "C_AttributeContainer", "fields", "m_Item" }, "m_Item");
			m_iItemDefinitionIndex = RequireOffset(clientJ, { "client.dll", "classes", "C_EconItemView", "fields", "m_iItemDefinitionIndex" }, "m_iItemDefinitionIndex");
			m_pClippingWeapon = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_pClippingWeapon" }, "m_pClippingWeapon");
			m_iszPlayerName = RequireOffset(clientJ, { "client.dll", "classes", "CBasePlayerController", "fields", "m_iszPlayerName" }, "m_iszPlayerName");
			m_Glow = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseModelEntity", "fields", "m_Glow" }, "m_Glow");
			m_glowColorOverride = RequireOffset(clientJ, { "client.dll", "classes", "CGlowProperty", "fields", "m_glowColorOverride" }, "m_glowColorOverride");
			m_bGlowing = RequireOffset(clientJ, { "client.dll", "classes", "CGlowProperty", "fields", "m_bGlowing" }, "m_bGlowing");
			m_ArmorValue = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_ArmorValue" }, "m_ArmorValue");
			m_iShotsFired = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_iShotsFired" }, "m_iShotsFired");
			m_iHealth = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseEntity", "fields", "m_iHealth" }, "m_iHealth");
			m_iClip1 = RequireOffset(clientJ, { "client.dll", "classes", "C_BasePlayerWeapon", "fields", "m_iClip1" }, "m_iClip1");
			m_bIsScoped = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_bIsScoped" }, "m_bIsScoped");
			m_pBulletServices = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_pBulletServices" }, "m_pBulletServices");
			m_totalHitsOnServer = RequireOffset(clientJ, { "client.dll", "classes", "CCSPlayer_BulletServices", "fields", "m_totalHitsOnServer" }, "m_totalHitsOnServer");
			m_vecAbsOrigin = RequireOffset(clientJ, { "client.dll", "classes", "CGameSceneNode", "fields", "m_vecAbsOrigin" }, "m_vecAbsOrigin");
			m_vecAbsVelocity = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseEntity", "fields", "m_vecAbsVelocity" }, "m_vecAbsVelocity");
			m_pInGameMoneyServices = RequireOffset(clientJ, { "client.dll", "classes", "CCSPlayerController", "fields", "m_pInGameMoneyServices" }, "m_pInGameMoneyServices");
			m_iAccount = RequireOffset(clientJ, { "client.dll", "classes", "CCSPlayerController_InGameMoneyServices", "fields", "m_iAccount" }, "m_iAccount");
			m_pItemServices = RequireOffset(clientJ, { "client.dll", "classes", "C_BasePlayerPawn", "fields", "m_pItemServices" }, "m_pItemServices");
			m_bHasDefuser = RequireOffset(clientJ, { "client.dll", "classes", "CCSPlayer_ItemServices", "fields", "m_bHasDefuser" }, "m_bHasDefuser");
			m_bIsDefusing = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_bIsDefusing" }, "m_bIsDefusing");
			m_bIsPlanting = RequireOffsetWithFallback(
				clientJ,
				{ "client.dll", "classes", "C_C4", "fields", "m_bIsPlantingViaUse" }, "m_bIsPlantingViaUse",
				{ "client.dll", "classes", "C_CSPlayerPawn", "fields", "m_bIsPlanting" }, "m_bIsPlanting"
			);
			m_bHasHelmet = RequireOffset(clientJ, { "client.dll", "classes", "CCSPlayer_ItemServices", "fields", "m_bHasHelmet" }, "m_bHasHelmet");
			m_flFlashDuration = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawnBase", "fields", "m_flFlashDuration" }, "m_flFlashDuration");
			m_flFlashMaxAlpha = RequireOffset(clientJ, { "client.dll", "classes", "C_CSPlayerPawnBase", "fields", "m_flFlashMaxAlpha" }, "m_flFlashMaxAlpha");
			m_pWeaponServices = RequireOffset(clientJ, { "client.dll", "classes", "C_BasePlayerPawn", "fields", "m_pWeaponServices" }, "m_pWeaponServices");
			m_hMyWeapons = RequireOffset(clientJ, { "client.dll", "classes", "CPlayer_WeaponServices", "fields", "m_hMyWeapons" }, "m_hMyWeapons");
			m_vecViewOffset = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseModelEntity", "fields", "m_vecViewOffset" }, "m_vecViewOffset");
			m_flC4Blow = RequireOffset(clientJ, { "client.dll", "classes", "C_PlantedC4", "fields", "m_flC4Blow" }, "m_flC4Blow");
			m_flDefuseCountDown = RequireOffset(clientJ, { "client.dll", "classes", "C_PlantedC4", "fields", "m_flDefuseCountDown" }, "m_flDefuseCountDown");
			m_bBeingDefused = RequireOffset(clientJ, { "client.dll", "classes", "C_PlantedC4", "fields", "m_bBeingDefused" }, "m_bBeingDefused");
			{
				uintptr_t detonateTime = 0;
				bool found = TryGetOffsetAny(clientJ, {
					{ "client.dll", "classes", "C_BaseCSGrenadeProjectile", "fields", "m_flDetonateTime" },
					{ "client.dll", "classes", "C_BaseGrenadeProjectile", "fields", "m_flDetonateTime" },
					{ "client.dll", "classes", "C_SmokeGrenadeProjectile", "fields", "m_flDetonateTime" }
					}, detonateTime);
				if (!found && serverJ != nullptr)
				{
					found = TryGetOffsetAny(*serverJ, {
						{ "server.dll", "classes", "C_BaseCSGrenadeProjectile", "fields", "m_flDetonateTime" },
						{ "server.dll", "classes", "C_BaseGrenadeProjectile", "fields", "m_flDetonateTime" },
						{ "server.dll", "classes", "C_SmokeGrenadeProjectile", "fields", "m_flDetonateTime" }
						}, detonateTime);
				}
				m_flDetonateTime = found ? detonateTime : 0;
			}
			m_flSmokeEffectTickBegin = RequireOffset(clientJ, { "client.dll", "classes", "C_SmokeGrenadeProjectile", "fields", "m_flSmokeEffectTickBegin" }, "m_flSmokeEffectTickBegin");
			m_flSmokeEffectTickEnd = RequireOffset(clientJ, { "client.dll", "classes", "C_SmokeGrenadeProjectile", "fields", "m_flSmokeEffectTickEnd" }, "m_flSmokeEffectTickEnd");
			m_hOwnerEntity = RequireOffset(clientJ, { "client.dll", "classes", "C_BaseEntity", "fields", "m_hOwnerEntity" }, "m_hOwnerEntity");
		}

		inline void LoadButtonsOffsets(const nlohmann::json& offsetsJ)
		{
			jump = RequireOffset(offsetsJ, { "client.dll", "jump" }, "jump");
		}
	}

	// carrega offsets online via github
	inline bool getOffsetsOnline()
	{
		HINTERNET hSession{ InternetOpenA("OffsetsFetcher", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0) };
		if (!hSession)
		{
			return false;
		}

		{
			HINTERNET hUrl{ InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/offsets.json", nullptr, 0, INTERNET_FLAG_RELOAD, 0) };
			if (!hUrl)
			{
				InternetCloseHandle(hSession);
				return false;
			};

			std::string content;
			char buffer[8192];
			DWORD bytesRead;
			while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
			{
				content.append(buffer, bytesRead);
			}

			nlohmann::json offsetsJ{ nlohmann::json::parse(content) };
			detail::LoadGlobalOffsets(offsetsJ);

			InternetCloseHandle(hUrl);
		}

		nlohmann::json clientJ;
		{
			HINTERNET hUrl{ InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.json", nullptr, 0, INTERNET_FLAG_RELOAD, 0) };
			if (!hUrl)
			{
				InternetCloseHandle(hSession);
				return false;
			};

			std::string content;
			char buffer[8192];
			DWORD bytesRead;
			while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
			{
				content.append(buffer, bytesRead);
			}
			clientJ = nlohmann::json::parse(content);

			InternetCloseHandle(hUrl);
		}

		nlohmann::json serverJ;
		nlohmann::json* serverPtr = nullptr;
		{
			HINTERNET hUrl{ InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/server_dll.json", nullptr, 0, INTERNET_FLAG_RELOAD, 0) };
			if (!hUrl)
			{
				InternetCloseHandle(hSession);
				return false;
			};

			std::string content;
			char buffer[8192];
			DWORD bytesRead;
			while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
			{
				content.append(buffer, bytesRead);
			}
			if (!content.empty())
			{
				serverJ = nlohmann::json::parse(content);
				serverPtr = &serverJ;
			}

			InternetCloseHandle(hUrl);
		}

		detail::LoadClientOffsets(clientJ, serverPtr);

		{
			HINTERNET hUrl{ InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/buttons.json", nullptr, 0, INTERNET_FLAG_RELOAD, 0) };
			if (!hUrl)
			{
				InternetCloseHandle(hSession);
				return false;
			};

			std::string content;
			char buffer[8192];
			DWORD bytesRead;
			while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
			{
				content.append(buffer, bytesRead);
			}

			nlohmann::json offsetsJ{ nlohmann::json::parse(content) };
			detail::LoadButtonsOffsets(offsetsJ);

			InternetCloseHandle(hUrl);
		}

		InternetCloseHandle(hSession);
		return true;
	}

	// executa dumper local e carrega offsets a partir dos jsons
	inline bool dumpOffsets()
	{
		HRESULT hr{ URLDownloadToFileW(nullptr, L"https://github.com/a2x/cs2-dumper/releases/latest/download/cs2-dumper.exe", L"dumper.exe", 0, nullptr) };

		if ((((HRESULT)(hr)) >= 0))
		{
			system(".\\dumper.exe");

			std::ifstream offsetsIF("output/offsets.json");
			std::string offsetsjson((std::istreambuf_iterator<char>(offsetsIF)), std::istreambuf_iterator<char>());
			std::ifstream clientIF("output/client_dll.json");
			std::string clientdll((std::istreambuf_iterator<char>(clientIF)), std::istreambuf_iterator<char>());
			std::ifstream serverIF("output/server_dll.json");
			std::string serverjson((std::istreambuf_iterator<char>(serverIF)), std::istreambuf_iterator<char>());
			std::ifstream buttonsIF("output/buttons.json");
			std::string buttonsjson((std::istreambuf_iterator<char>(buttonsIF)), std::istreambuf_iterator<char>());

			nlohmann::json offsetsJ{ nlohmann::json::parse(offsetsjson) };
			detail::LoadGlobalOffsets(offsetsJ);

			nlohmann::json clientJ{ nlohmann::json::parse(clientdll) };
			nlohmann::json serverJ;
			nlohmann::json* serverPtr = nullptr;
			if (!serverjson.empty())
			{
				serverJ = nlohmann::json::parse(serverjson);
				serverPtr = &serverJ;
			}
			detail::LoadClientOffsets(clientJ, serverPtr);

			nlohmann::json buttonsJ{ nlohmann::json::parse(buttonsjson) };
			detail::LoadButtonsOffsets(buttonsJ);
		}

		return true;
	}
}
