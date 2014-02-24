#include <windows.h>
#include "BKBRepErr.h"
#include "TobiiREX.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "TranspWnd.h"
#include "KeybWnd.h"
#include "AirMouse.h"
#include "BKBgdi.h"

// �������� WndProc ����� � .h �� ��������
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
// � ��� ��� ����
int StartupDialog();

// ���������� ����������, ������� ����� ������������� �����
char*		BKBAppName="���������� � ���� ��� ���������� ������� : ������ B";
HINSTANCE	BKBInst;
HWND		BKBhwnd;
bool flag_using_airmouse;

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

	if(flag_using_airmouse)
	{
		// ��������� �������� ������ ���������� Tobii ���������
		// ��� ������� � WM_CREATE ToolBar'a
	}
	else
	{
		// �������������� ������ � Tobii REX
		// ���� ��������� ������, ��������� �� ������ � [����]�����
		flag_using_airmouse=BKBTobiiREX::Init();
	}

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

	if(!flag_using_airmouse) BKBTobiiREX::Halt();

	return 0;
}