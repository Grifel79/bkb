#include <Windows.h>
#include "BKBProgressWnd.h"
#include "BKBRepErr.h"
#include "Fixation.h"
#include "KeybWnd.h"
#include "ToolWnd.h"

static const TCHAR *wnd_class_name=L"BKBProgress";
extern HINSTANCE BKBInst;
extern HPEN pink_pen;

HWND  BKBProgressWnd::PRhwnd=0;
int  BKBProgressWnd::percentage=0;

extern bool gBKB_2STEP_KBD_MODE;


// ������� ��������� 
LRESULT CALLBACK BKBProgressWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_CREATE:
		// ������� �� ��������� - ��� �� ������ ���� ���������� � ����� ��� ������
		// Suppose the window background color is white (255,255,255).
        // Call the SetLayeredWindowAttributes when create the window.
        SetLayeredWindowAttributes(hwnd,RGB(255,255,255),NULL,LWA_COLORKEY);
        break;
/*
	case WM_SIZE:
		break;
*/
	 case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		RECT rect;
		int width, height;

		hdc=BeginPaint(hwnd,&ps);
		
		GetClientRect(hwnd,&rect);
		width=rect.right-1;
		height=rect.bottom-1;

		SelectObject(hdc, pink_pen);

		MoveToEx(hdc,2,2,NULL);
		LineTo(hdc,width-3,2);
		LineTo(hdc,width-3,height-3);
		LineTo(hdc,2,height-3);
		LineTo(hdc,2,2);
		
		if(BKBProgressWnd::percentage>0)
		{
			MoveToEx(hdc,15,15, NULL);
			LineTo(hdc,(width-15)*BKBProgressWnd::percentage/100,15);
		}
		
		EndPaint(hwnd,&ps);
		break;
/*	
	 case WM_USER_MOVEWINDOW:
		 MoveWindow(hwnd,wparam,lparam,100,100,FALSE);
		 break;
*/
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // ����������, ��������� ���� �� break
}

//================================================================
// ������������� 
//================================================================
void BKBProgressWnd::Init(HWND master_hwnd)
{
	ATOM aresult; // ��� ������ ����� ��������
	
	// 1. ����������� ������ ����
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBProgressWndProc, 0,
		//sizeof(LONG_PTR), // ���� �������� ������ �� ������
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		(HBRUSH)GetStockObject(WHITE_BRUSH), 
		NULL,
		wnd_class_name
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__WIDEFILE__,L"RegisterClass (",__LINE__);
		return;
	}

// ������ ��� ���� � ���������� ���������� 
//	screen_x=GetSystemMetrics(SM_CXSCREEN);
//	screen_y=GetSystemMetrics(SM_CYSCREEN);

	PRhwnd=CreateWindowEx(
	WS_EX_LAYERED|WS_EX_TOPMOST,
	//|WS_EX_CLIENTEDGE,
	//WS_EX_LAYERED|WS_EX_TOPMOST,
	wnd_class_name,
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	//100,100, // �� ����� �� ������� ������� ������, ����� ���� � �������� ���������� � �������?? ���, ������ ��� ���� ��-�� HighDPI
	0,0,
	100,100, 
    //0,
	master_hwnd, // ����� � �������� � ��� ����-���� �� ���������� ������ ����
	0, BKBInst, 0L );

	if(NULL==PRhwnd)
	{
		BKBReportError(__WIDEFILE__,L"CreateWindow",__LINE__);
	}

	//�������� ��� �������
	ShowWindow(PRhwnd, SW_SHOW);
	UpdateWindow(PRhwnd);
}


//====================================================================================================================
// ��������, ���� ��� ���������� ������� �������������?
//====================================================================================================================
bool BKBProgressWnd::TryToShow(int _x, int _y, int _percentage) // true = Pink Approves, ���������� ��������, �������� �� ���������
{
	LPRECT lp_pink_rect;
	static int old_top, old_left, old_right, old_bottom;


	// ������� ����� ������
	lp_pink_rect=BKBToolWnd::PinkFrame(_x, _y);
	if(NULL==lp_pink_rect)
	{
		// ����� ��� ����� ���������
		// �� ����� - ������� ������ � ����������
		if(gBKB_2STEP_KBD_MODE&&(BKB_MODE_KEYBOARD==Fixation::CurrentMode()))
		{
			// � �������� �������� ������� �������� �� ��������
			if(BKBKeybWnd::Animate()) return false;

			lp_pink_rect=BKBKeybWnd::PinkFrame(_x, _y);
		}
	}
	
	if(NULL!=lp_pink_rect) // ��� ������!
	{
		if((old_top==lp_pink_rect->top)&&(old_left==lp_pink_rect->left)&&(old_right==lp_pink_rect->right)&&(old_bottom==lp_pink_rect->bottom))
		{
			// ������������� �� ������ �����, ������ ������ ProgressBar
			percentage=_percentage;
			InvalidateRect(PRhwnd,NULL,TRUE);
			return true; // Fixation approved
		}
		else // ����� �������������� ��������������
		{
			percentage=0;
			ShowWindow(PRhwnd, SW_SHOWNORMAL);
			MoveWindow(PRhwnd,lp_pink_rect->left,lp_pink_rect->top, lp_pink_rect->right-lp_pink_rect->left+1, lp_pink_rect->bottom-lp_pink_rect->top+1, TRUE);
			InvalidateRect(PRhwnd,NULL,TRUE);

			SetActiveWindow(PRhwnd);
			BringWindowToTop(PRhwnd); 

			old_top=lp_pink_rect->top;
			old_left=lp_pink_rect->left;
			old_right=lp_pink_rect->right;
			old_bottom=lp_pink_rect->bottom;

			// ����� ������������ �������������� ������ ���������� ���� fixation_count
			return false;
		}
		
		
		
	}
	else
	{
		old_top=-1; // ��� ����� ����� ����������� � ��� �� ������������� �� �������, �� �� �����, ��� ������ �� ���������, � ����� �� � SW_SHOWNORMAL
		ShowWindow(PRhwnd, SW_HIDE);
		return false;
	}
	
}