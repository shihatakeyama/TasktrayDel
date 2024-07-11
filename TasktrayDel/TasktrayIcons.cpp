// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// TasktrayIcons.cpp 
//
// �^�X�N�g���C�ɂ���A�C�R���폜
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

#if 0	// �Ȃ������悭�A�C�R���������Ă��܂��B

#include "stdafx.h"

#include <afxwin.h>
#include <commctrl.h>  // �ǉ�: �R�����R���g���[���w�b�_�[

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

#define WM_GETICON 0x007F
#define ICON_SMALL 0
#define ICON_BIG 1
#define ICON_SMALL2 2

// �w�肵���N���X���̎q�E�B���h�E���ċA�I�Ɍ���
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

// �c�[���o�[�A�C�e�������i�[����\����
struct ToolbarIconInfo {
    HWND hwnd;
    int index;
};

std::vector<ToolbarIconInfo> GetToolbarIcons(HWND hwndToolbar) {
    std::vector<ToolbarIconInfo> icons;
    int buttonCount = (int)SendMessage(hwndToolbar, TB_BUTTONCOUNT, 0, 0);

    for (int i = 0; i < buttonCount; i++) {
        TBBUTTON tbb = {0};
        if (SendMessage(hwndToolbar, TB_GETBUTTON, i, (LPARAM)&tbb)) {
            ToolbarIconInfo info = { hwndToolbar, i };
            icons.push_back(info);
        }
    }

    return icons;
}

std::vector<HICON> GetTrayIcons() {
    std::vector<HICON> icons;

    HWND hwndTray = FindWindow(L"Shell_TrayWnd", NULL);
    if (!hwndTray) {
        std::cerr << "�^�X�N�o�[�̃E�B���h�E��������܂���ł���" << std::endl;
        return icons;
    }

    HWND hwndToolbar = NULL;
    if (!FindChildWindow(hwndTray, L"ToolbarWindow32", hwndToolbar)) {
        std::cerr << "ToolbarWindow32�̃E�B���h�E��������܂���ł���" << std::endl;
        return icons;
    }

    std::vector<ToolbarIconInfo> toolbarIcons = GetToolbarIcons(hwndToolbar);

    for (const auto &info : toolbarIcons) {
        // �A�C�R���n���h�����擾����
        HICON hIcon = (HICON)SendMessage(info.hwnd, TB_GETIMAGELIST, 0, 0);
        if (hIcon) {
            icons.push_back(hIcon);
        }
    }

    return icons;
}

#endif

