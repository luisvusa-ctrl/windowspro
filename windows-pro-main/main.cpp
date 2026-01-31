#include <iostream>
#include <Windows.h>
#include <mmsystem.h>
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

namespace Globals
{
	bool imguiVisible{ false };

	bool aimbotEnabled{ false };
	bool aimbotToggleEnabled{ false };
	bool aimbotVisCheckEnabled{ false };
	std::string aimbotLockPart{ "Head" };
	bool aimbotVisualiseTargetEnabled{ false };
	bool aimbotTargetTracerEnabled{ false };
	bool aimbotFOVEnabled{ false };
	bool aimbotTeamcheckEnabled{ true };
	float aimbotFOVSize{ 1000.0f };
	float aimbotSmoothing{ 1.0f };
	int aimbotKey{ 2 };
	bool aimbotToggled{ false };

	bool triggerbotEnabled{ false };
	bool triggerbotTeamcheckEnabled{ true };
	int triggerbotKey{ 164 };

	bool rcsEnabled{ false };
	Vector2 rcsXY{ 2.0f, 2.0f };
	int rcsShotsFired{ 1 };

	bool espEnabled{ false };
	bool espBoxEnabled{ true };
	bool espSkeletonEnabled{ false };
	bool espTracersEnabled{ false };
	bool espFilled{ false };
	bool espShowName{ true };
	bool espShowHealth{ true };
	bool espShowArmor{ true };
	bool espAmmoBarEnabled{ false };
	bool espShowWeapon{ true };
	bool espEyeRay{ false };
	bool espTeamcheckEnabled{ true };
	bool espSpottedEnabled{ false };
	bool espVisCheckEnabled{ false };
	bool espDynamicColorEnabled{ true };
	ImVec4 espBoxColor{ 1.0f, 0.0f, 0.0f, 1.0f };
	ImVec4 espSkeletonColor{ 1.0f, 0.0f, 0.0f, 1.0f };
	ImVec4 espTracersColor{ 1.0f, 0.0f, 0.0f, 1.0f };
	ImVec4 espSpottedColor{ 1.0f, 1.0f, 0.0f, 1.0f };
	ImVec4 espVisCheckColor{ 0.0f, 0.0f, 1.0f, 1.0f };

	bool wEspbombEspEnabled{ false };
	bool wEspShowWeaponsEnabled{ false };
	bool wEspShowProjectilesEnabled{ false };

	bool spectatorListEnabled{ false };
	bool sniperCrosshairEnabled{ false };
	bool hitMarkerEnabled{ false };
	bool hitSoundEnabled{ false };
	wchar_t hitSoundPath[MAX_PATH]{};
	wchar_t visCheckMapPath[MAX_PATH]{};
	bool streamproofEnabled{ false };
	bool antiFlashEnabled{ false };
	bool bHopEnabled{ false };
	bool radarEnabled{ false };
	bool playerGlowEnabled{ false };

	const char* currentTab{ "Aiming" };
	Vector4 aimingTabColor{ 255, 255, 255, 255 };
	Vector4 visualsTabColor{ 222, 222, 222, 255 };
	Vector4 miscTabColor{ 222, 222, 222, 255 };
	Vector4 detectedTabColor{ 222, 222, 222, 255 };
	float aimingTabSelectedPos{ 70.0f };
	float visualsTabSelectedPos{ 129.0f };
	float miscTabSelectedPos{ 188.0f };
	float detectedTabSelectedPos{ 247.0f };
}

