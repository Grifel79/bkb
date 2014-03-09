#include <windows.h>
#include "BKBRepErr.h"
#include "TobiiREX.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "AirMouse.h"
#include "BKBgdi.h"
#include "TET.h"

// �������� WndProc ����� � .h �� ��������
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
// � ��� ��� ����
int StartupDialog();

// ���������� ����������, ������� ����� ������������� �����
char*		BKBAppName="���������� � ���� ��� ���������� ������� : ������ B";
HINSTANCE	BKBInst;
HWND		BKBhwnd;
int flag_using_airmouse;

// ��� ������ ����
static const char BKBWindowCName[]="BKB0B"; 

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE,LPSTR cline,INT)
// ��������� ������ �� ������������
{
	ATOM aresult; // ��� ������ ����� ��������
	BOOL boolresult;
	MSG msg; // ���������

	// ����� ������ Instance ��������� ����
	BKBInst=hInst;

	// ��� ����� ������������?
	flag_using_airmouse=StartupDialog();

	switch(flag_using_airmouse)
	{
	case 0: // Tobii
		// �������������� ������ � Tobii REX
		// ���� ��������� ������, ��������� �� ������ � [����]�����
		if(BKBTobiiREX::Init()) flag_using_airmouse=1; // ������� � TheEyeTribe (����������, ��� ������ � �����)
		else break; // �� ������ �������

	case 1: // TheEyeTribe
		if(BKBTET::Init()) flag_using_airmouse=2; // ������� � TheEyeTribe (����������, ��� ������ � �����)
		else break; // �� ������ �������
		
	case 2: // ������ ����... ������ ����� �� ������
		// ��������� �������� ������ ���������� Tobii ���������
		// ��� ������� � WM_CREATE ToolBar'a
		break;
	}

	/*	{
		// ��������� �������� ������ ���������� Tobii ���������
		// ��� ������� � WM_CREATE ToolBar'a
	}
	else
	{
		// �������������� ������ � Tobii REX
		// ���� ��������� ������, ��������� �� ������ � [����]�����
		flag_using_airmouse=BKBTobiiREX::Init();
	}
*/
	// �����-�����
	BKBgdiInit();

	// ������� ����
	BKBMagnifyWnd::Init(); // ��������������
	BKBToolWnd::Init(); // ����������� � ������ �������
	BKBKeybWnd::Init(); // ����������
	BKBTranspWnd::Init(); // ���������� ����

	//���� ��������� ���������
	while(GetMessage(&msg,NULL,0,0)) 
    {
		TranslateMessage( &msg );
        DispatchMessage( &msg );
	}// while !WM_QUIT

	// ������ �� �����

	// �����-�����
	BKBgdiHalt();

	switch(flag_using_airmouse)
	{
	case 0:
		BKBTobiiREX::Halt();
		break;

	case 1:
		BKBTET::Halt();
		break;
	}


	return 0;
}