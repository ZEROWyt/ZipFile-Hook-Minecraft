#include <Windows.h>

#include <io.h>

#include <fstream>
#include <sstream>

#include <filesystem>
#include <iostream>
#include <string>

#include "xor.hpp"

#include "jdk/jni.h"
#include "jdk/jvmti.h"

#include "splice/MinHook.h"

#pragma comment(lib, "splice\\libMinHook-x64-v141-mt.lib")

#define Close TerminateProcess(GetCurrentProcess(), 0)

using namespace std;

typedef jlong (JNICALL* ZipOpen)(JNIEnv* env, jclass cls, jstring name, jint mode, jlong lastModified, jboolean usemmap);
ZipOpen ZipOpen_Original = NULL;

jlong JNICALL ZipOpen_Detour(JNIEnv* env, jclass cls, jstring name, jint mode, jlong lastModified, jboolean usemmap)
{
	const char* ModifyName = env->GetStringUTFChars(name, false);
	printf("ModifyName\n", ModifyName);
	if (strstr(ModifyName, xor ("minecraft")))
	{
		string jarFilePath = xor ("C:\\test\newminecraft.jar");

		name = env->NewStringUTF(jarFilePath.c_str());

		env->ReleaseStringUTFChars(name, ModifyName);

		return ZipOpen_Original(env, cls, name, mode, lastModified, usemmap);
	}

	env->ReleaseStringUTFChars(name, ModifyName);

	return ZipOpen_Original(env, cls, name, mode, lastModified, usemmap);
}

DWORD WINAPI Drista_OnLoad(LPVOID)
{
	if (MH_Initialize() != MH_OK)
	{
		MessageBox(NULL, xor ("Failed to initialize Trampoline API."), xor ("DullWave | Error"), MB_OK | MB_ICONERROR);
		Close;
	}

	HMODULE hZip = GetModuleHandleA(xor ("zip.dll"));

	if (!hZip)
		hZip = LoadLibraryA(xor ("zip.dll"));

	if (!hZip)
	{
		MessageBox(NULL, xor ("ZIP not found."), xor ("STERN | Error"), MB_OK | MB_ICONERROR);
		Close;
	}

	LPVOID ZFO = GetProcAddress(hZip, xor ("Java_java_util_zip_ZipFile_open"));

	if (MH_CreateHook(ZFO, &ZipOpen_Detour, reinterpret_cast<LPVOID*>(&ZipOpen_Original)) != MH_OK)
	{
		MessageBox(NULL, xor ("Failed to create trampoline."), xor ("STERN| Error"), MB_OK | MB_ICONERROR);
		Close;
	}

	if (MH_EnableHook(ZFO) != MH_OK)
	{
		MessageBox(NULL, xor ("Failed to enable trampoline."), xor ("STERN | Error"), MB_OK | MB_ICONERROR);
		Close;
	}

	Beep(1000, 200);
	ExitThread(0);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE) Drista_OnLoad, 0, 0, 0);

		return TRUE;
	}

	return FALSE;
}
