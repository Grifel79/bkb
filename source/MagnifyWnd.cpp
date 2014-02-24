#include <Windows.h>
#include "MagnifyWnd.h"
#include "BKBRepErr.h"
#include "TranspWnd.h"

#include <stdio.h>
char debug_buffer[4096];


#define MAGNIFY_WINDOW_SIZE 400 // �� �������� �����o �������� �� 2*MAGNIFY_FACTOR
#define MAGNIFY_FACTOR 4 // �� ������� ��� �����������

extern HINSTANCE BKBInst;

static const char *wnd_class_name="BKBMagnify";

bool  BKBMagnifyWnd::mgf_visible=false; // ������� ����, ��� ���� ������ ����� �� ������
HWND  BKBMagnifyWnd::Mghwnd;
int BKBMagnifyWnd::x_size, BKBMagnifyWnd::y_size;
int BKBMagnifyWnd::screen_x, BKBMagnifyWnd::screen_y;
int BKBMagnifyWnd::midpoint_x, BKBMagnifyWnd::midpoint_y;

// ������� ��������� (�����������)
LRESULT CALLBACK BKBMagnifyWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	return DefWindowProc(hwnd,message,wparam,lparam);
}

//================================================================
// ������������� 
//================================================================
void BKBMagnifyWnd::Init()
{
	ATOM aresult; // ��� ������ ����� ��������
	
	// 1. ����������� ������ ����
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBMagnifyWndProc, 0,
		//sizeof(LONG_PTR), // ���� �������� ������ �� ������
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		//��� �� ���� ������� ���
        //(HBRUSH)GetStockObject(DKGRAY_BRUSH),
		0,
		NULL,
		TEXT(wnd_class_name)
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__FILE__,"RegisterClass (",__LINE__);
		return;
	}

	Mghwnd=CreateWindowEx(
	WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	TEXT(wnd_class_name),
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	100,700,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE, 
    0, 0, BKBInst, 0L );

	if(NULL==Mghwnd)
	{
		BKBReportError(__FILE__,"CreateWindow",__LINE__);
	}


	// ���������� ������ ���������������� ������� ����
	RECT rect;
	GetClientRect(Mghwnd,&rect);
	x_size=rect.right;
	y_size=rect.bottom;

	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);
	
	//ShowWindow(Mghwnd,SW_SHOWNORMAL);
}

