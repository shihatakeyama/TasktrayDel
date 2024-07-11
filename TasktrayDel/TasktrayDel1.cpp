// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
//
//
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


#include "stdafx.h"

#include <afxwin.h>
#include <commctrl.h>  // �ǉ�: �R�����R���g���[���w�b�_�[
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

#define WM_GETICON	0x007F
#define ICON_SMALL	0
#define ICON_BIG	1
#define ICON_SMALL2 2

#define BUFFER_SIZE 256


#if 1
// �c�[���`�b�v�̃e�L�X�g���擾���邽�߂̍\����
struct TooltipInfo {
    HWND hwnd;       // �e�E�B���h�E�̃n���h��
    std::wstring text; // �c�[���`�b�v�̃e�L�X�g
};


static DWORD GetProcessIDByName(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(snapshot, &pe)) {
            do {
                if (wcscmp(pe.szExeFile, processName) == 0) {
                    processID = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &pe));
        }
        CloseHandle(snapshot);
    }
    return processID;
}

static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam) {
	TooltipInfo* pInfo = reinterpret_cast<TooltipInfo*>(lParam);
	DWORD targetProcessID = *(DWORD*)lParam;
    DWORD processID = 0;
    GetWindowThreadProcessId(hwnd, &processID);

	    // �c�[���`�b�v�E�B���h�E��������
    wchar_t className[BUFFER_SIZE];
    GetClassName(hwnd, className, BUFFER_SIZE);

    if (wcscmp(className, TOOLTIPS_CLASS) == 0) {
        // �c�[���`�b�v�̃e�L�X�g���擾����
        TOOLINFO toolInfo = {};
        toolInfo.cbSize = sizeof(toolInfo);
        toolInfo.hwnd = pInfo->hwnd;
        toolInfo.uId = (UINT_PTR)pInfo->hwnd;
        
        wchar_t tooltipText[BUFFER_SIZE] = {0};
        toolInfo.lpszText = tooltipText;

        SendMessage(hwnd, TTM_GETTEXT, 0, (LPARAM)&toolInfo);
        pInfo->text = tooltipText;
        return FALSE;  // �ړI�̃E�B���h�E�����������̂ŗ񋓂��I��
    }

    std::wcout << L"EnumChildProc() hwnd: " << hwnd << L" processID: "<< processID << L"className :" << className << std::endl;

//    if (processID == targetProcessID) 
	{
        HICON hIcon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0);
        if (hIcon) {
            std::wcout << L"�A�C�R���n���h��: " << hIcon << std::endl;
        } else {
            hIcon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
            if (hIcon) {
                std::wcout << L"�A�C�R���n���h��: " << hIcon << std::endl;
            }
        }
//        return FALSE;  // �ړI�̃E�B���h�E�����������̂ŗ񋓂��I��
    }
    return TRUE;  // �񋓂𑱍s
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// �^�X�N�g���C�ɂ���FAH���폜���܂��B
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void delFAHTasktray()
{
    std::vector<HICON> icons;

    DWORD processID = GetProcessIDByName(L"ThPanel.exe");	// L"FAHCoreWrapper.exe"	FahCore_23.exe  L"FAHClient.exe"
    if (!processID) {
        std::cerr << "FAH�̃v���Z�X��������܂���ł���" << std::endl;
        return;
    }

    HWND hwndTray = FindWindow(L"Shell_TrayWnd", NULL);
    HWND hwndRebar = FindWindowEx(hwndTray, NULL, L"ReBarWindow32", NULL);
    HWND hwndTrayNotify = FindWindowEx(hwndTray, NULL, L"TrayNotifyWnd", NULL);
	HWND hwndSysPage = FindWindowEx(hwndTrayNotify, NULL, L"SysPager", NULL);
	HWND hwndToolBarWnd = FindWindowEx(hwndSysPage, NULL, L"ToolbarWindow32", NULL);
	HWND hwnd = hwndToolBarWnd;

    if (!hwndTrayNotify) {
        std::cerr << "�^�X�N�g���C�̃E�B���h�E�������邱�Ƃ��ł��܂���ł���" << std::endl;
        return;
    }

	DWORD pid;
	DWORD ack = GetWindowThreadProcessId(hwnd, &pid);

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	LPVOID ptrVa = VirtualAllocEx(hProc, 0, 256, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	auto buttonCount = SendMessage(hwndToolBarWnd, TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < buttonCount; i++) {
        // �{�^�������擾����
        TBBUTTON tbb = {0};
        if (SendMessage(hwndToolBarWnd, TB_GETBUTTON, i, (LPARAM)&tbb)) {
            // �A�C�R���n���h�����擾����
            HICON hIcon = (HICON)SendMessage(hwndToolBarWnd, TB_GETIMAGELIST, 0, 0);
            if (hIcon) {
                icons.push_back(hIcon);
            }
        }
    }


	TooltipInfo info = { hwndTrayNotify, L"" };
    EnumChildWindows(hwndTrayNotify, EnumChildProc, (LPARAM)&processID);

}
#endif
