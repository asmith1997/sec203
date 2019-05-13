// dear imgui: standalone example application for DirectX 10
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
#include <winsock2.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx10.h"
#include <d3d10_1.h>
#include <d3d10.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <iostream>
#include <stdio.h>
#include <string>

#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <thread>

#include <windows.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <wincrypt.h>
#include <ntstatus.h>
#include <winnt.h>
#include <winternl.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "ws2_32.lib") // WinSock for posting our file to server...
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3dcompiler.lib")
// Data
static ID3D10Device* g_pd3dDevice = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D10RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MyKeyboardHook(int code, WPARAM wParam, LPARAM lParam);
void WriteToFile(DWORD vkCode, DWORD time, bool wasKeyUp);
static void initialize_hook_thread();
static void begin_file_transfer(std::string userName, std::string password);
bool hash_password(std::string pass, std::string* outPass);
bool authorize_user(SOCKET *connectionSocket, std::string userName, std::string password);
std::string readWholeFile();
DWORD lastKey = 0x0;
DWORD lastAction = 0x0;
DWORD* keyDuration = new DWORD[253];
std::string* stringStore = new std::string[253];
std::vector<std::string> bigVectors;
//FILE* fp;
std::fstream *fp;


bool FirstEntry = true;
DWORD prevKey = 0x0;
bool boxChange = false;
bool islogin = true;


bool isActiveWindow = false;

bool loggedIn = false;
static bool isLearn;
std::string BadCharacters = "!\"£$%^&*()_+-={}[]:;@'~#<,>.?/|\\+";
// SCAN CODE, TIMESTAMP, DURATION
// SCAN CODE IS KEY PRESSED
// TIMESTAMP IS WHEN KEY PRESSED
// DURATION IS HOW LONG IT WAS PRESSED FOR


#define API_HOST "82.47.162.246"

