#include "stdafx.h"
#include "Desktop_Monitor.h"
#include <fstream>

RemoteDesktop::DesktopMonitor::DesktopMonitor(){

	m_hCurWinsta = GetProcessWindowStation();

	m_hWinsta = OpenWindowStation(L"winsta0", false,
		WINSTA_ENUMDESKTOPS |
		WINSTA_READATTRIBUTES |
		WINSTA_ACCESSCLIPBOARD |
		WINSTA_CREATEDESKTOP |
		WINSTA_WRITEATTRIBUTES |
		WINSTA_ACCESSGLOBALATOMS |
		WINSTA_EXITWINDOWS |
		WINSTA_ENUMERATE |
		WINSTA_READSCREEN);

	if (m_hWinsta != NULL) SetProcessWindowStation(m_hWinsta);

	m_hDesk = OpenDesktop(L"default", 0, false,
		DESKTOP_CREATEMENU |
		DESKTOP_CREATEWINDOW |
		DESKTOP_ENUMERATE |
		DESKTOP_HOOKCONTROL |
		DESKTOP_JOURNALPLAYBACK |
		DESKTOP_JOURNALRECORD |
		DESKTOP_READOBJECTS |
		DESKTOP_SWITCHDESKTOP |
		DESKTOP_WRITEOBJECTS);

	if (m_hDesk != NULL) {
		SetThreadDesktop(m_hDesk);
	}

	Current_Desktop = GetDesktop(m_hDesk);

}

RemoteDesktop::DesktopMonitor::~DesktopMonitor(){
	if (m_hCurWinsta != NULL) CloseWindowStation(m_hCurWinsta);
	if (m_hWinsta != NULL)CloseWindowStation(m_hWinsta);
	if (m_hDesk != NULL)CloseDesktop(m_hDesk);
}


RemoteDesktop::Desktops RemoteDesktop::DesktopMonitor::GetDesktop(HDESK s){
	if (s == NULL)
		return Default;
	DWORD needed = 0;
	wchar_t new_name[256];
	auto result = GetUserObjectInformation(s, UOI_NAME, &new_name, 256, &needed);
	std::wstring dname = new_name;
	std::transform(dname.begin(), dname.end(), dname.begin(), ::tolower);
	if (!result)
		return Default;
	if (std::wstring(L"default") == dname)
		return Default;
	else if (std::wstring(L"screensaver") == dname)
		return ScreenSaver;
	else
		return Winlogon;
	
}

RemoteDesktop::Desktops  RemoteDesktop::DesktopMonitor::GetActiveDesktop(){
	auto s = OpenInputDesktop(0, false, DESKTOP_SWITCHDESKTOP);
	auto d = GetDesktop(s);
	CloseDesktop(s);
	return d;
}
bool RemoteDesktop::DesktopMonitor::SwitchDesktop(Desktops dname){
	HDESK desktop = NULL;
	DWORD mask = DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
		DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
		DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
		DESKTOP_SWITCHDESKTOP | GENERIC_WRITE;

	if (dname == Default)
		desktop = OpenDesktop(L"default", 0, false, mask);
	else if (dname == ScreenSaver)
		desktop = OpenDesktop(L"ScreenSaver", 0, false, mask);
	else
		desktop = OpenDesktop(L"Winlogon", 0, false, mask);

	

	if (desktop == NULL)
		return false;
	if (!SetThreadDesktop(desktop))
	{
		CloseDesktop(desktop);
		return false;
	}
	CloseDesktop(m_hDesk);
	m_hDesk = desktop;
	Current_Desktop = dname;
	return true;


}