// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// TasktrayDel.cpp
//
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


std::vector<HICON> GetTrayIcons();



int delTasktrayMain(_TCHAR *tname);

int _tmain(int argc, _TCHAR* argv[])
{
	_TCHAR *tname;

	if(argc >= 2){
		tname = argv[1];
	}else{
		tname = _T("Folding active");
	}


//	GetGoogleDriveIcon();

//	delFAHTasktray();

//	getchar();

//	GetTrayIcons();

	delTasktrayMain(tname);

	return 0;
}