//=========================================================================================
// ���� ���� ��������, �� �������� ���. ���� ������, �� �������� ���������� �����.
// ���������� true, ���� �� ������ � ����, � ���������� ����� �������� ������ ����������
//=========================================================================================
bool BKBMagnifyWnd::FixPoint(POINT *pnt)
{
	if(mgf_visible) // �������������� ���� �������
	{
		// ������ ���� � ����� ������
		mgf_visible=false;
		ShowWindow(Mghwnd,SW_HIDE);

		// � ������ �� �� � �������� ����?
		POINT local_point=*pnt;
		// ��� �������
		POINT p1,p2,p3;
		p1=local_point;
		ScreenToClient(Mghwnd,&local_point); // ���������� � ���������� ����
		p2=local_point;
		if((local_point.x>=0)&&(local_point.x<x_size)&&
			(local_point.y>=0)&&(local_point.y<y_size))
		{
			// �������� ���������� ��� �����
			// ��������� ����� ������� �� �������� - ����� �� ���������� � ��������� � midpoint
			pnt->x=midpoint_x+(local_point.x-x_size/2)/MAGNIFY_FACTOR;
			pnt->y=midpoint_y+(local_point.y-y_size/2)/MAGNIFY_FACTOR;
			// �������� �� ��������
			p3=*pnt;
			sprintf(debug_buffer,"�� ���������� (��������): %d %d\n� ���� ���������� (���������): %d %d\n����������: %d %d",p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
			//MessageBox(Mghwnd,debug_buffer,"�������",MB_OK);
			return true;
		}
		else
		{
			return false; // ������� �� ����
		}
		
	}
	else // ���� ��������, ����� ��� �������
	{
		// ����� � �������� ����
		midpoint_x=pnt->x; midpoint_y=pnt->y;

		// � �����, ������� �������� ����������, ������ ���� ����� �������� ����
		// ���� ����� ������� ������ � ����, ������������ �
		// ����� ����
		if(midpoint_x<MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_x=MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
		if(midpoint_y<MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_y=MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
		// ������ ����
		if(midpoint_x>screen_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_x=screen_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;
		if(midpoint_y>screen_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2) midpoint_y=screen_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2;

		// ������� ����
		// ���� �� ������ �������� �� �����
		int x_pos=midpoint_x-MAGNIFY_WINDOW_SIZE/2;
		if(x_pos<0) x_pos=0;
		if(x_pos>screen_x-MAGNIFY_WINDOW_SIZE) x_pos=screen_x-MAGNIFY_WINDOW_SIZE;
		int y_pos=midpoint_y-MAGNIFY_WINDOW_SIZE/2;
		if(y_pos<0) y_pos=0;
		if(y_pos>screen_y-MAGNIFY_WINDOW_SIZE) y_pos=screen_y-MAGNIFY_WINDOW_SIZE;
		
		MoveWindow(Mghwnd,x_pos,y_pos,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE, FALSE); // ��������� �������� ���������

		// ������ ����������� ����� ��������� ������

		// � ���� ����� ����� ������ ���� � ��������������
		BKBTranspWnd::Hide();

		HDC ScreenDC=GetDC(NULL); // �������� DC ������
		HDC MagBmpDC=CreateCompatibleDC(ScreenDC); // ������ ����������� DC
		HBITMAP MagBmp=CreateCompatibleBitmap(ScreenDC,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE); // ������� � �� ������ ������� �������
		HGDIOBJ OldBmp=SelectObject(MagBmpDC,MagBmp); // �������� ������ � DC (�� ��������� ������!!!)

		// ��� ������� ������� ����� ���������
		//RECT rrr={20,20,50,50};
		//FillRect(MagBmpDC,&rrr,(HBRUSH)GetStockObject(WHITE_BRUSH));

		// �������� ����� ������ � ������ (� �����������)
		StretchBlt(MagBmpDC,0,0,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE,ScreenDC,
			midpoint_x-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2,
			midpoint_y-MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR/2,
			MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR,MAGNIFY_WINDOW_SIZE/MAGNIFY_FACTOR,
			SRCCOPY);


		// ������ ���� �������
		mgf_visible=true;
		ShowWindow(Mghwnd,SW_SHOWNORMAL);

		// ���������� ���� � ��������������
		BKBTranspWnd::Show();

		// ������� ���������� ���� ������!!! (������ ��� �������� ����� ������ Fixation)
		//BKBTranspWnd::ToTop();
		

		// ����������� ����������� ����� ������ � ����
		HDC MgWindowDC=GetDC(Mghwnd);
		// �� �������� ��� ����� ����! ���� ��� � ���� ��������, �� �� ��...
		//BitBlt(MgWindowDC,0,0,MAGNIFY_WINDOW_SIZE,MAGNIFY_WINDOW_SIZE,MagBmpDC,0,0,SRCCOPY);
		BitBlt(MgWindowDC,0,0,x_size,y_size,MagBmpDC,
			(MAGNIFY_WINDOW_SIZE-x_size)/2,
			(MAGNIFY_WINDOW_SIZE-y_size)/2,
			SRCCOPY);
		ReleaseDC(Mghwnd,MgWindowDC);

		SelectObject(MagBmpDC,OldBmp); // ����������� ��� ������ �� DC
		DeleteObject(MagBmp); // ������ ��� ������ ����� �������� �������
		DeleteDC(MagBmpDC); // ���� �� � ����������� DC
		ReleaseDC(NULL,ScreenDC);
	}

	return false; // ����� �� �������� ���������� ��������
}