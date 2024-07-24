// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// TasktrayDel.cpp
//
//
// 
//
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

#include "stdafx.h"

#include <afxwin.h>
#include <commctrl.h>  // 追加: コモンコントロールヘッダー
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>


std::vector<HICON> GetTrayIcons();

DWORD GetExplorerProcessID();
int delTaskbarMain(HANDLE hProcess ,_TCHAR *tname);
int delTasktrayMain(HANDLE hProcess ,_TCHAR *tname);

int _tmain(int argc, _TCHAR* argv[])
{
	_TCHAR *tname;

	if(argc >= 2){
		tname = argv[1];
	}else{
		tname = _T("Folding active");
	}


	DWORD explorerPID = GetExplorerProcessID();
    if (!explorerPID) return -10;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, explorerPID);	// PROCESS_VM_READ | PROCESS_QUERY_INFORMATION
    if (!hProcess) return -11;


//	GetGoogleDriveIcon();

//	delFAHTasktray();

//	getchar();

//	GetTrayIcons();

	delTaskbarMain(hProcess ,tname);
	Sleep(100);	// 気休め
	delTasktrayMain(hProcess ,tname);

	CloseHandle(hProcess);

	return 0;
}

