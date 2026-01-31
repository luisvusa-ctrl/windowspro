#pragma once

#include <Windows.h>
#include <wininet.h>
#include <urlmon.h>

#include "dependencies/json.hpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "urlmon.lib")

// https://github.com/a2x/cs2-dumper

namespace Offsets
{
	// offsets.hpp
	uintptr_t dwLocalPlayerPawn;
	uintptr_t dwEntityList;
	uintptr_t dwGameRules;
	uintptr_t dwPlantedC4;
	uintptr_t dwViewMatrix;
	uintptr_t dwSensitivity;
	uintptr_t dwSensitivity_sensitivity;

	// client_dll.hpp
	uintptr_t m_pGameSceneNode;
	uintptr_t m_vOldOrigin;
	uintptr_t m_fFlags; // C_BaseEntity
	uintptr_t m_aimPunchAngle;
	uintptr_t m_angEyeAngles;
	uintptr_t m_entitySpottedState; // C_CSPlayerPawn
	uintptr_t m_pObserverServices;
	uintptr_t m_hObserverTarget;
	uintptr_t m_pBulletServices;
	uintptr_t m_totalHitsOnServer;
	uintptr_t m_bSpotted;
	uintptr_t m_modelState;
	uintptr_t m_iTeamNum;
	uintptr_t m_bBombPlanted; // C_CSGameRules
	uintptr_t m_nBombSite;
	uintptr_t m_hPlayerPawn;
	uintptr_t m_hPawn;
	uintptr_t m_flFlashBangTime;
	uintptr_t m_iIDEntIndex;
	uintptr_t m_AttributeManager; // C_EconEntity
	uintptr_t m_Item; // C_AttributeContainer
	uintptr_t m_iItemDefinitionIndex; // C_EconItemView
	uintptr_t m_pClippingWeapon;
	uintptr_t m_iszPlayerName;
	uintptr_t m_vecAbsOrigin;
	uintptr_t m_Glow;
	uintptr_t m_glowColorOverride;
	uintptr_t m_bGlowing;
	uintptr_t m_ArmorValue;
	uintptr_t m_iShotsFired;
	uintptr_t m_bIsScoped;
	uintptr_t m_iHealth;
	uintptr_t m_iClip1;

	// buttons.hpp
	uintptr_t jump;

	bool getOffsetsOnline()
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
			dwLocalPlayerPawn = offsetsJ["client.dll"]["dwLocalPlayerPawn"];
			dwEntityList = offsetsJ["client.dll"]["dwEntityList"];
			dwGameRules = offsetsJ["client.dll"]["dwGameRules"];
			dwPlantedC4 = offsetsJ["client.dll"]["dwPlantedC4"];
			dwViewMatrix = offsetsJ["client.dll"]["dwViewMatrix"];
			dwSensitivity = offsetsJ["client.dll"]["dwSensitivity"];
			dwSensitivity_sensitivity = offsetsJ["client.dll"]["dwSensitivity_sensitivity"];

