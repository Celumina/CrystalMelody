#pragma once
#include <qglobal.h>

#ifdef Q_OS_WIN
#include <Windows.h>
class CrystalDynamicEngine {
public:
	CrystalDynamicEngine(HWND handle = nullptr);
	void setWidget(HWND handle);
	void display();
	void release();
	void validationCheck();
	bool isActive();

private:
	static int enumWindowProcess(HWND handle, LPARAM longParam);

	bool active = false;
	HWND progmanHandle = nullptr;
	HWND widgetHandle = nullptr;
};
#endif

