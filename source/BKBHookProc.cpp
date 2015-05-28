#include <Windows.h>
#include "BKBHookProc.h"

bool volatile BKB_MBUTTON_PRESSED=false;
extern bool my_own_click;
extern int gBKB_MBUTTONFIX;
DWORD last_mouse_time=0;

//====================================================================================
// ����������, ��� (��� ������)
//====================================================================================
LRESULT  CALLBACK HookProc2(int disabled,WPARAM wParam,LPARAM lParam) 
{
	if (!disabled&&!my_own_click)
	{
		last_mouse_time=timeGetTime();

		MOUSEHOOKSTRUCT * pMouseStruct = (MOUSEHOOKSTRUCT *)lParam;
		if (pMouseStruct != NULL)
		{
			switch(wParam)
			{
			case WM_MBUTTONDOWN:
				if(0!=gBKB_MBUTTONFIX) // � ������ 0 - ������ ��������� ��������, ���� ����������
				{
					BKB_MBUTTON_PRESSED=true;
					return 1; // �������������, �� ��� ������������ � �������� �������
				}
				
				break;

			case WM_MBUTTONUP:
				if(0!=gBKB_MBUTTONFIX) // � ������ 0 - ������ ��������� ��������, ���� ����������
				{
					BKB_MBUTTON_PRESSED=false; // ������������ � ongazedata, �� �������������
					return 1; // �������������, �� ��� ������������ � �������� ����������
				}
				break;
			}
		}
	}
	return CallNextHookEx(NULL,disabled,wParam,lParam);
}