			InternetCloseHandle(hUrl);
		}

		{
			HINTERNET hUrl{ InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/client_dll.json", nullptr, 0, INTERNET_FLAG_RELOAD, 0) };
			if (!hUrl)
			{
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
			m_pGameSceneNode = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_pGameSceneNode"];
			m_iTeamNum = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"];
			m_fFlags = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_fFlags"];
			m_vOldOrigin = offsetsJ["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"];
			m_pObserverServices = offsetsJ["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_pObserverServices"];
			m_aimPunchAngle = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_aimPunchAngle"];
			m_angEyeAngles = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_angEyeAngles"];
			m_entitySpottedState = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_entitySpottedState"];
			m_hObserverTarget = offsetsJ["client.dll"]["classes"]["CPlayer_ObserverServices"]["fields"]["m_hObserverTarget"];
			m_bSpotted = offsetsJ["client.dll"]["classes"]["EntitySpottedState_t"]["fields"]["m_bSpotted"];
			m_modelState = offsetsJ["client.dll"]["classes"]["CSkeletonInstance"]["fields"]["m_modelState"];
			m_bBombPlanted = offsetsJ["client.dll"]["classes"]["C_CSGameRules"]["fields"]["m_bBombPlanted"];
			m_nBombSite = offsetsJ["client.dll"]["classes"]["C_PlantedC4"]["fields"]["m_nBombSite"];
			m_hPlayerPawn = offsetsJ["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
			m_hPawn = offsetsJ["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_hPawn"];
			m_flFlashBangTime = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_flFlashBangTime"];
			m_iIDEntIndex = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iIDEntIndex"];
			m_AttributeManager = offsetsJ["client.dll"]["classes"]["C_EconEntity"]["fields"]["m_AttributeManager"];
			m_Item = offsetsJ["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"];
			m_iItemDefinitionIndex = offsetsJ["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];
			m_pClippingWeapon = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pClippingWeapon"];
			m_iszPlayerName = offsetsJ["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"];
			m_Glow = offsetsJ["client.dll"]["classes"]["C_BaseModelEntity"]["fields"]["m_Glow"];
			m_glowColorOverride = offsetsJ["client.dll"]["classes"]["CGlowProperty"]["fields"]["m_glowColorOverride"];
			m_bGlowing = offsetsJ["client.dll"]["classes"]["CGlowProperty"]["fields"]["m_bGlowing"];
			m_ArmorValue = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_ArmorValue"];
			m_iShotsFired = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iShotsFired"];
			m_iHealth = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"];
			m_iClip1 = offsetsJ["client.dll"]["classes"]["C_BasePlayerWeapon"]["fields"]["m_iClip1"];
			m_bIsScoped = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsScoped"];
			m_pBulletServices = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pBulletServices"];
			m_totalHitsOnServer = offsetsJ["client.dll"]["classes"]["CCSPlayer_BulletServices"]["fields"]["m_totalHitsOnServer"];
			m_vecAbsOrigin = offsetsJ["client.dll"]["classes"]["CGameSceneNode"]["fields"]["m_vecAbsOrigin"];

			InternetCloseHandle(hUrl);
		}

		{
			HINTERNET hUrl{ InternetOpenUrlA(hSession, "https://raw.githubusercontent.com/a2x/cs2-dumper/refs/heads/main/output/buttons.json", nullptr, 0, INTERNET_FLAG_RELOAD, 0) };
			if (!hUrl)
			{
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
			jump = offsetsJ["client.dll"]["jump"];

			InternetCloseHandle(hUrl);
		}

		InternetCloseHandle(hSession);
	}

	bool dumpOffsets()
	{
		HRESULT hr{ URLDownloadToFileW(nullptr, L"https://github.com/a2x/cs2-dumper/releases/latest/download/cs2-dumper.exe", L"dumper.exe", 0, nullptr) };

		if ((((HRESULT)(hr)) >= 0))
		{
			system(".\\dumper.exe");

			std::ifstream offsetsIF("output/offsets.json");
			std::string offsetsjson((std::istreambuf_iterator<char>(offsetsIF)), std::istreambuf_iterator<char>());
			std::ifstream clientIF("output/client_dll.json");
			std::string clientdll((std::istreambuf_iterator<char>(clientIF)), std::istreambuf_iterator<char>());
			std::ifstream buttonsIF("output/buttons.json");
			std::string buttonsjson((std::istreambuf_iterator<char>(buttonsIF)), std::istreambuf_iterator<char>());

			nlohmann::json offsetsJ{ nlohmann::json::parse(offsetsjson) };
			dwLocalPlayerPawn = offsetsJ["client.dll"]["dwLocalPlayerPawn"];
			dwEntityList = offsetsJ["client.dll"]["dwEntityList"];
			dwGameRules = offsetsJ["client.dll"]["dwGameRules"];
			dwPlantedC4 = offsetsJ["client.dll"]["dwPlantedC4"];
			dwViewMatrix = offsetsJ["client.dll"]["dwViewMatrix"];
			dwSensitivity = offsetsJ["client.dll"]["dwSensitivity"];
			dwSensitivity_sensitivity = offsetsJ["client.dll"]["dwSensitivity_sensitivity"];

			offsetsJ = nlohmann::json::parse(clientdll);
			m_pGameSceneNode = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_pGameSceneNode"];
			m_iTeamNum = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iTeamNum"];
			m_fFlags = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_fFlags"];
			m_vOldOrigin = offsetsJ["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"];
			m_pObserverServices = offsetsJ["client.dll"]["classes"]["C_BasePlayerPawn"]["fields"]["m_pObserverServices"];
			m_aimPunchAngle = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_aimPunchAngle"];
			m_angEyeAngles = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_angEyeAngles"];
			m_entitySpottedState = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_entitySpottedState"];
			m_hObserverTarget = offsetsJ["client.dll"]["classes"]["CPlayer_ObserverServices"]["fields"]["m_hObserverTarget"];
			m_bSpotted = offsetsJ["client.dll"]["classes"]["EntitySpottedState_t"]["fields"]["m_bSpotted"];
			m_modelState = offsetsJ["client.dll"]["classes"]["CSkeletonInstance"]["fields"]["m_modelState"];
			m_bBombPlanted = offsetsJ["client.dll"]["classes"]["C_CSGameRules"]["fields"]["m_bBombPlanted"];
			m_nBombSite = offsetsJ["client.dll"]["classes"]["C_PlantedC4"]["fields"]["m_nBombSite"];
			m_hPlayerPawn = offsetsJ["client.dll"]["classes"]["CCSPlayerController"]["fields"]["m_hPlayerPawn"];
			m_hPawn = offsetsJ["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_hPawn"];
			m_flFlashBangTime = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawnBase"]["fields"]["m_flFlashBangTime"];
			m_iIDEntIndex = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iIDEntIndex"];
			m_AttributeManager = offsetsJ["client.dll"]["classes"]["C_EconEntity"]["fields"]["m_AttributeManager"];
			m_Item = offsetsJ["client.dll"]["classes"]["C_AttributeContainer"]["fields"]["m_Item"];
			m_iItemDefinitionIndex = offsetsJ["client.dll"]["classes"]["C_EconItemView"]["fields"]["m_iItemDefinitionIndex"];
			m_pClippingWeapon = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pClippingWeapon"];
			m_iszPlayerName = offsetsJ["client.dll"]["classes"]["CBasePlayerController"]["fields"]["m_iszPlayerName"];
			m_Glow = offsetsJ["client.dll"]["classes"]["C_BaseModelEntity"]["fields"]["m_Glow"];
			m_glowColorOverride = offsetsJ["client.dll"]["classes"]["CGlowProperty"]["fields"]["m_glowColorOverride"];
			m_bGlowing = offsetsJ["client.dll"]["classes"]["CGlowProperty"]["fields"]["m_bGlowing"];
			m_ArmorValue = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_ArmorValue"];
			m_iShotsFired = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_iShotsFired"];
			m_iHealth = offsetsJ["client.dll"]["classes"]["C_BaseEntity"]["fields"]["m_iHealth"];
			m_iClip1 = offsetsJ["client.dll"]["classes"]["C_BasePlayerWeapon"]["fields"]["m_iClip1"];
			m_bIsScoped = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_bIsScoped"];
			m_pBulletServices = offsetsJ["client.dll"]["classes"]["C_CSPlayerPawn"]["fields"]["m_pBulletServices"];
			m_totalHitsOnServer = offsetsJ["client.dll"]["classes"]["CCSPlayer_BulletServices"]["fields"]["m_totalHitsOnServer"];
			m_vecAbsOrigin = offsetsJ["client.dll"]["classes"]["CGameSceneNode"]["fields"]["m_vecAbsOrigin"];

			offsetsJ = nlohmann::json::parse(buttonsjson);
			jump = offsetsJ["client.dll"]["jump"];
		}

		return true;
	}
}