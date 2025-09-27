#include "CrystalDynamicEngine.h"

CrystalDynamicEngine::CrystalDynamicEngine(HWND handle) :
	progmanHandle(FindWindow("Progman", 0)), widgetHandle(handle) {
}

void CrystalDynamicEngine::setWidget(HWND handle) {
	widgetHandle = handle;
}

void CrystalDynamicEngine::display() {
	SendMessageTimeout(progmanHandle, 0x52C, 0, 0, 0, 100, 0);
	SetParent(widgetHandle, progmanHandle);
	EnumWindows((WNDENUMPROC)&CrystalDynamicEngine::enumWindowProcess, 0);
	active = true;
}

void CrystalDynamicEngine::release() {
	SetParent(widgetHandle, 0);
	active = false;
}

void CrystalDynamicEngine::validationCheck() {
	if (GetWindow(progmanHandle, 5) == NULL) {
		SetParent(widgetHandle, progmanHandle);
	}
	EnumWindows((WNDENUMPROC)&CrystalDynamicEngine::enumWindowProcess, 0);
}

bool CrystalDynamicEngine::isActive() {
	return active;
}



int CrystalDynamicEngine::enumWindowProcess(HWND handle, LPARAM longParam) {
	HWND DefViewHandle = FindWindowEx(handle, 0, "SHELLDLL_DefView", 0);
	if (DefViewHandle != NULL) {
		HWND WorkerWHandle = FindWindowEx(0, handle, "WorkerW", 0);
		ShowWindow(WorkerWHandle, SW_HIDE);
		return 0;
	}
	return 1;
}
