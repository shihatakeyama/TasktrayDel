// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// アイコンを見つけて削除します。
//
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *



#include <stdint.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <iostream>

#include <afxwin.h>
#include <commctrl.h>  // 追加: コモンコントロールヘッダー
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

#include "stdafx.h"

// ツールチップテキストを取得するためのバッファサイズ
#define TOOLTIP_TEXT_LENGTH 256


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// トレイアイコンの情報を保持する構造体
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
struct TrayIconInfo {
	UINT index;
    HWND hwnd;
    UINT uID;
    UINT uCallbackMessage;
    HICON hIcon;
    std::wstring tooltxt;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// エクスプローラプロセスのPIDを取得する関数
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
DWORD GetExplorerProcessID() 
{
    DWORD explorerPID = 0;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry)) {
        do {
            if (!_tcsicmp(entry.szExeFile, _T("explorer.exe"))) {
                explorerPID = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return explorerPID;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ボタン情報取得
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
bool GetButtonInfo(HWND toolbarWnd, int index, TBBUTTON &tbButton) 
{
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(toolbarWnd, &dwProcessId);
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, dwProcessId);
    if (!hProcess) {
        return false;
    }

    TBBUTTON *pButton = (TBBUTTON *)VirtualAllocEx(hProcess, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
    if (!pButton) {
        CloseHandle(hProcess);
        return false;
    }

    if (!SendMessage(toolbarWnd, TB_GETBUTTON, index, (LPARAM)pButton)) {
        VirtualFreeEx(hProcess, pButton, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    ReadProcessMemory(hProcess, pButton, &tbButton, sizeof(TBBUTTON), NULL);

    VirtualFreeEx(hProcess, pButton, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return true;
}

typedef struct {
    UINT cbSize;
    DWORD dwMask;
    int idCommand;
    int iImage;
    BYTE fsState;
    BYTE fsStyle;
    WORD cx;
    int64_t lParam;	// OSが64bitの場合は64bit幅
    int64_t pszText;
    int cchText;
} TBBUTTONINFOWA;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// ツールチップ情報取得
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
std::wstring GetTooltipText(HWND toolbarWnd ,HANDLE hProc, int index) 
{
	BOOL back;
	SIZE_T dwBytesRead;
	TBBUTTONINFOWA mtbbi = {0};
    WCHAR textBuffer[TOOLTIP_TEXT_LENGTH] = {0};
	TBBUTTONINFOWA *lpData = (TBBUTTONINFOWA*)VirtualAllocEx(hProc , 0, sizeof(TBBUTTONINFOWA), MEM_COMMIT, PAGE_READWRITE);
	WCHAR *vtxt = (WCHAR*)VirtualAllocEx(hProc , 0, sizeof(textBuffer), MEM_COMMIT, PAGE_READWRITE);

	memset(&mtbbi,0,sizeof(mtbbi));
	mtbbi.cbSize	= sizeof(TBBUTTONINFOWA);
	mtbbi.dwMask	= TBIF_BYINDEX | TBIF_TEXT | TBIF_IMAGE | TBIF_STYLE | TBIF_LPARAM | TBIF_COMMAND | TBIF_STATE;
	mtbbi.pszText	= reinterpret_cast<int64_t>(vtxt);
	mtbbi.cchText	= reinterpret_cast<int64_t>(vtxt);

	WriteProcessMemory(hProc,lpData,&mtbbi,sizeof(TBBUTTONINFOWA),&dwBytesRead);
	back = SendMessage(toolbarWnd, TB_GETBUTTONINFO, index, (LPARAM)lpData);
	ReadProcessMemory(hProc, lpData, &mtbbi, sizeof(TBBUTTONINFOWA), &dwBytesRead);
	ReadProcessMemory(hProc, reinterpret_cast<LPVOID>(vtxt), textBuffer, sizeof(textBuffer), &dwBytesRead);

    VirtualFreeEx(hProc, lpData, 0, MEM_RELEASE);
    VirtualFreeEx(hProc, vtxt, 0, MEM_RELEASE);

	return std::wstring(textBuffer);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// タスクトレイアイコン情報を取得する関数
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
std::vector<TrayIconInfo> GetTrayIcons(HWND hToolWnd ,HANDLE hProc) 
{
    std::vector<TrayIconInfo> trayIcons;
	

    int buttonCount = SendMessage(hToolWnd, TB_BUTTONCOUNT, 0, 0);

    for (int i = 0; i < buttonCount; i++) {

        TBBUTTON tbButton = { 0 };
        char btnData[256] = { 0 };
		BOOL  back;

		back = GetButtonInfo(hToolWnd, i, tbButton);
		if(back == FALSE)	continue;

		std::wstring tooltxt = GetTooltipText(hToolWnd ,hProc ,tbButton.idCommand); 
		if(tooltxt.empty())	continue;

        TrayIconInfo iconInfo = { 0 };
		iconInfo.index	= i;
//        iconInfo.hwnd = tbButton.hwnd;
        iconInfo.uID	= tbButton.idCommand;
        iconInfo.hIcon	= (HICON)tbButton.dwData;
        iconInfo.tooltxt= tooltxt;

        trayIcons.push_back(iconInfo);
    }

    return trayIcons;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// 削除するアイコンを見つけて削除します。
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
void findAndDel(HANDLE hProcess ,HWND toolWnd ,_TCHAR *tname)
{
	int32_t ack;

	setlocale(LC_ALL, ""); // ロケールをデフォルトのロケールに設定

	std::vector<TrayIconInfo> icons = GetTrayIcons(toolWnd ,hProcess);

    for (const auto &e : icons) {

		if(tname == _T("-l")){
			wprintf(L"No.%4d \t %s\n" , e.index+1 , e.tooltxt.c_str());

		}else if(tname == e.tooltxt){

			ack = SendMessage(toolWnd ,TB_DELETEBUTTON ,e.uID ,0);
			if(ack == 1){
				wprintf(L"Delete %s\n" , tname);
			}
		}
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// タスクバーにあるアイコン削除 メイン
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
int delTaskbarMain(HANDLE hProcess ,_TCHAR *tname)
{
	HWND notifyWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
    if (!notifyWnd) return -12;

    HWND toolWnd = FindWindowEx(notifyWnd, NULL, _T("TrayNotifyWnd"), NULL);
    if (!toolWnd) return -13;

    HWND promotWnd = FindWindowEx(toolWnd, NULL, _T("ToolbarWindow32"), NULL);
    if (!promotWnd) return -13;

	findAndDel(hProcess ,promotWnd ,tname);

	return 0;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// タスクトレイにあるアイコン削除 メイン
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
int delTasktrayMain(HANDLE hProcess ,_TCHAR *tname) 
{
    HWND notifyWnd = FindWindow(_T("NotifyIconOverflowWindow"), NULL);
    if (!notifyWnd) return -12;

    HWND toolWnd = FindWindowEx(notifyWnd, NULL, _T("ToolbarWindow32"), NULL);
    if (!toolWnd) return -13;

	findAndDel(hProcess ,toolWnd ,tname);

    return 0;
}


