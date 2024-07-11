// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// TasktrayIconsB.cpp 
//
// タスクトレイにあるアイコン削除
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// まだいい感じに動いていない。。(/o＼)

#include "stdafx.h"

#include <afxwin.h>
#include <commctrl.h>  // 追加: コモンコントロールヘッダー


#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

std::wstring GetWindowTitle(HWND hwnd) {
    int length = GetWindowTextLength(hwnd);
    if (length == 0) return L"";
    std::wstring title(length + 1, L'\0');
    GetWindowText(hwnd, &title[0], length + 1);
    title.resize(length);
    return title;
}

void EnumWindowsDebug(HWND hwnd, int depth = 0) {
    HWND hwndChild = FindWindowEx(hwnd, NULL, NULL, NULL);
    while (hwndChild) {
        wchar_t className[256];
        GetClassName(hwndChild, className, sizeof(className) / sizeof(className[0]));
        std::wstring title = GetWindowTitle(hwndChild);
        for (int i = 0; i < depth; ++i) std::wcout << L"  ";
//        std::wcout << L"Class: " << className << L", Title: " << title << L", HWND: " << hwndChild << std::endl;

        EnumWindowsDebug(hwndChild, depth + 1);
        hwndChild = FindWindowEx(hwnd, hwndChild, NULL, NULL);
    }
}

BOOL FindChildWindow(HWND hwndParent, LPCWSTR className, HWND &foundHwnd) {
    HWND hwndChild = FindWindowEx(hwndParent, NULL, className, NULL);
    if (hwndChild) {
        foundHwnd = hwndChild;
        return TRUE;
    }

    hwndChild = FindWindowEx(hwndParent, NULL, NULL, NULL);
    while (hwndChild) {
        if (FindChildWindow(hwndChild, className, foundHwnd)) {
            return TRUE;
        }
        hwndChild = FindWindowEx(hwndParent, hwndChild, NULL, NULL);
    }

    return FALSE;
}

std::vector<HICON> GetToolbarIcons(HWND hwndToolbar) {
    std::vector<HICON> icons;
    int buttonCount = (int)SendMessage(hwndToolbar, TB_BUTTONCOUNT, 0, 0);
    std::wcout << L"TB_BUTTONCOUNT: " << buttonCount << std::endl;

    DWORD processID;
    GetWindowThreadProcessId(hwndToolbar, &processID);
    std::wcout << L"ToolbarWindow32 Process ID: " << processID << std::endl;

    if (buttonCount == 0) {
        std::cerr << "ToolbarWindow32のボタンが見つかりませんでした" << std::endl;
        EnumWindowsDebug(hwndToolbar);
    }

    for (int i = 0; i < buttonCount; i++) {
        TBBUTTON tbb = {0};
        if (SendMessage(hwndToolbar, TB_GETBUTTON, i, (LPARAM)&tbb)) {
            DWORD dwProcessId;
            GetWindowThreadProcessId(hwndToolbar, &dwProcessId);
            HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION, FALSE, dwProcessId);
            if (hProcess) {
                TBBUTTON tbRemote;
                SIZE_T bytesRead;
                if (ReadProcessMemory(hProcess, (LPCVOID)tbb.dwData, &tbRemote, sizeof(TBBUTTON), &bytesRead)) {
                    HWND hwndButton = (HWND)tbRemote.dwData;
                    HICON hIcon = (HICON)SendMessage(hwndButton, WM_GETICON, ICON_SMALL, 0);
                    if (!hIcon) {
                        hIcon = (HICON)SendMessage(hwndButton, WM_GETICON, ICON_BIG, 0);
                    }
                    if (hIcon) {
                        icons.push_back(hIcon);
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }

    return icons;
}

std::vector<HICON> GetTrayIcons() {
    std::vector<HICON> icons;

    HWND hwndTray = FindWindow(L"Shell_TrayWnd", NULL);
    if (!hwndTray) {
        std::cerr << "タスクバーのウィンドウが見つかりませんでした" << std::endl;
        return icons;
    }

    std::wcout << L"Shell_TrayWnd HWND: " << hwndTray << std::endl;

    HWND hwndTrayNotify = NULL;
    if (!FindChildWindow(hwndTray, L"TrayNotifyWnd", hwndTrayNotify)) {
        std::cerr << "TrayNotifyWndのウィンドウが見つかりませんでした" << std::endl;
        EnumWindowsDebug(hwndTray);
        return icons;
    }

    std::wcout << L"TrayNotifyWnd HWND: " << hwndTrayNotify << std::endl;

    HWND hwndToolbar = NULL;
    if (!FindChildWindow(hwndTrayNotify, L"ToolbarWindow32", hwndToolbar)) {
        std::cerr << "ToolbarWindow32のウィンドウが見つかりませんでした" << std::endl;
        EnumWindowsDebug(hwndTrayNotify);
        return icons;
    }

    std::wcout << L"ToolbarWindow32 HWND: " << hwndToolbar << std::endl;

    std::vector<HICON> toolbarIcons = GetToolbarIcons(hwndToolbar);
    icons.insert(icons.end(), toolbarIcons.begin(), toolbarIcons.end());

    return icons;
}