int main()
{
	DWORD cs2PID{ getPID(L"cs2.exe") };

	if (cs2PID == 0)
	{
		std::cout << "Failed to find cs2 process.\n";

		system("pause");
		return 1;
	}

	int offsetsChoice{ 0 };
	while (true)
	{
		std::cout << "1. Get offsets online.\n";
		std::cout << "2. Dump offsets. (use this if online offsets are outdated)\n";
		std::cout << "> ";
		std::cin >> offsetsChoice;

		if (offsetsChoice == 1 || offsetsChoice == 2)
		{
			break;
		}
	}

	system("cls");
	std::cout << "Getting offsets...\n";

	if (offsetsChoice == 1)
	{
		if (Offsets::getOffsetsOnline())
		{
			system("cls");
		}
		else
		{
			std::cerr << "Failed to get offsets!\n";

			system("pause");
			return 1;
		}
	}
	else
	{
		if (Offsets::dumpOffsets())
		{
			system("cls");
		}
		else
		{
			std::cerr << "Failed to get offsets!\n";

			system("pause");
			return 1;
		}
	}

	Memory cs2{ cs2PID };
	uintptr_t client{ getModuleBase(cs2PID, L"client.dll") };

	int monitorWidth{ GetSystemMetrics(SM_CXSCREEN) };
	int monitorHeight{ GetSystemMetrics(SM_CYSCREEN) };

	Renderer renderer;
	renderer.Init();

	ULONGLONG bombLastTick{ GetTickCount64() };
	int bombTimeLeft{ 41 };
	bool bombTimerStarted{ false };

	ImColor terroristColor{ ImColor(234, 209, 139, 255) };
	ImColor counterTerroristColor{ ImColor(182, 212, 238, 255) };

	std::vector<Entity> aimbotPlrList;
	std::vector<Entity> spectatingPlrs;

	Vector2 rcsOldPunch{ 0, 0 };

	ULONGLONG hitMarkerLastTick{ GetTickCount64() };
	int hitMarkerPrevTotalHits{ 0 };
	int hitSoundPrevTotalHits{ 0 };
	float hitMarkerAlpha{ 0.0f };

	VisCheck vCheck{ "" };

	std::vector<std::string> aimbotLockParts{ "Head", "Chest", "Left Arm", "Right Arm", "Left Leg", "Right Leg" };

	NtUserInjectMouseInput::InitInjectMouseInput();

	ImGuiIO& imguiIo{ ImGui::GetIO() };

	
	imguiIo.Fonts->AddFontDefault();

	
	imguiIo.Fonts->AddFontFromMemoryTTF(weaponFont, weaponFontSize, 20.0f);
	imguiIo.Fonts->AddFontFromMemoryTTF(firaCodeFont, firaCodeFontSize, 16.0f);	
	tahomaBoldFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahomabd.ttf", 12.0f);
	if (!tahomaBoldFont) {
		tahomaBoldFont = imguiIo.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 12.0f); 
		if (!tahomaBoldFont) tahomaBoldFont = ImGui::GetFont(); 
	}

	ImGuiStyle& imguiStyle{ ImGui::GetStyle() };
	imguiStyle.Colors[ImGuiCol_Border] = { 0.0f, 0.0f, 0.0f, 0.0f };
	imguiStyle.Colors[ImGuiCol_CheckMark] = ImColor(222, 222, 222, 255);
	imguiStyle.Colors[ImGuiCol_TitleBg] = ImColor(22, 22, 22, 255);
	imguiStyle.Colors[ImGuiCol_TitleBgActive] = ImColor(22, 22, 22, 255);
	imguiStyle.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(22, 22, 22, 255);
	imguiStyle.Colors[ImGuiCol_FrameBg] = ImColor(15, 15, 15, 255);
	imguiStyle.Colors[ImGuiCol_Button] = ImColor(15, 15, 15, 255);
	imguiStyle.Colors[ImGuiCol_FrameBgHovered] = ImColor(26, 26, 26, 255);
	imguiStyle.Colors[ImGuiCol_FrameBgActive] = ImColor(26, 26, 26, 255);
	imguiStyle.Colors[ImGuiCol_ButtonHovered] = ImColor(26, 26, 26, 255);
	imguiStyle.Colors[ImGuiCol_ButtonActive] = ImColor(26, 26, 26, 255);
	imguiStyle.Colors[ImGuiCol_SliderGrab] = ImColor(222, 222, 222, 255);
	imguiStyle.Colors[ImGuiCol_SliderGrabActive] = ImColor(222, 222, 222, 255);
	imguiStyle.Colors[ImGuiCol_HeaderHovered] = ImColor(22, 22, 22, 255);
	imguiStyle.Colors[ImGuiCol_HeaderActive] = ImColor(22, 22, 22, 255);
	imguiStyle.Colors[ImGuiCol_Header] = ImColor(22, 22, 22, 255);
	imguiStyle.ScrollbarRounding = 0.0f;

	while (true)
	{
		renderer.StartRender();

		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			Globals::imguiVisible = !Globals::imguiVisible;
		}

		if (Globals::imguiVisible)
		{
			SetWindowLong(renderer.hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT);

			ImGui::SetNextWindowSize({ 650, 610 });
			ImGui::Begin("Vortix CS2", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

			ImVec2 p{ ImGui::GetCursorScreenPos() };
			DrawGlow(ImGui::GetBackgroundDrawList(), { p.x - 9.0f, p.y - 9.0f }, { p.x + 643.0f, p.y + 603.0f }, { 0.0039215686274509803921568627451f, 0.0039215686274509803921568627451f, 0.0039215686274509803921568627451f, 0.9f }, 20, 0.05f, 0.0f);

			ImDrawList* drawList{ ImGui::GetWindowDrawList() };

			drawList->AddText(imguiIo.Fonts->Fonts[2], 14.0f, { p.x, p.y - 4.0f }, IM_COL32_WHITE, "Vortix CS2");

			drawList->AddLine({ p.x - 7.0f, p.y + 15.0f }, { p.x + 641.0f, p.y + 15.0f }, IM_COL32(222, 222, 222, 255));
			drawList->AddLine({ p.x + 100.0f, p.y + 15.0f }, { p.x + 100.0f, p.y + 601.0f }, IM_COL32(222, 222, 222, 255));

			POINT mouse;
			GetCursorPos(&mouse);

			bool aimingTabHover{ mouse.x >= p.x - 2.0f && mouse.x <= p.x + 92.0f && mouse.y >= p.y + 21.0f && mouse.y <= p.y + 70.0f };
			if (aimingTabHover)
			{
				ImGui::Animate(222, 255, 200, &Globals::aimingTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::aimingTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::aimingTabColor.z);

				ImGui::Animate(70, 21, 200, &Globals::aimingTabSelectedPos);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					Globals::currentTab = "Aiming";
				}
			}
			else
			{
				ImGui::Animate(255, 222, 200, &Globals::aimingTabColor.x);
				ImGui::Animate(255, 222, 200, &Globals::aimingTabColor.y);
				ImGui::Animate(255, 222, 200, &Globals::aimingTabColor.z);

				if (Globals::currentTab != "Aiming")
				{
					ImGui::Animate(21, 70, 200, &Globals::aimingTabSelectedPos);
				}
			}

			if (Globals::currentTab == "Aiming")
			{
				ImGui::Animate(222, 255, 200, &Globals::aimingTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::aimingTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::aimingTabColor.z);

				ImGui::Animate(70, 21, 200, &Globals::aimingTabSelectedPos);
			}

			drawList->AddLine({ p.x - 2.0f, p.y + 21.0f }, { p.x + 92.0f, p.y + 21.0f }, IM_COL32(Globals::aimingTabColor.x, Globals::aimingTabColor.y, Globals::aimingTabColor.z, Globals::aimingTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 70.0f }, { p.x + 92.0f, p.y + 70.0f }, IM_COL32(Globals::aimingTabColor.x, Globals::aimingTabColor.y, Globals::aimingTabColor.z, Globals::aimingTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 70.0f }, { p.x - 2.0f, p.y + Globals::aimingTabSelectedPos }, IM_COL32(Globals::aimingTabColor.x, Globals::aimingTabColor.y, Globals::aimingTabColor.z, Globals::aimingTabColor.w));
			drawList->AddText(imguiIo.Fonts->Fonts[2], 28.0f, { p.x + 6.0f, p.y + 30.0f }, IM_COL32(Globals::aimingTabColor.x, Globals::aimingTabColor.y, Globals::aimingTabColor.z, Globals::aimingTabColor.w), "Aiming");

			bool visualsTabHover{ mouse.x >= p.x - 2.0f && mouse.x <= p.x + 92.0f && mouse.y >= p.y + 80.0f && mouse.y <= p.y + 129.0f };
			if (visualsTabHover)
			{
				ImGui::Animate(222, 255, 200, &Globals::visualsTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::visualsTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::visualsTabColor.z);

				ImGui::Animate(129, 80, 200, &Globals::visualsTabSelectedPos);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					Globals::currentTab = "Visuals";
				}
			}
			else
			{
				ImGui::Animate(255, 222, 200, &Globals::visualsTabColor.x);
				ImGui::Animate(255, 222, 200, &Globals::visualsTabColor.y);
				ImGui::Animate(255, 222, 200, &Globals::visualsTabColor.z);

				if (Globals::currentTab != "Visuals")
				{
					ImGui::Animate(80, 129, 200, &Globals::visualsTabSelectedPos);
				}
			}

			if (Globals::currentTab == "Visuals")
			{
				ImGui::Animate(222, 255, 200, &Globals::visualsTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::visualsTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::visualsTabColor.z);

				ImGui::Animate(129, 80, 200, &Globals::visualsTabSelectedPos);
			}

			drawList->AddLine({ p.x - 2.0f, p.y + 80.0f }, { p.x + 92.0f, p.y + 80.0f }, IM_COL32(Globals::visualsTabColor.x, Globals::visualsTabColor.y, Globals::visualsTabColor.z, Globals::visualsTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 129.0f }, { p.x + 92.0f, p.y + 129.0f }, IM_COL32(Globals::visualsTabColor.x, Globals::visualsTabColor.y, Globals::visualsTabColor.z, Globals::visualsTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 129.0f }, { p.x - 2.0f, p.y + Globals::visualsTabSelectedPos }, IM_COL32(Globals::visualsTabColor.x, Globals::visualsTabColor.y, Globals::visualsTabColor.z, Globals::visualsTabColor.w));
			drawList->AddText(imguiIo.Fonts->Fonts[2], 28.0f, { p.x, p.y + 89.0f }, IM_COL32(Globals::visualsTabColor.x, Globals::visualsTabColor.y, Globals::visualsTabColor.z, Globals::visualsTabColor.w), "Visuals");

			bool miscTabHover{ mouse.x >= p.x - 2.0f && mouse.x <= p.x + 92.0f && mouse.y >= p.y + 139.0f && mouse.y <= p.y + 188.0f };
			if (miscTabHover)
			{
				ImGui::Animate(222, 255, 200, &Globals::miscTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::miscTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::miscTabColor.z);

				ImGui::Animate(188, 139, 200, &Globals::miscTabSelectedPos);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					Globals::currentTab = "Misc";
				}
			}
			else
			{
				ImGui::Animate(255, 222, 200, &Globals::miscTabColor.x);
				ImGui::Animate(255, 222, 200, &Globals::miscTabColor.y);
				ImGui::Animate(255, 222, 200, &Globals::miscTabColor.z);

				if (Globals::currentTab != "Misc")
				{
					ImGui::Animate(139, 188, 200, &Globals::miscTabSelectedPos);
				}
			}

			if (Globals::currentTab == "Misc")
			{
				ImGui::Animate(222, 255, 200, &Globals::miscTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::miscTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::miscTabColor.z);

				ImGui::Animate(188, 139, 200, &Globals::miscTabSelectedPos);
			}

			drawList->AddLine({ p.x - 2.0f, p.y + 139.0f }, { p.x + 92.0f, p.y + 139.0f }, IM_COL32(Globals::miscTabColor.x, Globals::miscTabColor.y, Globals::miscTabColor.z, Globals::miscTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 188.0f }, { p.x + 92.0f, p.y + 188.0f }, IM_COL32(Globals::miscTabColor.x, Globals::miscTabColor.y, Globals::miscTabColor.z, Globals::miscTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 188.0f }, { p.x - 2.0f, p.y + Globals::miscTabSelectedPos }, IM_COL32(Globals::miscTabColor.x, Globals::miscTabColor.y, Globals::miscTabColor.z, Globals::miscTabColor.w));
			drawList->AddText(imguiIo.Fonts->Fonts[2], 28.0f, { p.x + 20.0f, p.y + 148.0f }, IM_COL32(Globals::miscTabColor.x, Globals::miscTabColor.y, Globals::miscTabColor.z, Globals::miscTabColor.w), "Misc");

			bool detectedTabHover{ mouse.x >= p.x - 2.0f && mouse.x <= p.x + 92.0f && mouse.y >= p.y + 198.0f && mouse.y <= p.y + 247.0f };
			if (detectedTabHover)
			{
				ImGui::Animate(222, 255, 200, &Globals::detectedTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::detectedTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::detectedTabColor.z);

				ImGui::Animate(247, 198, 200, &Globals::detectedTabSelectedPos);

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					Globals::currentTab = "Detected";
				}
			}
			else
			{
				ImGui::Animate(255, 222, 200, &Globals::detectedTabColor.x);
				ImGui::Animate(255, 222, 200, &Globals::detectedTabColor.y);
				ImGui::Animate(255, 222, 200, &Globals::detectedTabColor.z);

				if (Globals::currentTab != "Detected")
				{
					ImGui::Animate(198, 247, 200, &Globals::detectedTabSelectedPos);
				}
			}

			if (Globals::currentTab == "Detected")
			{
				ImGui::Animate(222, 255, 200, &Globals::detectedTabColor.x);
				ImGui::Animate(222, 255, 200, &Globals::detectedTabColor.y);
				ImGui::Animate(222, 255, 200, &Globals::detectedTabColor.z);

				ImGui::Animate(247, 198, 200, &Globals::detectedTabSelectedPos);
			}

			drawList->AddLine({ p.x - 2.0f, p.y + 198.0f }, { p.x + 92.0f, p.y + 198.0f }, IM_COL32(Globals::detectedTabColor.x, Globals::detectedTabColor.y, Globals::detectedTabColor.z, Globals::detectedTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 247.0f }, { p.x + 92.0f, p.y + 247.0f }, IM_COL32(Globals::detectedTabColor.x, Globals::detectedTabColor.y, Globals::detectedTabColor.z, Globals::detectedTabColor.w));
			drawList->AddLine({ p.x - 2.0f, p.y + 247.0f }, { p.x - 2.0f, p.y + Globals::detectedTabSelectedPos }, IM_COL32(Globals::detectedTabColor.x, Globals::detectedTabColor.y, Globals::detectedTabColor.z, Globals::detectedTabColor.w));
			drawList->AddText(imguiIo.Fonts->Fonts[2], 26.0f, { p.x - 2.0f, p.y + 207.0f }, IM_COL32(Globals::detectedTabColor.x, Globals::detectedTabColor.y, Globals::detectedTabColor.z, Globals::detectedTabColor.w), "Detected");

			if (Globals::currentTab == "Aiming")
			{
				ImVec2 windowPos{ ImGui::GetWindowPos() };
				drawList->AddRectFilled({ windowPos.x + 116, windowPos.y + 26 }, { windowPos.x + 374, windowPos.y + 284 }, IM_COL32(20, 20, 20, 200));

				ImGui::SetCursorPos({ 120, 30 });
				ImGui::BeginChild("AimbotChild", { 250, 250 });

				drawList->AddText(imguiIo.Fonts->Fonts[2], 14.0f, { windowPos.x + 311, windowPos.y + 30 }, IM_COL32_WHITE, "Aimbot");
				drawList->AddLine({ windowPos.x + 306, windowPos.y + 44 }, { windowPos.x + 354, windowPos.y + 44 }, IM_COL32(222, 222, 222, 255));

				ImGui::Toggle("Enabled", &Globals::aimbotEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Aimbot Toggle", &Globals::aimbotToggleEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Visual Check", &Globals::aimbotVisCheckEnabled, ImGuiToggleFlags_Animated);
				ImGui::Text("Body Part");
				if (ImGui::BeginCombo("##LockParts", Globals::aimbotLockPart.c_str()))
				{
					for (const std::string& part : aimbotLockParts)
					{
						bool selected{ Globals::aimbotLockPart == part };
						if (ImGui::Selectable(part.c_str(), selected))
						{
							Globals::aimbotLockPart = part;
						}

						if (selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Toggle("Visualise Target", &Globals::aimbotVisualiseTargetEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Target Tracer", &Globals::aimbotTargetTracerEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show FOV", &Globals::aimbotFOVEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Teamcheck", &Globals::aimbotTeamcheckEnabled, ImGuiToggleFlags_Animated);
				ImGui::Text("FOV Size");
				ImGui::SliderFloat("##FOV Size", &Globals::aimbotFOVSize, 1.0f, 1000.0f, "%.0f");
				ImGui::Text("Smoothing");
				ImGui::SliderFloat("##Aimbot Smoothing", &Globals::aimbotSmoothing, 1.0f, 30.0f, "%.2f");
				ImGui::Text("Keybind");
				ImGui::Hotkey(&Globals::aimbotKey);

				ImGui::EndChild();

				drawList->AddRectFilled({ windowPos.x + 386, windowPos.y + 26 }, { windowPos.x + 646, windowPos.y + 284 }, IM_COL32(20, 20, 20, 200));

				drawList->AddText(imguiIo.Fonts->Fonts[2], 14.0f, { windowPos.x + 556, windowPos.y + 30 }, IM_COL32_WHITE, "Triggerbot");
				drawList->AddLine({ windowPos.x + 552, windowPos.y + 44 }, { windowPos.x + 624, windowPos.y + 44 }, IM_COL32(222, 222, 222, 255));

				ImGui::SetCursorPos({ 390, 30 });
				ImGui::BeginChild("TriggerbotChild", { 250, 250 });

				ImGui::Toggle("Enabled", &Globals::triggerbotEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Teamcheck", &Globals::triggerbotTeamcheckEnabled, ImGuiToggleFlags_Animated);
				ImGui::Text("Keybind");
				ImGui::Hotkey(&Globals::triggerbotKey);

				ImGui::EndChild();

				drawList->AddRectFilled({ windowPos.x + 116, windowPos.y + 296 }, { windowPos.x + 374, windowPos.y + 542 }, IM_COL32(20, 20, 20, 200));

				drawList->AddText(imguiIo.Fonts->Fonts[2], 14.0f, { windowPos.x + 320, windowPos.y + 300 }, IM_COL32_WHITE, "RCS");
				drawList->AddLine({ windowPos.x + 317, windowPos.y + 314 }, { windowPos.x + 343, windowPos.y + 314 }, IM_COL32(222, 222, 222, 255));

				ImGui::SetCursorPos({ 120, 300 });
				ImGui::BeginChild("RCSChild", { 250, 250 });

				ImGui::Toggle("Enabled", &Globals::rcsEnabled, ImGuiToggleFlags_Animated);
				ImGui::Text("X");
				ImGui::SliderFloat("##RCSX", &Globals::rcsXY.x, 0.0f, 2.0f, "%.1f");
				ImGui::Text("Y");
				ImGui::SliderFloat("##RCSY", &Globals::rcsXY.y, 0.0, 2.0f, "%.1f");
				ImGui::Text("Shots Fired");
				ImGui::SliderInt("##Shots Fired", &Globals::rcsShotsFired, 0, 10);

				ImGui::EndChild();
			}
			else if (Globals::currentTab == "Visuals")
			{
				ImVec2 windowPos{ ImGui::GetWindowPos() };
				drawList->AddRectFilled({ windowPos.x + 116, windowPos.y + 26 }, { windowPos.x + 374, windowPos.y + 284 }, IM_COL32(20, 20, 20, 200));

				ImGui::SetCursorPos({ 120, 30 });
				ImGui::BeginChild("ESPChild", { 250, 250 }, 0);

				drawList->AddText(imguiIo.Fonts->Fonts[2], 14.0f, { windowPos.x + 320, windowPos.y + 30 }, IM_COL32_WHITE, "ESP");
				drawList->AddLine({ windowPos.x + 317, windowPos.y + 44 }, { windowPos.x + 343, windowPos.y + 44 }, IM_COL32(222, 222, 222, 255));

				ImGui::Toggle("Esp", &Globals::espEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Bounding Box", &Globals::espBoxEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Skeleton Esp", &Globals::espSkeletonEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Tracers", &Globals::espTracersEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Filled Esp", &Globals::espFilled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show Name", &Globals::espShowName, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show Health", &Globals::espShowHealth, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show Armor", &Globals::espShowArmor, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show Ammo", &Globals::espAmmoBarEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show Weapon", &Globals::espShowWeapon, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Show Eye Ray", &Globals::espEyeRay, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Teamcheck", &Globals::espTeamcheckEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Spotted", &Globals::espSpottedEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Visual Check", &Globals::espVisCheckEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Dynamic Color", &Globals::espDynamicColorEnabled, ImGuiToggleFlags_Animated);

				ImGui::Text("Box Esp Color");
				ImGui::ColorEdit4("##Box Esp Color", (float*)&Globals::espBoxColor);
				ImGui::Text("Skeleton Esp Color");
				ImGui::ColorEdit4("##Skeleton Esp Color", (float*)&Globals::espSkeletonColor);
				ImGui::Text("Tracers Color");
				ImGui::ColorEdit4("##Tracers Color", (float*)&Globals::espTracersColor);
				ImGui::Text("Spotted Color");
				ImGui::ColorEdit4("##Spotted Color", (float*)&Globals::espSpottedColor);
				ImGui::Text("Visual Check Color");
				ImGui::ColorEdit4("##VisCheck Color", (float*)&Globals::espVisCheckColor);

				ImGui::EndChild();

				drawList->AddRectFilled({ windowPos.x + 386, windowPos.y + 26 }, { windowPos.x + 646, windowPos.y + 284 }, IM_COL32(20, 20, 20, 200));

				drawList->AddText(imguiIo.Fonts->Fonts[2], 14.0f, { windowPos.x + 569, windowPos.y + 30 }, IM_COL32_WHITE, "World");
				drawList->AddLine({ windowPos.x + 565, windowPos.y + 44 }, { windowPos.x + 604, windowPos.y + 44 }, IM_COL32(222, 222, 222, 255));

				ImGui::SetCursorPos({ 390, 30 });
				ImGui::BeginChild("WorldESPChild", { 250, 250 });

				ImGui::Toggle("Bomb ESP", &Globals::wEspbombEspEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Weapon ESP", &Globals::wEspShowWeaponsEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Projectile ESP", &Globals::wEspShowProjectilesEnabled, ImGuiToggleFlags_Animated);

				ImGui::EndChild();
			}
			else if (Globals::currentTab == "Misc")
			{
				ImGui::SetCursorPos({ 120, 30 });
				ImGui::BeginChild("MiscChild", { 300, 500 });

				ImGui::Toggle("Spectator List", &Globals::spectatorListEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Sniper Crosshair", &Globals::sniperCrosshairEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Hit Marker", &Globals::hitMarkerEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Hit Sound", &Globals::hitSoundEnabled, ImGuiToggleFlags_Animated);
				if (ImGui::Button("Set Hit Sound"))
				{
					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = renderer.hwnd;
					ofn.lpstrFile = Globals::hitSoundPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"WAV Files (*.wav)\0*.wav\0";
					ofn.nFilterIndex = 1;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					GetOpenFileNameW(&ofn);
				}
				if (ImGui::Button("Set Visual Check Map"))
				{
					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = renderer.hwnd;
					ofn.lpstrFile = Globals::visCheckMapPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"OPT Files (*.opt)\0*.opt\0";
					ofn.nFilterIndex = 1;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if (GetOpenFileNameW(&ofn))
					{
						int size{ WideCharToMultiByte(CP_UTF8, 0, Globals::visCheckMapPath, -1, nullptr, 0, nullptr, nullptr) };
						std::string s(size, 0);
						WideCharToMultiByte(CP_UTF8, 0, Globals::visCheckMapPath, -1, &s[0], size, nullptr, nullptr);
						s.pop_back();

						vCheck = VisCheck(s);
					}
				}
				if (ImGui::Button("Save Config"))
				{
					wchar_t cfgPath[MAX_PATH];

					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = renderer.hwnd;
					ofn.lpstrFile = cfgPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"All Files\0*.*\0";
					ofn.nFilterIndex = 1;
					ofn.Flags = OFN_OVERWRITEPROMPT;

					if (GetOpenFileNameW(&ofn))
					{
						int size{ WideCharToMultiByte(CP_UTF8, 0, cfgPath, -1, nullptr, 0, nullptr, nullptr) };
						std::string s(size, 0);
						WideCharToMultiByte(CP_UTF8, 0, cfgPath, -1, &s[0], size, nullptr, nullptr);
						s.pop_back();

						nlohmann::json j;
						j["Aiming"]["Aimbot"]["Enabled"] = Globals::aimbotEnabled;
						j["Aiming"]["Aimbot"]["Toggle"] = Globals::aimbotToggleEnabled;
						j["Aiming"]["Aimbot"]["VisCheck"] = Globals::aimbotVisCheckEnabled;
						j["Aiming"]["Aimbot"]["LockPart"] = Globals::aimbotLockPart;
						j["Aiming"]["Aimbot"]["VisualiseTarget"] = Globals::aimbotVisualiseTargetEnabled;
						j["Aiming"]["Aimbot"]["TargetTracer"] = Globals::aimbotTargetTracerEnabled;
						j["Aiming"]["Aimbot"]["FOVEnabled"] = Globals::aimbotFOVEnabled;
						j["Aiming"]["Aimbot"]["TeamCheck"] = Globals::aimbotTeamcheckEnabled;
						j["Aiming"]["Aimbot"]["FOVSize"] = Globals::aimbotFOVSize;
						j["Aiming"]["Aimbot"]["Smoothing"] = Globals::aimbotSmoothing;
						j["Aiming"]["Aimbot"]["Keybind"] = Globals::aimbotKey;

						j["Aiming"]["Triggerbot"]["Enabled"] = Globals::triggerbotEnabled;
						j["Aiming"]["Triggerbot"]["TeamCheck"] = Globals::triggerbotTeamcheckEnabled;
						j["Aiming"]["Triggerbot"]["Keybind"] = Globals::triggerbotKey;

						j["Aiming"]["RCS"]["Enabled"] = Globals::rcsEnabled;
						j["Aiming"]["RCS"]["X"] = Globals::rcsXY.x;
						j["Aiming"]["RCS"]["Y"] = Globals::rcsXY.y;
						j["Aiming"]["RCS"]["ShotsFired"] = Globals::rcsShotsFired;

						j["Visuals"]["ESP"]["Enabled"] = Globals::espEnabled;
						j["Visuals"]["ESP"]["BoundingBox"] = Globals::espBoxEnabled;
						j["Visuals"]["ESP"]["Skeleton"] = Globals::espSkeletonEnabled;
						j["Visuals"]["ESP"]["Tracers"] = Globals::espTracersEnabled;
						j["Visuals"]["ESP"]["Filled"] = Globals::espFilled;
						j["Visuals"]["ESP"]["Name"] = Globals::espShowName;
						j["Visuals"]["ESP"]["Health"] = Globals::espShowHealth;
						j["Visuals"]["ESP"]["Armor"] = Globals::espShowArmor;
						j["Visuals"]["ESP"]["Weapon"] = Globals::espShowWeapon;
						j["Visuals"]["ESP"]["EyeRay"] = Globals::espEyeRay;
						j["Visuals"]["ESP"]["TeamCheck"] = Globals::espTeamcheckEnabled;
						j["Visuals"]["ESP"]["Spotted"] = Globals::espSpottedEnabled;
						j["Visuals"]["ESP"]["VisCheck"] = Globals::espVisCheckEnabled;
						j["Visuals"]["ESP"]["DynamicColor"] = Globals::espDynamicColorEnabled;
						j["Visuals"]["ESP"]["BoxColor"] = { Globals::espBoxColor.x, Globals::espBoxColor.y, Globals::espBoxColor.z, Globals::espBoxColor.w };
						j["Visuals"]["ESP"]["SkeletonColor"] = { Globals::espSkeletonColor.x, Globals::espSkeletonColor.y, Globals::espSkeletonColor.z, Globals::espSkeletonColor.w };
						j["Visuals"]["ESP"]["TracersColor"] = { Globals::espTracersColor.x, Globals::espTracersColor.y, Globals::espTracersColor.z, Globals::espTracersColor.w };
						j["Visuals"]["ESP"]["SpottedColor"] = { Globals::espSpottedColor.x, Globals::espSpottedColor.y, Globals::espSpottedColor.z, Globals::espSpottedColor.w };
						j["Visuals"]["ESP"]["VisCheckColor"] = { Globals::espVisCheckColor.x, Globals::espVisCheckColor.y, Globals::espVisCheckColor.z, Globals::espVisCheckColor.w };

						j["Visuals"]["World"]["BombESP"] = Globals::wEspbombEspEnabled;
						j["Visuals"]["World"]["DroppedWeaponsESP"] = Globals::wEspShowWeaponsEnabled;
						j["Visuals"]["World"]["ProjectileESP"] = Globals::wEspShowProjectilesEnabled;

						j["Misc"]["SpectatorList"] = Globals::spectatorListEnabled;
						j["Misc"]["SniperCrosshair"] = Globals::sniperCrosshairEnabled;
						j["Misc"]["HitMarker"] = Globals::hitMarkerEnabled;
						j["Misc"]["HitSoundEnabled"] = Globals::hitSoundEnabled;
						int hSize{ WideCharToMultiByte(CP_UTF8, 0, Globals::hitSoundPath, -1, nullptr, 0, nullptr, nullptr) };
						if (size > 0) {
							std::string hitSoundUtf8(hSize, 0);
							WideCharToMultiByte(CP_UTF8, 0, Globals::hitSoundPath, -1, &hitSoundUtf8[0], hSize, nullptr, nullptr);
							hitSoundUtf8.pop_back();

							j["Misc"]["HitSoundPath"] = hitSoundUtf8;
						}
						j["Misc"]["StreamProof"] = Globals::streamproofEnabled;

						std::ofstream oF(s);
						oF << j.dump(4);

						oF.close();
					}
				}
				if (ImGui::Button("Load Config"))
				{
					wchar_t cfgPath[MAX_PATH];

					OPENFILENAME ofn;
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = renderer.hwnd;
					ofn.lpstrFile = cfgPath;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = L"All Files\0*.*\0";
					ofn.nFilterIndex = 1;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					GetOpenFileNameW(&ofn);

					int size{ WideCharToMultiByte(CP_UTF8, 0, cfgPath, -1, nullptr, 0, nullptr, nullptr) };
					std::string s(size, 0);
					WideCharToMultiByte(CP_UTF8, 0, cfgPath, -1, &s[0], size, nullptr, nullptr);
					s.pop_back();

					std::ifstream iF(s);
					std::string cfgContent((std::istreambuf_iterator<char>(iF)), std::istreambuf_iterator<char>());

					nlohmann::json j{ nlohmann::json::parse(cfgContent) };
					Globals::aimbotEnabled = j["Aiming"]["Aimbot"]["Enabled"].get<bool>();
					Globals::aimbotToggleEnabled = j["Aiming"]["Aimbot"]["Toggle"].get<bool>();
					Globals::aimbotVisCheckEnabled = j["Aiming"]["Aimbot"]["VisCheck"].get<bool>();
					Globals::aimbotLockPart = j["Aiming"]["Aimbot"]["LockPart"].get<std::string>();
					Globals::aimbotVisualiseTargetEnabled = j["Aiming"]["Aimbot"]["VisualiseTarget"].get<bool>();
					Globals::aimbotTargetTracerEnabled = j["Aiming"]["Aimbot"]["TargetTracer"].get<bool>();
					Globals::aimbotFOVEnabled = j["Aiming"]["Aimbot"]["FOVEnabled"].get<bool>();
					Globals::aimbotTeamcheckEnabled = j["Aiming"]["Aimbot"]["TeamCheck"].get<bool>();
					Globals::aimbotFOVSize = j["Aiming"]["Aimbot"]["FOVSize"].get<float>();
					Globals::aimbotSmoothing = j["Aiming"]["Aimbot"]["Smoothing"].get<float>();
					Globals::aimbotKey = j["Aiming"]["Aimbot"]["Keybind"].get<int>();

					Globals::triggerbotEnabled = j["Aiming"]["Triggerbot"]["Enabled"].get<bool>();
					Globals::triggerbotTeamcheckEnabled = j["Aiming"]["Triggerbot"]["TeamCheck"].get<bool>();
					Globals::triggerbotKey = j["Aiming"]["Triggerbot"]["Keybind"].get<int>();

					Globals::rcsEnabled = j["Aiming"]["RCS"]["Enabled"].get<bool>();
					Globals::rcsXY.x = j["Aiming"]["RCS"]["X"].get<float>();
					Globals::rcsXY.y = j["Aiming"]["RCS"]["Y"].get<float>();
					Globals::rcsShotsFired = j["Aiming"]["RCS"]["ShotsFired"].get<int>();

					Globals::espEnabled = j["Visuals"]["ESP"]["Enabled"].get<bool>();
					Globals::espBoxEnabled = j["Visuals"]["ESP"]["BoundingBox"].get<bool>();
					Globals::espSkeletonEnabled = j["Visuals"]["ESP"]["Skeleton"].get<bool>();
					Globals::espTracersEnabled = j["Visuals"]["ESP"]["Tracers"].get<bool>();
					Globals::espFilled = j["Visuals"]["ESP"]["Filled"].get<bool>();
					Globals::espShowName = j["Visuals"]["ESP"]["Name"].get<bool>();
					Globals::espShowHealth = j["Visuals"]["ESP"]["Health"].get<bool>();
					Globals::espShowArmor = j["Visuals"]["ESP"]["Armor"].get<bool>();
					Globals::espShowWeapon = j["Visuals"]["ESP"]["Weapon"].get<bool>();
					Globals::espEyeRay = j["Visuals"]["ESP"]["EyeRay"].get<bool>();
					Globals::espTeamcheckEnabled = j["Visuals"]["ESP"]["TeamCheck"].get<bool>();
					Globals::espSpottedEnabled = j["Visuals"]["ESP"]["Spotted"].get<bool>();
					Globals::espVisCheckEnabled = j["Visuals"]["ESP"]["VisCheck"].get<bool>();
					Globals::espDynamicColorEnabled = j["Visuals"]["ESP"]["DynamicColor"].get<bool>();
					Globals::espBoxColor = {
						j["Visuals"]["ESP"]["BoxColor"][0].get<float>(),
						j["Visuals"]["ESP"]["BoxColor"][1].get<float>(),
						j["Visuals"]["ESP"]["BoxColor"][2].get<float>(),
						j["Visuals"]["ESP"]["BoxColor"][3].get<float>()
					};
					Globals::espSkeletonColor = {
						j["Visuals"]["ESP"]["SkeletonColor"][0].get<float>(),
						j["Visuals"]["ESP"]["SkeletonColor"][1].get<float>(),
						j["Visuals"]["ESP"]["SkeletonColor"][2].get<float>(),
						j["Visuals"]["ESP"]["SkeletonColor"][3].get<float>()
					};
					Globals::espTracersColor = {
						j["Visuals"]["ESP"]["TracersColor"][0].get<float>(),
						j["Visuals"]["ESP"]["TracersColor"][1].get<float>(),
						j["Visuals"]["ESP"]["TracersColor"][2].get<float>(),
						j["Visuals"]["ESP"]["TracersColor"][3].get<float>()
					};
					Globals::espSpottedColor = {
						j["Visuals"]["ESP"]["SpottedColor"][0].get<float>(),
						j["Visuals"]["ESP"]["SpottedColor"][1].get<float>(),
						j["Visuals"]["ESP"]["SpottedColor"][2].get<float>(),
						j["Visuals"]["ESP"]["SpottedColor"][3].get<float>()
					};
					Globals::espVisCheckColor = {
						j["Visuals"]["ESP"]["VisCheckColor"][0].get<float>(),
						j["Visuals"]["ESP"]["VisCheckColor"][1].get<float>(),
						j["Visuals"]["ESP"]["VisCheckColor"][2].get<float>(),
						j["Visuals"]["ESP"]["VisCheckColor"][3].get<float>()
					};

					Globals::wEspbombEspEnabled = j["Visuals"]["World"]["BombESP"].get<bool>();
					Globals::wEspShowWeaponsEnabled = j["Visuals"]["World"]["DroppedWeaponsESP"].get<bool>();
					Globals::wEspShowProjectilesEnabled = j["Visuals"]["World"]["ProjectileESP"].get<bool>();

					Globals::spectatorListEnabled = j["Misc"]["SpectatorList"].get<bool>();
					Globals::sniperCrosshairEnabled = j["Misc"]["SniperCrosshair"].get<bool>();
					Globals::hitMarkerEnabled = j["Misc"]["HitMarker"].get<bool>();
					Globals::hitSoundEnabled = j["Misc"]["HitSoundEnabled"].get<bool>();
					std::string hitSoundUtf8 = j["Misc"]["HitSoundPath"].get<std::string>();
					MultiByteToWideChar(CP_UTF8, 0, hitSoundUtf8.c_str(), -1, Globals::hitSoundPath, MAX_PATH);
					Globals::streamproofEnabled = j["Misc"]["StreamProof"].get<bool>();
				}
				if (ImGui::Toggle("Stream Proof", &Globals::streamproofEnabled, ImGuiToggleFlags_Animated))
				{
					if (Globals::streamproofEnabled)
					{
						SetWindowDisplayAffinity(renderer.hwnd, WDA_EXCLUDEFROMCAPTURE);
					}
					else
					{
						SetWindowDisplayAffinity(renderer.hwnd, WDA_NONE);
					}
				}

				ImGui::EndChild();
			}
			else if (Globals::currentTab == "Detected")
			{
				ImGui::SetCursorPos({ 120, 30 });
				ImGui::BeginChild("DetectedChild", { 300, 500 });

				ImGui::Toggle("Anti Flash", &Globals::antiFlashEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Bunny Hop", &Globals::bHopEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Radar", &Globals::radarEnabled, ImGuiToggleFlags_Animated);
				ImGui::Toggle("Player Glow", &Globals::playerGlowEnabled, ImGuiToggleFlags_Animated);

				ImGui::EndChild();
			}

			ImGui::End();
		}
		else
		{
			SetWindowLong(renderer.hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED);
		}

		ImDrawList* drawList{ ImGui::GetBackgroundDrawList() };

		uintptr_t playerPawnAddress{ cs2.read<uintptr_t>(client + Offsets::dwLocalPlayerPawn) };
		Entity localPlayerEntity{ cs2, playerPawnAddress };
		uintptr_t entityList{ cs2.read<uintptr_t>(client + Offsets::dwEntityList) };

		Matrix viewMatrix{ cs2.read<Matrix>(client + Offsets::dwViewMatrix) };
		int entityIndex{ cs2.read<int>(playerPawnAddress + Offsets::m_iIDEntIndex) };

		if (Globals::aimbotFOVEnabled)
		{
			drawList->AddCircle({ (float)monitorWidth / 2, (float)monitorHeight / 2 }, Globals::aimbotFOVSize, IM_COL32(255, 0, 0, 255));
		}

		if (Globals::rcsEnabled)
		{
			if (localPlayerEntity.shotsFired > Globals::rcsShotsFired)
			{
				Vector2 viewAngles{ cs2.read<Vector2>(playerPawnAddress + Offsets::m_angEyeAngles) };
				Vector2 delta{ viewAngles - (viewAngles + (rcsOldPunch - (localPlayerEntity.aimPunchAngle * 2.0f))) };

				float sensitivity{ cs2.read<float>(cs2.read<uintptr_t>(client + Offsets::dwSensitivity) + Offsets::dwSensitivity_sensitivity) };

				int x{ static_cast<int>(std::round((delta.y * Globals::rcsXY.x / sensitivity) / 0.044f)) };
				int y{ static_cast<int>(std::round((delta.x * Globals::rcsXY.y / sensitivity) / 0.044f)) };

				if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
				{
					NtUserInjectMouseInput::NTInjectInput(x, -y);

					rcsOldPunch = localPlayerEntity.aimPunchAngle * 2.0f;
				}
			}
			else
			{
				rcsOldPunch = { 0, 0 };
			}
		}

		for (int i{ 0 }; i < 64; i++)
		{
			uintptr_t listEntry{ cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16) };

			if (!listEntry)
			{
				continue;
			}

			uintptr_t currentController{ cs2.read<uintptr_t>(listEntry + 112 * (i & 0x1FF)) };

			if (!currentController)
			{
				continue;
			}

			int pawnHandle{ cs2.read<int>(currentController + Offsets::m_hPlayerPawn) };

			if (pawnHandle == 0)
			{
				continue;
			}

			uintptr_t listEntry2{ cs2.read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16) };
			uintptr_t currentPawn{ cs2.read<uintptr_t>(listEntry2 + 112 * (pawnHandle & 0x1FF)) };

			Entity entity{ cs2, currentPawn, currentController };

			if (Globals::radarEnabled)
			{
				cs2.write<bool>(currentPawn + Offsets::m_entitySpottedState + Offsets::m_bSpotted, true);
			}

			if (Globals::playerGlowEnabled)
			{
				if (currentPawn != playerPawnAddress)
				{
					cs2.write<unsigned int>(currentPawn + Offsets::m_Glow + Offsets::m_glowColorOverride, 0x800000FF); // ARGB
					cs2.write<int>(currentPawn + Offsets::m_Glow + Offsets::m_bGlowing, 1);
				}
			}

			if (Globals::spectatorListEnabled)
			{
				int ph{ cs2.read<int>(currentController + Offsets::m_hPawn) };
				uintptr_t e{ cs2.read<uintptr_t>(entityList + 0x8 * ((ph & 0x7FFF) >> 9) + 16) };
				uintptr_t pawn{ cs2.read<uintptr_t>(e + 112 * (ph & 0x1FF)) };

				uintptr_t observerServices{ cs2.read<uintptr_t>(pawn + Offsets::m_pObserverServices) };
				int observerTarget{ cs2.read<int>(observerServices + Offsets::m_hObserverTarget) };
				uintptr_t entry{ cs2.read<uintptr_t>(entityList + 0x8 * ((observerTarget & 0x7FFF) >> 9) + 16) };
				uintptr_t observerTargetPawn{ cs2.read<uintptr_t>(entry + 112 * (observerTarget & 0x1FF)) };

				if (observerTargetPawn == playerPawnAddress)
				{
					spectatingPlrs.push_back(entity);
				}
			}

			if (Globals::aimbotEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
			{
				bool check{ true };

				if (currentPawn == playerPawnAddress)
				{
					check = false;
				}

				if (entity.health <= 0 && check == true)
				{
					check = false;
				}

				if (Globals::aimbotTeamcheckEnabled)
				{
					if (localPlayerEntity.team == entity.team && check == true)
					{
						check = false;
					}
				}

				if (check == true)
				{
					aimbotPlrList.push_back(entity);
				}
			}

			if (Globals::triggerbotEnabled)
			{
				bool keyDown{ static_cast<bool>(Globals::triggerbotKey != 0 && GetAsyncKeyState(Globals::triggerbotKey) & 0x8000) };

				if (keyDown)
				{
					if (entityIndex > 0)
					{
						uintptr_t entry{ cs2.read<uintptr_t>(entityList + 0x8 * ((entityIndex & 0x7FFF) >> 9) + 16) };
						uintptr_t pawn{ cs2.read<uintptr_t>(listEntry2 + 112 * (entityIndex & 0x1FF)) };
						Entity pawnEntity{ cs2, pawn };

						if (Globals::triggerbotTeamcheckEnabled)
						{
							if (pawnEntity.team != localPlayerEntity.team && pawnEntity.health > 0)
							{
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
						else
						{
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

			if (Globals::espEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
			{
				bool check{ true };

				if (currentPawn == playerPawnAddress)
				{
					check = false;
				}

				if (entity.health <= 0)
				{
					check = false;
				}

				if (entity.position.x == 0.0f && entity.position.y == 0.0f && entity.position.z == 0.0f)
				{
					check = false;
				}

				if (Globals::espTeamcheckEnabled)
				{
					if (localPlayerEntity.team == entity.team)
					{
						check = false;
					}
				}

				if (check)
				{
					Vector3 headPos;
					headPos.x = entity.position.x;
					headPos.y = entity.position.y;
					headPos.z = entity.position.z + 72.0f;

					Vector2 screenPos{ WorldToScreen(entity.position, viewMatrix) };
					Vector2 headScreenPos{ WorldToScreen(headPos, viewMatrix) };

					if (Globals::espSkeletonEnabled)
					{
						uintptr_t boneMatrix{ getBoneMatrix(cs2, currentPawn) };

						// Define a espessura da linha (ajuste conforme preferir, 1.0f ou 1.5f)
						float thickness = 1.5f;

						for (Vector2 connection : boneConnections)
						{
							int bone1{ (int)connection.x };
							int bone2{ (int)connection.y };

							Vector3 boneVector1{ cs2.read<Vector3>(boneMatrix + bone1 * 32) };
							Vector3 boneVector2{ cs2.read<Vector3>(boneMatrix + bone2 * 32) };

							// Verifica se a entidade está na tela
							if (screenPos.x != 0.0f && screenPos.y != 0.0f)
							{
								Vector2 b1{ WorldToScreen(boneVector1, viewMatrix) };
								Vector2 b2{ WorldToScreen(boneVector2, viewMatrix) };

								// Verifica se ambos os ossos da conexão estão visíveis
								if ((b1.x != 0.0f && b1.y != 0.0f) && (b2.x != 0.0f && b2.y != 0.0f))
								{
									ImU32 skeletonColor;

									// Lógica de Cor Dinâmica (Respeitando seu sistema de cores)
									if (Globals::espDynamicColorEnabled)
									{
										skeletonColor = (entity.team == 2) ? terroristColor : counterTerroristColor;
									}
									else
									{
										skeletonColor = ImColor(Globals::espSkeletonColor);
									}

									// Desenha a linha do osso
									drawList->AddLine({ b1.x, b1.y }, { b2.x, b2.y }, skeletonColor, thickness);
								}
							}
						}
					}

					if ((screenPos.x != 0.0f && screenPos.y != 0.0f) && (headScreenPos.x != 0.0f && headScreenPos.y != 0.0f))
					{
						float boxHeight{ std::fabs(headScreenPos.y - screenPos.y) };
						float boxWidth{ boxHeight * 0.6f };

						Vector2 topLeft{ screenPos.x - boxWidth / 2, headScreenPos.y };
						Vector2 topRight{ screenPos.x + boxWidth / 2 + 12.0f, headScreenPos.y };
						Vector2 bottomRight{ screenPos.x + boxWidth / 2, screenPos.y };

						if (Globals::espBoxEnabled)
						{
							// --- 1. LÓGICA DE DEFINIÇÃO DA COR (PROCESSAMENTO) ---
							ImColor finalColor;

							// Se a cor dinâmica estiver ligada, decide entre Terrorista ou CT
							if (Globals::espDynamicColorEnabled) {
								finalColor = (entity.team == 2) ? terroristColor : counterTerroristColor;
							}
							else {
								finalColor = ImColor(Globals::espBoxColor);
							}

							// Lógica de Visibilidade (VisCheck)
							if (Globals::espVisCheckEnabled) {
								uintptr_t boneMatrix{ getBoneMatrix(cs2, currentPawn) };
								uintptr_t boneMatrix2{ getBoneMatrix(cs2, playerPawnAddress) };
								Vector3 enemyPos1{ cs2.read<Vector3>(boneMatrix + (BoneIndex::HEAD * 0x20)) };
								Vector3 enemyPos2{ cs2.read<Vector3>(boneMatrix + (BoneIndex::CHEST * 0x20)) };
								Vector3 localPos1{ cs2.read<Vector3>(boneMatrix2 + (BoneIndex::HEAD * 0x20)) };
								Vector3 localPos2{ cs2.read<Vector3>(boneMatrix2 + (BoneIndex::CHEST * 0x20)) };

								if (vCheck.IsPointVisible(enemyPos1, localPos1) || vCheck.IsPointVisible(enemyPos2, localPos2)) {
									finalColor = ImColor(Globals::espVisCheckColor);
								}
								else if (Globals::espSpottedEnabled && entity.spotted) {
									finalColor = ImColor(Globals::espSpottedColor);
								}
							}
							// Se VisCheck estiver desligado mas Spotted estiver ligado
							else if (Globals::espSpottedEnabled && entity.spotted) {
								finalColor = ImColor(Globals::espSpottedColor);
							}

							// --- 2. O SEU VISUAL DE BOX (DESENHO) ---

							// Desenha uma borda preta fina (Outline) para destacar a box em qualquer mapa
							drawList->AddRect({ topLeft.x - 1, topLeft.y - 1 }, { bottomRight.x + 1, bottomRight.y + 1 }, IM_COL32(0, 0, 0, 255));
							drawList->AddRect({ topLeft.x + 1, topLeft.y + 1 }, { bottomRight.x - 1, bottomRight.y - 1 }, IM_COL32(0, 0, 0, 255));

							// Desenha a Box Principal com a cor calculada
							drawList->AddRect({ topLeft.x, topLeft.y }, { bottomRight.x, bottomRight.y }, finalColor);

							// Se o preenchimento (Filled) estiver ligado
							if (Globals::espFilled)
							{
								// Pega a cor atual e aplica 50% de transparência (0.5f)
								ImColor filledColor = finalColor;
								filledColor.Value.w = 0.5f;
								drawList->AddRectFilled({ topLeft.x, topLeft.y }, { bottomRight.x, bottomRight.y }, filledColor);
							}
						}					

						// if (Globals::espAmmoBarEnabled)
						


						if (Globals::espShowArmor) {
							float armorValue = (float)entity.armor;
							float armorPerc = armorValue / 100.0f;

							if (armorPerc > 1.0f) armorPerc = 1.0f;
							if (armorPerc < 0.0f) armorPerc = 0.0f;

							// --- SEUS VISUAIS (AJUSTE MILIMÉTRICO) ---
							float compParaCima = 1.1f;  // Igual ao HP
							float compParaBaixo = 1.0f; // Igual ao HP

							float barWidth = 4.0f;
							float gap = 2.0f; // Distância do lado direito da box

							// --- APLICAÇÃO DA ALTURA ---
							float topY = topLeft.y - compParaCima;
							float bottomY = bottomRight.y + compParaBaixo;
							float fullHeight = bottomY - topY;

							// --- POSIÇÃO HORIZONTAL (CANTO DIREITO) ---
							float barLeftX = bottomRight.x + gap;
							float barRightX = bottomRight.x + gap + barWidth;

							// 1. Fundo (Background)
							drawList->AddRectFilled({ barLeftX, topY }, { barRightX, bottomY }, IM_COL32(0, 0, 0, 255));

							// 2. Preenchimento do Colete (Altura proporcional)
							float armorVisualHeight = fullHeight * armorPerc;
							ImVec2 armorTop = { barLeftX, bottomY - armorVisualHeight };
							ImVec2 armorBottom = { barRightX, bottomY };

							// 3. Cor (Azul que você já estava usando)
							ImU32 armorCol = IM_COL32(91, 135, 255, 255);

							if (armorValue > 0) {
								drawList->AddRectFilled(armorTop, armorBottom, armorCol);
							}

							// 4. Borda (Outline preta para o seu visual ficar nítido)
							drawList->AddRect({ barLeftX, topY }, { barRightX, bottomY }, IM_COL32(0, 0, 0, 255));
						}

						if (Globals::espShowWeapon)
						{
							std::string weaponIcon{ getWeaponIcon(entity.weaponName) };
							WeaponIconSize iconSize{ weaponIconSizes[entity.weaponName] };
							ImVec2 textPos{ screenPos.x - iconSize.width / 2 + iconSize.offsetX, screenPos.y + 1 + iconSize.offsetY };

							if (entity.weaponName == "hegrenade" || entity.weaponName == "flashbang" || entity.weaponName == "smokegrenade" || entity.weaponName == "decoy" || entity.weaponName == "incgrenade" || entity.weaponName == "molotov")
							{
								ImGui::GetBackgroundDrawList()->AddText(imguiIo.Fonts->Fonts[1], 10.0f, textPos, ImColor(255, 0, 0, 255), weaponIcon.c_str());
							}
							else
							{
								ImGui::GetBackgroundDrawList()->AddText(imguiIo.Fonts->Fonts[1], 10.0f, textPos, ImColor(255, 255, 255, 255), weaponIcon.c_str());
							}
						}

						if (Globals::espShowHealth) {
							float healthValue = (float)entity.health;
							float healthPerc = healthValue / 100.0f;

							if (healthPerc > 1.0f) healthPerc = 1.0f;
							if (healthPerc < 0.0f) healthPerc = 0.0f;

							// --- CONTROLE MANUAL DE COMPRIMENTO ---
							float compParaCima = 1.1f;  // Aumente para a barra subir mais que a box
							float compParaBaixo = 1.0f; // Aumente para a barra descer mais que a box

							float barWidth = 4.0f;
							float gap = 2.0f;

							// --- APLICAÇÃO DO COMPRIMENTO ---
							// Subtraímos no Y para subir, somamos no Y para descer
							float topY = topLeft.y - compParaCima;
							float bottomY = bottomRight.y + compParaBaixo;
							float fullHeight = bottomY - topY;

							// Posição horizontal
							float barLeftX = topLeft.x - gap - barWidth;
							float barRightX = topLeft.x - gap;

							// 1. Fundo (Background)
							drawList->AddRectFilled({ barLeftX, topY }, { barRightX, bottomY }, IM_COL32(0, 0, 0, 255));

							// 2. Preenchimento da Vida
							float healthVisualHeight = fullHeight * healthPerc;

							// A base da vida agora é o novo bottomY
							ImVec2 healthTop = { barLeftX, bottomY - healthVisualHeight };
							ImVec2 healthBottom = { barRightX, bottomY };

							// 3. Cor Dinâmica
							ImU32 healthCol = IM_COL32(255 * (1.0f - healthPerc), 255 * healthPerc, 0, 255);

							if (healthValue > 0) {
								drawList->AddRectFilled(healthTop, healthBottom, healthCol);
							}

							// 4. Borda (Outline)
							drawList->AddRect({ barLeftX, topY }, { barRightX, bottomY }, IM_COL32(0, 0, 0, 255));
						}

						if (Globals::espShowName)
						{
							// Ativa a Tahoma Bold
							ImGui::PushFont(tahomaBoldFont);

							ImVec2 textSize = ImGui::CalcTextSize(entity.name.c_str());
							float boxWidth = bottomRight.x - topLeft.x;
							float centerX = topLeft.x + (boxWidth / 2.0f) - (textSize.x / 2.0f);

							// Desenha Sombra (Preta)
							drawList->AddText({ centerX + 1, topLeft.y - textSize.y - 1.0f }, IM_COL32(0, 0, 0, 255), entity.name.c_str());
							// Desenha Nome (Branco)
							drawList->AddText({ centerX, topLeft.y - textSize.y - 2.0f }, IM_COL32_WHITE, entity.name.c_str());

							ImGui::PopFont();
						}


						if (Globals::espEyeRay)
						{
							uintptr_t boneMatrix{ getBoneMatrix(cs2, entity.currentPawn) };
							Vector3 headPos{ cs2.read<Vector3>(boneMatrix + (BoneIndex::HEAD * 0x20)) };

							Vector2 headScreenPos{ WorldToScreen(headPos, viewMatrix) };
							if (headScreenPos.x != 0.0f && headScreenPos.y != 0.0f)
							{
								Vector2 viewAngle{ cs2.read<Vector2>(entity.currentPawn + Offsets::m_angEyeAngles) };
								float viewAngleX{ static_cast<float>(viewAngle.x * (3.14159265358979323846 / 180.0f)) };
								float viewAngleY{ static_cast<float>(viewAngle.y * (3.14159265358979323846 / 180.0f)) };

								float lineLenght{ cos(viewAngleX) * 50.0f };

								Vector3 endPoint;
								endPoint.x = headPos.x + cos(viewAngleY) * lineLenght;
								endPoint.y = headPos.y + sin(viewAngleY) * lineLenght;
								endPoint.z = headPos.z - sin(viewAngleX) * 50.0f;

								Vector2 endPointScreen{ WorldToScreen(endPoint, viewMatrix) };

								if ((headScreenPos.x != 0.0f && headScreenPos.y != 0.0f) && (endPointScreen.x != 0.0f && endPointScreen.y != 0.0f))
								{
									if (Globals::espDynamicColorEnabled)
									{
										if (entity.team == 2)
										{
											drawList->AddLine({ headScreenPos.x, headScreenPos.y }, { endPointScreen.x, endPointScreen.y }, ImColor(terroristColor));
										}
										else if (entity.team == 3)
										{
											drawList->AddLine({ headScreenPos.x, headScreenPos.y }, { endPointScreen.x, endPointScreen.y }, ImColor(counterTerroristColor));
										}
									}
									else
									{
										drawList->AddLine({ headScreenPos.x, headScreenPos.y }, { endPointScreen.x, endPointScreen.y }, ImColor(Globals::espSkeletonColor));
									}
								}
							}
						}
					}

					if (Globals::espTracersEnabled)
					{
						if (screenPos.x != 0.0f && screenPos.y != 0.0f)
						{
							if (Globals::espDynamicColorEnabled)
							{
								if (entity.team == 2)
								{
									drawList->AddLine({ monitorWidth / 2.0f, (float)monitorHeight }, { screenPos.x, screenPos.y }, terroristColor);
								}
								else if (entity.team == 3)
								{
									drawList->AddLine({ monitorWidth / 2.0f, (float)monitorHeight }, { screenPos.x, screenPos.y }, counterTerroristColor);
								}
							}
							else
							{
								drawList->AddLine({ monitorWidth / 2.0f, (float)monitorHeight }, { screenPos.x, screenPos.y }, ImColor(Globals::espTracersColor));
							}
						}
					}
				}
			}
		}

		if (Globals::wEspbombEspEnabled)
		{
			uintptr_t gameRules{ cs2.read<uintptr_t>(client + Offsets::dwGameRules) };
			bool bombPlanted{ cs2.read<bool>(gameRules + Offsets::m_bBombPlanted) };

			if (!bombPlanted)
			{
				bombTimeLeft = 41;
				bombTimerStarted = false;
			}
			else
			{
				if (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2"))
				{
					uintptr_t plantedC4{ cs2.read<uintptr_t>(cs2.read<uintptr_t>(client + Offsets::dwPlantedC4)) };
					Vector3 bombPos{ cs2.read<Vector3>(cs2.read<uintptr_t>(plantedC4 + Offsets::m_pGameSceneNode) + Offsets::m_vecAbsOrigin) };
					Vector2 bombPosScreen{ WorldToScreen(bombPos, viewMatrix) };
					int bombPlantedSite{ cs2.read<int>(plantedC4 + Offsets::m_nBombSite) };

					drawList->AddText({ 2.0f, monitorHeight / 2.0f }, IM_COL32(0, 255, 0, 255), std::string("Bomb planted.\nSite: " + ((bombPlantedSite == 1) ? std::string("B") : std::string("A")) + "\nTime left: " + std::to_string(bombTimeLeft) + std::string("s")).c_str());
					ImGui::GetBackgroundDrawList()->AddText(imguiIo.Fonts->Fonts[1], 20.0f, { bombPosScreen.x, bombPosScreen.y }, ImColor(255, 255, 255, 255), "o");
				}

				bombTimerStarted = true;
			}

			if (bombTimerStarted && (GetTickCount64() - bombLastTick >= 1000))
			{
				bombTimeLeft--;
				bombLastTick = GetTickCount64();

				if (bombTimeLeft == 0)
				{
					bombTimeLeft = 0;
					bombTimerStarted = false;
				}
			}
		}

		if (Globals::sniperCrosshairEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
		{
			if ((localPlayerEntity.weaponName == "awp" || localPlayerEntity.weaponName == "ssg08" || localPlayerEntity.weaponName == "g3Sg1" || localPlayerEntity.weaponName == "scar20") && !localPlayerEntity.isScoped)
			{
				drawList->AddLine({ monitorWidth / 2.0f, (monitorHeight / 2.0f) + 6.0f }, { monitorWidth / 2.0f, (monitorHeight / 2.0f) - 5.0f }, IM_COL32(0, 255, 0, 255));
				drawList->AddLine({ (monitorWidth / 2.0f) - 5.0f, monitorHeight / 2.0f }, { (monitorWidth / 2.0f) + 6.0f, monitorHeight / 2.0f }, IM_COL32(0, 255, 0, 255));
			}
		}

		if (Globals::hitMarkerEnabled)
		{
			uintptr_t bulletServices{ cs2.read<uintptr_t>(playerPawnAddress + Offsets::m_pBulletServices) };
			int totalHits{ cs2.read<int>(bulletServices + Offsets::m_totalHitsOnServer) };

			if (totalHits != hitMarkerPrevTotalHits)
			{
				if (totalHits > hitMarkerPrevTotalHits)
				{
					hitMarkerAlpha = 1.0f;
					hitMarkerLastTick = GetTickCount64();
				}
			}
			else if (totalHits == 0 && hitMarkerPrevTotalHits != 0)
			{
				hitMarkerAlpha = 0.0f;
			}

			hitMarkerPrevTotalHits = totalHits;

			if (hitMarkerAlpha > 0.0f)
			{
				ULONGLONG currTicks{ GetTickCount64() };

				if (currTicks - hitMarkerLastTick >= 500.0f)
				{
					hitMarkerAlpha = 0.0f;
				}
				else
				{
					hitMarkerAlpha = 1.0f - ((float)(currTicks - hitMarkerLastTick) / 500.0f);
				}

				ImVec2 center{ monitorWidth / 2.0f, monitorHeight / 2.0f };

				drawList->AddLine({center.x - 10.0f, center.y - 10.0f }, { center.x - 3.0f, center.y - 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha) & IM_COL32_A_MASK, 2.4f);
				drawList->AddLine({ center.x - 10.0f, center.y + 10.0f }, { center.x - 3.0f, center.y + 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha) & IM_COL32_A_MASK, 2.4f);
				drawList->AddLine({ center.x + 10.0f, center.y - 10.0f }, { center.x + 3.0f, center.y - 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha) & IM_COL32_A_MASK, 2.4f);
				drawList->AddLine({ center.x + 10.0f, center.y + 10.0f }, { center.x + 3.0f, center.y + 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha) & IM_COL32_A_MASK, 2.4f);

				drawList->AddLine({ center.x - 10.0f, center.y - 10.0f }, { center.x - 3.0f, center.y - 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha), 1.4f);
				drawList->AddLine({ center.x - 10.0f, center.y + 10.0f }, { center.x - 3.0f, center.y + 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha), 1.4f);
				drawList->AddLine({ center.x + 10.0f, center.y - 10.0f }, { center.x + 3.0f, center.y - 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha), 1.4f);
				drawList->AddLine({ center.x + 10.0f, center.y + 10.0f }, { center.x + 3.0f, center.y + 3.0f }, ImColor(1.0f, 1.0f, 1.0f, hitMarkerAlpha), 1.4f);
			}
		}

		if (Globals::hitSoundEnabled)
		{
			uintptr_t bulletServices{ cs2.read<uintptr_t>(playerPawnAddress + Offsets::m_pBulletServices) };
			int totalHits{ cs2.read<int>(bulletServices + Offsets::m_totalHitsOnServer) };

			if (totalHits != hitSoundPrevTotalHits)
			{
				if (totalHits > hitSoundPrevTotalHits)
				{
					PlaySound(Globals::hitSoundPath, NULL, SND_FILENAME | SND_ASYNC);
				}
			}

			hitSoundPrevTotalHits = totalHits;
		}

		if (Globals::aimbotEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
		{
			Entity closestEntity{ cs2 };
			float closestDist{ Globals::aimbotFOVSize };

			if (Globals::aimbotToggleEnabled)
			{
				if (GetAsyncKeyState(Globals::aimbotKey) & 1)
				{
					Globals::aimbotToggled = !Globals::aimbotToggled;
				}
			}
			else
			{
				if (GetAsyncKeyState(Globals::aimbotKey) & 0x8000)
				{
					Globals::aimbotToggled = true;
				}
				else
				{
					Globals::aimbotToggled = false;
				}
			}

			Vector3 partPos{};
			Vector2 partScreenPos;

			if (Globals::aimbotToggled || Globals::aimbotVisualiseTargetEnabled || Globals::aimbotTargetTracerEnabled)
			{
				for (Entity entity : aimbotPlrList)
				{
					uintptr_t boneMatrix{ getBoneMatrix(cs2, entity.currentPawn) };
					Vector2 screenPos;
					if (Globals::aimbotLockPart == "Head")
					{
						screenPos = WorldToScreen(cs2.read<Vector3>(boneMatrix + (BoneIndex::HEAD * 0x20)), viewMatrix);
					}
					else if (Globals::aimbotLockPart == "Chest")
					{
						screenPos = WorldToScreen(cs2.read<Vector3>(boneMatrix + (BoneIndex::CHEST * 0x20)), viewMatrix);
					}
					else if (Globals::aimbotLockPart == "Left Arm")
					{
						screenPos = WorldToScreen(cs2.read<Vector3>(boneMatrix + (BoneIndex::LARM * 0x20)), viewMatrix);
					}
					else if (Globals::aimbotLockPart == "Right Arm")
					{
						screenPos = WorldToScreen(cs2.read<Vector3>(boneMatrix + (BoneIndex::RARM * 0x20)), viewMatrix);
					}
					else if (Globals::aimbotLockPart == "Left Leg")
					{
						screenPos = WorldToScreen(cs2.read<Vector3>(boneMatrix + (BoneIndex::LCALF * 0x20)), viewMatrix);
					}
					else if (Globals::aimbotLockPart == "Right Leg")
					{
						screenPos = WorldToScreen(cs2.read<Vector3>(boneMatrix + (BoneIndex::RCALF * 0x20)), viewMatrix);
					}
					float distance{ (float)sqrt(pow(screenPos.x - monitorWidth / 2, 2) + pow(screenPos.y - monitorHeight / 2, 2)) };

					if (distance < closestDist)
					{
						if (Globals::aimbotVisCheckEnabled)
						{
							uintptr_t boneMatrix{ getBoneMatrix(cs2, entity.currentPawn) };
							uintptr_t boneMatrix2{ getBoneMatrix(cs2, playerPawnAddress) };
							Vector3 enemyPos1{ cs2.read<Vector3>(boneMatrix + (BoneIndex::HEAD * 0x20)) };
							Vector3 enemyPos2{ cs2.read<Vector3>(boneMatrix + (BoneIndex::CHEST * 0x20)) };
							Vector3 localPos1{ cs2.read<Vector3>(boneMatrix2 + (BoneIndex::HEAD * 0x20)) };
							Vector3 localPos2{ cs2.read<Vector3>(boneMatrix2 + (BoneIndex::CHEST * 0x20)) };

							if (vCheck.IsPointVisible(enemyPos1, localPos1) || vCheck.IsPointVisible(enemyPos2, localPos2))
							{
								closestEntity = entity;
								closestDist = distance;
							}
						}
						else
						{
							closestEntity = entity;
							closestDist = distance;
						}
					}
				}

				uintptr_t boneMatrix{ getBoneMatrix(cs2, closestEntity.currentPawn) };

				if (Globals::aimbotLockPart == "Head")
				{
					partPos = cs2.read<Vector3>(boneMatrix + (BoneIndex::HEAD * 0x20));
				}
				else if (Globals::aimbotLockPart == "Chest")
				{
					partPos = cs2.read<Vector3>(boneMatrix + (BoneIndex::CHEST * 0x20));
				}
				else if (Globals::aimbotLockPart == "Left Arm")
				{
					partPos = cs2.read<Vector3>(boneMatrix + (BoneIndex::LARM * 0x20));
				}
				else if (Globals::aimbotLockPart == "Right Arm")
				{
					partPos = cs2.read<Vector3>(boneMatrix + (BoneIndex::RARM * 0x20));
				}
				else if (Globals::aimbotLockPart == "Left Leg")
				{
					partPos = cs2.read<Vector3>(boneMatrix + (BoneIndex::LCALF * 0x20));
				}
				else if (Globals::aimbotLockPart == "Right Leg")
				{
					partPos = cs2.read<Vector3>(boneMatrix + (BoneIndex::RCALF * 0x20));
				}

				partScreenPos = WorldToScreen(partPos, viewMatrix);
			}

			if (Globals::aimbotToggled)
			{
				if ((partScreenPos.x != 0.0f && partScreenPos.y != 0.0f) && (partPos.x != 0.0f && partPos.y != 0.0f && partPos.z != 0.0f))
				{
					POINT mousePos;
					GetCursorPos(&mousePos);

					float dx{ partScreenPos.x - mousePos.x };
					float dy{ partScreenPos.y - mousePos.y };

					static float accX{ 0 };
					static float accY{ 0 };

					accX += dx / Globals::aimbotSmoothing;
					accY += dy / Globals::aimbotSmoothing;

					int moveX{ (int)accX };
					int moveY{ (int)accY };

					accX -= moveX;
					accY -= moveY;

					NtUserInjectMouseInput::InjectMouseMoveRelative(moveX, moveY);
				}
			}

			if (Globals::aimbotVisualiseTargetEnabled)
			{
				if (partPos.x != 0.0f && partPos.y != 0.0f && partPos.z != 0.0f)
				{
					drawList->AddCircleFilled({ partScreenPos.x, partScreenPos.y }, 5.0f, IM_COL32(255, 0, 0, 255));
				}
			}

			if (Globals::aimbotTargetTracerEnabled)
			{
				if (partPos.x != 0.0f && partPos.y != 0.0f && partPos.z != 0.0f)
				{
					drawList->AddLine({ monitorWidth / 2.0f, monitorHeight / 2.0f }, { partScreenPos.x, partScreenPos.y }, IM_COL32(255, 0, 0, 255));
				}
			}

			aimbotPlrList.clear();
		}

		if (Globals::spectatorListEnabled)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0, 0, 0, 0 });

			ImGui::SetNextWindowSize({ 250, 200 });
			ImGui::Begin("Spectator List", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

			for (Entity entity : spectatingPlrs)
			{
				ImGui::Text(entity.name.c_str());
			}

			ImGui::End();

			ImGui::PopStyleColor();
		}

		if (Globals::antiFlashEnabled)
		{
			float flashDuration{ cs2.read<float>(playerPawnAddress + Offsets::m_flFlashBangTime) };

			if (flashDuration > 0.0f)
			{
				cs2.write<float>(playerPawnAddress + Offsets::m_flFlashBangTime, 0.0f);
			}
		}

		if (Globals::bHopEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
		{
			if (GetAsyncKeyState(VK_SPACE) & 0x8000)
			{
				bool onGround{ static_cast<bool>(cs2.read<int>(playerPawnAddress + Offsets::m_fFlags) & (int)(1 << 0)) };

				if (onGround)
				{
					cs2.write<int>(client + Offsets::jump, 65537);

					Sleep(1);

					cs2.write<int>(client + Offsets::jump, 256);
				}
			}
		}

		spectatingPlrs.clear();

		if (Globals::wEspShowWeaponsEnabled || Globals::wEspShowProjectilesEnabled)
		{
			for (int i{ 64 }; i < 2000; i++)
			{
				uintptr_t itemlistEntry{ cs2.read<uintptr_t>(entityList + 8 * ((i & 0x7FFF) >> 9) + 16) };

				if (!itemlistEntry)
				{
					continue;
				}

				uintptr_t itemEntity{ cs2.read<uintptr_t>(itemlistEntry + 112 * (i & 0x1FF)) };

				if (!itemEntity)
				{
					continue;
				}

				Vector3 itemPos{ cs2.read<Vector3>(cs2.read<uintptr_t>(itemEntity + Offsets::m_pGameSceneNode) + Offsets::m_vecAbsOrigin) };

				uintptr_t itemInfo{ cs2.read<uintptr_t>(itemEntity + 0x10) };
				uintptr_t itemType{ cs2.read<uintptr_t>(itemInfo + 0x20) };

				std::string type{ cs2.readstr(itemType) };

				if (Globals::wEspShowWeaponsEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
				{
					std::string weapon{ getWeaponType(type) };
					if (weapon != "unknown")
					{
						Vector2 sP{ WorldToScreen(itemPos, viewMatrix) };
						if (sP.x != 0.0f || sP.y != 0.0f)
						{
							drawList->AddText({ sP.x, sP.y }, IM_COL32_WHITE, weapon.c_str());
						}
					}
				}

				if (Globals::wEspShowProjectilesEnabled && (GetForegroundWindow() == FindWindowA(NULL, "Counter-Strike 2")))
				{
					std::string projectile{ getProjectileType(type) };
					if (projectile != "unknown")
					{
						Vector2 sP{ WorldToScreen(itemPos, viewMatrix) };
						if (sP.x != 0.0f || sP.y != 0.0f)
						{
							drawList->AddText({ sP.x, sP.y }, IM_COL32(255, 0, 0, 255), projectile.c_str());
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