// Main code
int main(int, char**)
{
	// Create application window
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Cristallo"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Cristallo Login"), WS_OVERLAPPEDWINDOW, 100, 100, 600, 400, NULL, NULL, wc.hInstance, NULL);
	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}
	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX10_Init(g_pd3dDevice);
	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	std::string hostIP;
	std::string username;
	std::string password;

	HANDLE threadHandle = CreateThread(0, 0,
		(LPTHREAD_START_ROUTINE)& initialize_hook_thread,
		0, 0, 0);

	if (threadHandle == INVALID_HANDLE_VALUE)
	{
		printf("[!] - Thread error...[GLE]: 0x%08x\n", GetLastError());
		return 0;
	}

	DWORD thread_status;
	bool console_once = false;

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	HWND thishwnd = GetConsoleWindow();
	ShowWindow(thishwnd, SW_HIDE);
	
	while (msg.message != WM_QUIT)
	{

		
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		if (GetForegroundWindow() == hwnd)
		{
			isActiveWindow = true;
		}
		else
		{
			isActiveWindow = false;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX10_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		{
			static float f = 0.0f;
			static int counter = 0;
			static char ip[32];
			static char c_username[32];
			static char c_password[32];
			static bool console;
			

			ImGui::Begin("---Cristallo Window---", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("Type your details here, you can either use the mouse\nto navigate these boxes or the tab key\n");
			ImGui::Text("Pressing enter whilst the password field is active will\ninitiate the POST to the API");

			ImGui::Checkbox("Log in test mode?", &loggedIn);
			ImGui::Checkbox("Console Window?", &console);


			if (ImGui::InputText("User", c_username, IM_ARRAYSIZE(c_username), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				printf("[DEBUG]: Enter has been pressed in c_username\n");
				boxChange = true;
			}
			username.assign(c_username);
			
			if (ImGui::InputText("Pass", c_password, IM_ARRAYSIZE(c_password), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				printf("[DEBUG]: Enter has been pressed in c_password\n");
				islogin = true;
				boxChange = true;
				keybd_event(VK_RETURN, 0, 0, 0);
				keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
				std::thread fileTransferThread(begin_file_transfer, username, password);
				fileTransferThread.join();
				fp->close();
				strcpy_s(c_username, sizeof(""), "");
				strcpy_s(c_password, sizeof(""), "");
				//fp->open("data.csv", std::fstream::in | std::fstream::out | std::fstream::trunc);
				prevKey = 0;
				for (int x = 0; x <= sizeof(keyDuration); x++)
				{
					keyDuration[x] = 0;
				}
				FirstEntry = true;
			}
			password.assign(c_password);

			if (ImGui::Button("Reset"))
			{
				fp->close();
				// strcpy_s empty strings into our char buffers to clear form.
				strcpy_s(c_username, sizeof(""), "");
				strcpy_s(c_password, sizeof(""), "");

				// Re-open our file and truncate it, ready to start again.
				fp->open("data.csv", std::fstream::in | std::fstream::out | std::fstream::trunc);
				prevKey = 0;

				// Reset all of our keydurations stored on keypresses.
				for (int x = 0; x <= sizeof(keyDuration); x++)
				{
					keyDuration[x] = 0;
					stringStore[x].clear();
				}
				FirstEntry = true;
				islogin = true;
			}


			ImGui::SameLine();

			if (ImGui::Button("Create User"))
			{
				islogin = false;
				keybd_event(VK_RETURN, 0, 0, 0);
				keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
				std::thread fileTransferThread(begin_file_transfer, username, password);
				fileTransferThread.join();
				fp->close();
				// Reset our input buffers to blank strings.
				strcpy_s(c_username, sizeof(""), "");
				strcpy_s(c_password, sizeof(""), "");

				// Re-open our file and truncate it, ready to start again.
				fp->open("data.csv", std::fstream::in | std::fstream::out | std::fstream::trunc);
				prevKey = 0;

				for (int x = 0; x <= sizeof(keyDuration); x++)
				{
					keyDuration[x] = 0;
					stringStore[x].clear();
				}
				FirstEntry = true;
			}

			ImGui::SameLine();
			if (ImGui::Button("Exit"))
			{
				fp->flush();
				exit(0);
			}

			if (console)
			{
				if (!console_once)
				{
					HWND thishwnd = GetConsoleWindow();
					ShowWindow(thishwnd, SW_SHOW);
					console_once = true;
				}
			}
			else
			{
				HWND thishwnd = GetConsoleWindow();
				ShowWindow(thishwnd, SW_HIDE);
				console_once = false;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();


		}

		if(loggedIn)
		{
			ImGui::Begin("Logged in Window", &loggedIn);
			ImGui::Text("Congratulations, you have successfully logged\n in through our biometric authentication system\n");
			if (ImGui::Button("Logout")) // To logout, we just need to set the loggedIn bool to false.
			{
				loggedIn = false;
			}
			ImGui::End(); // End this Window instance
		}



		// Rendering
		ImGui::Render();
		g_pd3dDevice->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		g_pd3dDevice->ClearRenderTargetView(g_mainRenderTargetView, (float*)& clear_color);
		ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // Present with vsync
		//g_pSwapChain->Present(0, 0); // Present without vsync
	}

	ImGui_ImplDX10_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
	if (D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D10Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Win32 message handler
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}





// Cristallo API Functions
static void initialize_hook_thread()
{
	HHOOK myKbHook = NULL;

	//printf("[Thread]: 0x%08x started...\n", GetCurrentThreadId());

	fp = new std::fstream("data.csv", std::fstream::in | std::fstream::out | std::fstream::trunc);
	if (!fp->is_open())
	{
		printf("[!] - An error occured opening data.csv :(\n");
		FILE *file;
		fopen_s(&file, "data.csv", "w");
		if (file == nullptr)
		{
			printf("[!] - Failure to create data.csv, closing thread...\n");

			delete[] keyDuration;
			ExitThread(100);
			return;
		}
		else
		{
			fclose(file);
			printf("[+] - Closed creating file handle, opening again with fstream...[It's still working Jay]\n");
			fp->open("data.csv", std::fstream::in | std::fstream::out | std::fstream::trunc);
		}
	}

	printf("[+] - File open, creating hook\n");
	// Need to call SetWindowsHookEx with code of either 2 or 13.
	// * 13 is Lowlevel Keyboard hook
	// 2 is generic keyboard hook
	myKbHook = SetWindowsHookEx(WH_KEYBOARD_LL, MyKeyboardHook, NULL, 0);
	if (myKbHook == NULL)
	{
		printf("[!!] - There was an error starting SetWindowsHookEx\n");
		printf("[GLE]: 0x%08x\n", GetLastError());
		fp->close();
		delete[] keyDuration;
		ExitThread(100);
	}


	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{

	}

	fp->close();
	UnhookWindowsHookEx(myKbHook);
	delete[] keyDuration;
	ExitThread(100);


}

void WriteToFile(DWORD vkCode, DWORD time, bool wasKeyUp)
{
	if (!fp->is_open())
	{
		fp->open("data.csv", std::fstream::out);
	}
	UINT mapKey = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);
	if (mapKey == 0)
	{
		keyDuration[vkCode] = 0; // If it doesn't map to a character we don't care
		return;
	}

	switch (vkCode)
	{
	case VK_CONTROL:
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
	case VK_BACK:
	{
		// When this is found, we will delete the last line in the file.
		if (wasKeyUp)
		{
			if (bigVectors.size() == 0)
			{
				return;
			}
			bigVectors.erase(bigVectors.end() - 1);
		}
		return;
	}
	case VK_LWIN:
	case VK_RWIN:
	case VK_TAB:
	case VK_LMENU:
	case VK_RMENU:
		return;
	case VK_RETURN:
	{
		//printf("[DEBUG]: Enter has been pressed in VK_RETURN\n");
		if (!fp->is_open())
		{
			fp->open("data.csv", std::fstream::in | std::fstream::out);
			//printf("[DEBUG]: data.csv is not open in VK_RETURN case\n");
		}
		for (std::string x : bigVectors)
		{
			printf("[DEBUG]: Writing %s to file...\n", x.c_str());

			fp->write(x.c_str(), x.size());
		}

		fp->flush();
		bigVectors.clear();
		return;
	}
	default:
		break;

	}

	if (wasKeyUp == false) // Was it not a key up event?
	{
		if (BadCharacters.rfind((char)(char)mapKey) != std::string::npos) // is the character sent a punctuation character or anything?
			return;

		stringStore[vkCode] += std::to_string(mapKey);
		stringStore[vkCode] += ',';
		//_itoa_s((unsigned long)time, itoaOutput, sizeof(itoaOutput), 10);
		stringStore[vkCode] += std::to_string(time);
		stringStore[vkCode] += ',';
		//printf("[KEYSTORE %d] : %s\n", vkCode, stringStore[vkCode].c_str());

	}

	else if (wasKeyUp)
	{
		if (BadCharacters.rfind((char)(char)mapKey) != std::string::npos)
			return;


		// Our key was released, we now have the duration time by doing
		// time - keyDuration[vkCode];
		//_itoa_s(time - keyDuration[vkCode], itoaOutput, sizeof(itoaOutput), 10);

		// DWELL TIME

		stringStore[vkCode] += std::to_string(time - keyDuration[vkCode]); // We add our final pieces of data to our buffer within the following "if"
		if (FirstEntry)
		{
			FirstEntry = false;
			stringStore[vkCode] += ",0";
			prevKey = vkCode;
		}
		else
		{
			// Time released is 
			// TIME RELEASED: keyDuration[vkCode] + (time - keyDuration[vkCode])

			// Latency is
			// TimeReleased - keyDuration[lastKey]
			stringStore[vkCode] += ',';
			if (vkCode != prevKey)
			{
				stringStore[vkCode] += std::to_string((keyDuration[vkCode] + (time - keyDuration[vkCode])) - keyDuration[prevKey]);
			}
			else
			{
				stringStore[vkCode] += std::to_string((keyDuration[vkCode] + (time - keyDuration[vkCode])) - keyDuration[vkCode]);
			}
		}

		stringStore[vkCode] += '\n';
		//printf("[KEYSTORE %d] : %s\n", vkCode, stringStore[vkCode].c_str());
		if (fp->is_open())
		{
			//printf("[DEBUG]: %s\n", stringStore[vkCode].c_str());
			//fp->write(stringStore[vkCode].c_str(), stringStore[vkCode].size()); // Write it to the data.txt file.
			bigVectors.push_back(stringStore[vkCode].c_str());
			printf("[KEYSTORE]: %s", stringStore[vkCode].c_str());
			stringStore[vkCode].clear(); // Empty out our string buffer for a new character at that location.
		}
	}

}

LRESULT WINAPI MyKeyboardHook(int code, WPARAM wParam, LPARAM lParam)
{
	// Type cast WPARAM to tagKBDLLHOOKSTRUCT as it containers a pointer to this
	if (code < 0)
		return CallNextHookEx(NULL, code, wParam, lParam);

	if (!isActiveWindow)
	{
		printf("[DEBUG]: Not recording keystroke, window not focused...\n");
		return CallNextHookEx(NULL, code, wParam, lParam);
	}


	// We need to typecast lParam to a KBDLL struct, lParam contains a pointer to this
	tagKBDLLHOOKSTRUCT kbHook = *(tagKBDLLHOOKSTRUCT*)lParam;
	switch (wParam) // wParam is the Window Message.
	{
	case WM_SYSKEYUP:
	{
		break;
	}

	case WM_SYSKEYDOWN:
	{
		break;
	}

	case WM_KEYUP:
	{
		//printf("[KEYUP]: %x released...\n", kbHook.vkCode);
		//printf("%ul\n", kbHook.time);//, GetTickCount());
		WriteToFile(kbHook.vkCode, kbHook.time, true);
		lastAction = WM_KEYUP;
		prevKey = kbHook.vkCode;
		break;
	}

	case WM_KEYDOWN:
	{
		if (lastAction == WM_KEYDOWN && lastKey == kbHook.vkCode)
		{
			break;
		}

		lastKey = kbHook.vkCode;
		lastAction = WM_KEYDOWN;
		keyDuration[kbHook.vkCode] = kbHook.time;
		//printf("[KEYDOWN]: %c pressed...\n", kbHook.vkCode);
		//printf("%ul\n", kbHook.time);//, GetTickCount());
		WriteToFile(kbHook.vkCode, kbHook.time, false);
		break;
	}

	default:
	{
		break;
	}
	}

	return CallNextHookEx(NULL, code, wParam, lParam); // We have to call the next hook in sequence.
}

bool hash_password(std::string password, std::string * outPassword)
{
	NTSTATUS Status;
	BCRYPT_ALG_HANDLE AlgHandle = NULL;
	BCRYPT_HASH_HANDLE HashHandle = NULL;

	PBYTE Hash = NULL;
	DWORD HashLength = 0;
	DWORD ResultLength = 0;
	std::vector<unsigned char> hash;

	if (outPassword == nullptr)
	{
		printf("OutPassword == nullptr\n");
		outPassword = new std::string();
	}

	// Open a handle to the crypto service from Windows
	Status = BCryptOpenAlgorithmProvider(&AlgHandle, BCRYPT_SHA256_ALGORITHM,
		NULL, BCRYPT_HASH_REUSABLE_FLAG);
	if (!NT_SUCCESS(Status))
	{
		printf("[!] - Error opening CryptoAlgorithmProvider\n");
		return false;
	}

	Status = BCryptGetProperty(AlgHandle, BCRYPT_HASH_LENGTH, (PBYTE)& HashLength, sizeof(HashLength), &ResultLength, 0);
	if (!NT_SUCCESS(Status))
	{
		printf("[!] - Failure to get BCryptProperty\n");
		return false;
	}
	hash.resize(HashLength);


	// Allocate a space on the heap for our bytes. 
	Hash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, HashLength);
	if (Hash == NULL)
	{
		//Status = STATUS_NO_MEMORY;
		printf("[!] - No memory for HeapAllocation for Hash\n");
		return false;
	}

	// Create our hash, we also put a key in this function, but we're not using one.
	Status = BCryptCreateHash(AlgHandle, &HashHandle, NULL, 0, NULL, 0, 0);
	if (!NT_SUCCESS(Status))
	{
		printf("[!] - Failure to CryptCreateHash\n");
		return false;
	}
	
	// Actually hash our password
	Status = BCryptHashData(HashHandle,
		(PBYTE)password.c_str(), password.size(), 0);
	if (!NT_SUCCESS(Status))
	{
		printf("[!] - Failure to BCryptHashData\n");
		return false;
	}

	// Store our hash into our std::vector<>
	Status = BCryptFinishHash(HashHandle, hash.data(), HashLength, 0);
	if (!NT_SUCCESS(Status))
	{
		printf("[!] - Failure to BCryptFinishHash\n");
		return false;
	}

	HeapFree(GetProcessHeap(), 0, Hash);
	BCryptDestroyHash(HashHandle);
	BCryptCloseAlgorithmProvider(AlgHandle, 0);
	std::string hashPass(hash.begin(), hash.end());

	outPassword->clear();
	outPassword->assign(hashPass.c_str());
	printf("[PASSWORD]: %s\n", outPassword->c_str());
	return true;
}

bool authorize_user(SOCKET *connectionSocket, std::string userName, std::string password)
{
	if (connectionSocket == nullptr || *connectionSocket == INVALID_SOCKET)
	{
		printf("[!] - Cannot authorize user... Connection socket is bad.\n");
		return false;
	}



	std::string retFile = readWholeFile();
	//printf("\n\n\n\n%s\n\n\n\n", retFile.c_str());

	std::string postData; // Construct 
	postData = "username=" + userName;
	postData += "&passHash=" + password;


	std::string header; // Construct our raw http header
	std::string body;

	body += "--------------dataentry\r\n";
	body += "Content-Disposition: form-data; name=\"username\"\r\n";
	body += "\r\n";

	body += userName + "\r\n";

	body += "--------------dataentry\r\n";
	body += "Content-Disposition: form-data; name=\"passHash\"\r\n";
	body += "\r\n";

	body += password + "\r\n";
	if (islogin == false) // if it is not a login mode, i.e create user mode, finish our header here
	{
		body += "--------------dataentry--\r\n";
		body += "\r\n";
	}
	else if (islogin)
	{
		body += "--------------dataentry\r\n";
		body += "Content-Disposition: form-data; name=\"file\"; filename=\"file.csv\"\r\n";
		body += "Content-Type: text/csv\r\n";
		body += "\r\n";

		body += retFile + "\r\n";

		body += "--------------dataentry--\r\n";
		body += "\r\n";
	}

	if (islogin)
	{
		header = "POST /user/login HTTP/1.1\r\n";
	}
	else if (islogin == false)
	{
		header = "POST /user/create HTTP/1.1\r\n";
	}
	header += "Host: " + std::string(API_HOST) + ":5000\r\n";
	header += "Content-Type: multipart/form-data; boundary=------------dataentry\r\n";
	header += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
	header += body;
	//header += "\r\n";






	//printf("[DataLen]: %i\n[Header Len]: %i\n", dataSize, header.size());
	printf("[HEADER]:\n%s\n", header.c_str());
	//printf("[POSTDATA]: %s\n", postData.c_str());
	if (header.size() > 0)
	{
		if (send(*connectionSocket, header.c_str(), header.size(), 0) != SOCKET_ERROR)
		{
			/*if (send(*connectionSocket, body.c_str(), body.size(), 0) == SOCKET_ERROR)
			{
				printf("Error in sending the content body\n");
				return false;
			}*/
			// Received HTTP response buffer.
			char bigbuff[1024];
			ZeroMemory(&bigbuff, sizeof(bigbuff));
			printf("Waiting for data from server\n");
			if (recv(*connectionSocket, bigbuff, sizeof(bigbuff), 0) != SOCKET_ERROR)
			{
				//MessageBoxA(NULL, sRecv, 0, 0);			
				std::string responseData(bigbuff);
				printf("%s\n", responseData.c_str());
				if (responseData.find("200") != std::string::npos)
				{
					printf("[+] - Successfully authorized user\n");
					
					return true;
				}
				else if (responseData.find("201") != std::string::npos)
				{
					printf("[+] - Successfully created a new user\n");
					return false;
				}
				else if (responseData.find("401") != std::string::npos)
				{
					printf("[!] - Biometric rejection...\n");
					MessageBoxA(NULL, "Biometrically rejected. Try again if this is you", "Error", MB_ICONERROR | MB_OK);
					return false;
				}
				else if (responseData.find("402") != std::string::npos)
				{
					printf("[!] - Invalid credentials...\n");
					MessageBoxA(NULL, "Invalid credentials entered.", "Error", MB_ICONERROR | MB_OK);
					return false;
				}
				else
				{
					std::string errMsg;
					//printf("Error occured.\n[Response]: %s\n", responseData.c_str());
					errMsg = "Error occured.\n[Response]\n" + responseData;
					MessageBoxA(NULL, errMsg.c_str(), "Error", MB_ICONERROR | MB_OK);
					return false;
				}
			}
			else
			{
				printf("[!] - Failure to receive information from the server...\n");
				return false;
			}
		}
		else
		{
			printf("[!] - Failure to send information to the server...\n");
			return false;
		}
	}
	else
	{
		printf("[!] - Something went wrong with assigning the header?\n");
		return false;
	}

	return false;
}


static void begin_file_transfer(std::string userName, std::string password)
{

	std::string outPass; // Hash our password
	if (!hash_password(password, &outPass))
	{
		printf("[!] - Something went wrong in hashing password...\n");
		return;
	}

#pragma region WinSock

	WSADATA* wsaData = new WSADATA;
	struct addrinfo* result = NULL,
		*ptr = NULL,
		hints;
	SOCKET ConnectionSocket = INVALID_SOCKET;

	ZeroMemory(wsaData, sizeof(wsaData));
	int init_result = WSAStartup(MAKEWORD(2, 2), wsaData);
	if (init_result != 0)
	{
		printf("WSAStartup failed... File transfer cancelled!\n");
		return;
	}


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Change the IP and port here accordingly
	init_result = getaddrinfo(API_HOST, "5000", &hints, &result);
	if (init_result != 0)
	{
		printf("getaddrinfo failed!\n[GLE]: 0x%08x\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	ptr = result;
	ConnectionSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectionSocket == INVALID_SOCKET)
	{
		printf("ConnectionSocket is INVALID_SOCKET...\n[GLE]: 0x%08x\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	init_result = connect(ConnectionSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (init_result == SOCKET_ERROR)
	{
		printf("Failed to connect to that address...\n[GLE]: 0x%08x\n", WSAGetLastError());
		closesocket(ConnectionSocket);
		ConnectionSocket = INVALID_SOCKET;
		WSACleanup();
		return;
	}
#pragma endregion



	if (!(authorize_user(&ConnectionSocket, userName, outPass)))
	{
		printf("Bad authentication details...\n");
		//MessageBoxA(NULL, "Login unsuccessful\n", "Error", MB_ICONERROR | MB_OK);
		closesocket(ConnectionSocket);
		ConnectionSocket = INVALID_SOCKET;
		WSACleanup();
		return;
	}
	else
	{
		loggedIn = true;
	}


}

std::string readWholeFile()
{
	std::string wholeFile;
	fp->flush(); // Synchronize data to the file.
	std::ifstream sendFile("data.csv", std::ifstream::in);
	//printf("0x%08x\n", (DWORD&)sendFile);
	if (sendFile)
	{
		sendFile.seekg(0, std::ifstream::beg);
		char* buffer = new char[1024];
		ZeroMemory(buffer, 1024);

		while (!sendFile.eof())
		{
			sendFile.read(buffer, sizeof(buffer));
			//printf("[Buffer]: %s\n", buffer);
			wholeFile += buffer;

			//printf("%s\n", buffer);
			ZeroMemory(buffer, 1024);
		}

		delete[] buffer;
	}
	sendFile.close();
	return wholeFile;
}