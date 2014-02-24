#include <Windows.h>
#include <stdint.h> // ��� ��� uint64_t
#include <stdio.h> // ��� ��� ������� 
#include "Fixation.h"
#include "MagnifyWnd.h"
#include "ToolWnd.h"
#include "KeybWnd.h"

static char debug_buf[4096];

BKB_MODE Fixation::BKB_Mode=BKB_MODE_NONE;

//==============================================================================================
// ������ ��������������
// ���������� true, ���� ������ �������������� ��� ���� , � ����� ���������� � ���������� ����
//==============================================================================================
bool Fixation::Fix(POINT p)
{
	// ��� ����� �������� �� ����������� ����� � �������� �����
	static bool drag_in_progress=false;

	// ������ ����� ����� ������
	switch (BKB_Mode)
	{
	// ������� ��� ������� � �����������
	case BKB_MODE_LCLICK: // ������ ����� ������� ����
	case BKB_MODE_RCLICK: // ������ ������ ������� ����
	case BKB_MODE_DOUBLECLICK: // ������� ������
	case BKB_MODE_DRAG: // ��, ����

		// ���� ���� ��� ��������, �������� ���������� �� ������ � ���� ����
		// ������������ ������ � �������� ����� Magnify ����������
		if(BKBMagnifyWnd::IsVisible())
		{
			if(BKBMagnifyWnd::FixPoint(&p)) // ������ � ����, ���������� ����� �������� ��� ����������
			{
				switch (BKB_Mode) // �! �����!
				{
					case BKB_MODE_LCLICK: // ������ ����� ������� ����
						LeftClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // ���� ��� ������ ����� ����-��
						break;

					case BKB_MODE_RCLICK: // ������ ������ ������� ����
						RightClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // ���� ��� ������ ����� ����-��
						break;

					case BKB_MODE_DOUBLECLICK: // ������� ������
						DoubleClick(p);
						BKBToolWnd::Reset(&BKB_Mode); // ���� ��� ������ ����� ����-��
						break;

					case BKB_MODE_DRAG: // ��, ����
						drag_in_progress=Drag(p);
						if(!drag_in_progress) BKBToolWnd::Reset(&BKB_Mode);
						break;
				}
			}
			// else = ������������ ���� ����, ���� ��������, ����� �� ���������
		}
		else // ���� � ����������� �� �������
		{
			if(!drag_in_progress) // ����-�� ���������, � ����� �����?
			{
				// ���� ����������� �����, ���� ���������� ���� Magnify
				if(!BKBToolWnd::IsItYours(&p, &BKB_Mode))
				{
					// ���� Toolbox, ���������� ���� Magnify
					BKBMagnifyWnd::FixPoint(&p);
				}
			}
			else // ��� drag_in_progress ����� ��������� ���
			{
				if(BKBMagnifyWnd::FixPoint(&p)) // ������ � ����, ���������� ����� �������� ��� ����������
				{
					drag_in_progress=Drag(p); 
				}
			}
		}
		break;

	case BKB_MODE_KEYBOARD: 
		// ������� ������?
		if(!BKBKeybWnd::IsItYours(&p))
		{
			// ���, ��������, ��� ������������ ������
			BKBToolWnd::IsItYours(&p, &BKB_Mode);
		}
		break;

	default: // ���, ���� �� ���� �� ����� ������������, ������ ����-�� ����������� �����
		// � ��� ����� BKB_MODE_NONE
		BKBToolWnd::IsItYours(&p, &BKB_Mode);
	}



	return true;
}

//==============================================================================================
// ��������� ������� � ���������� ����� ������ ����
//==============================================================================================
void Fixation::LeftClick(POINT p)
{
	// ������� �� ���������
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	int xs,ys,x,y;
	
	xs=XSCALEFACTOR;
	ys=YSCALEFACTOR;
	x=p.x;
	y=p.y;
	//screenx=GetSystemMetrics(SM_CXSCREEN);
	sprintf(debug_buf,"xs:%d ys:%d x:%d y:%d",xs,ys,x,y);
	//MessageBox(NULL,debug_buf,"debug",MB_OK);

	INPUT input[3];

	// 1. ������� �������� ������
	input[0].type=INPUT_MOUSE;
	input[0].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[0].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[0].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
	input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
	input[0].mi.time=0;
	input[0].mi.dwExtraInfo=0;

	// 2. ������� ����� ������
	input[1].type=INPUT_MOUSE;
	input[1].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[1].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[1].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
	input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
	input[1].mi.time=0;
	input[1].mi.dwExtraInfo=0;

	// 3. ���������� ����� ������
	input[2].type=INPUT_MOUSE;
	input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[2].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
	input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
	input[2].mi.time=0;
	input[2].mi.dwExtraInfo=0;
		
	// ��������� ������� � ���������� ����� ������ ����
	SendInput(3,input,sizeof(INPUT));
}

//==============================================================================================
// ��������� ������� � ���������� ������ ������ ����
//==============================================================================================
void Fixation::RightClick(POINT p)
{
	// ������� �� ���������
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	INPUT input[3];

	// 1. ������� �������� ������
	input[0].type=INPUT_MOUSE;
	input[0].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[0].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[0].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
	input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
	input[0].mi.time=0;
	input[0].mi.dwExtraInfo=0;

	// 2. ������� ������ ������
	input[1].type=INPUT_MOUSE;
	input[1].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[1].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[1].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
	input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTDOWN;
	input[1].mi.time=0;
	input[1].mi.dwExtraInfo=0;

	// 3. ���������� ������ ������
	input[2].type=INPUT_MOUSE;
	input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
	input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
	input[2].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
	input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_RIGHTUP;
	input[2].mi.time=0;
	input[2].mi.dwExtraInfo=0;
		
	// ��������� ������� � ���������� ������ ������ ����
	SendInput(3,input,sizeof(INPUT));
}

//==============================================================================================
// ��������� ���� ������ ���
//==============================================================================================
void Fixation::DoubleClick(POINT p)
{
	LeftClick(p);
	Sleep(80);
	LeftClick(p);
}


//==============================================================================================
// ��������� ������ � ����� �����
//==============================================================================================
bool Fixation::Drag(POINT p)
{
	static bool drag_in_progress=false;
	static POINT p_initial;

	// ������� �� ���������
	double XSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
    double YSCALEFACTOR = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	INPUT input[4];

	if(!drag_in_progress) // ������ ��������
	{
		drag_in_progress=true;
		// ������ ���������� �������� �������
		p_initial=p;
	}
	else
	{
		drag_in_progress=false;

		// 1. ������� �������� ������
		input[0].type=INPUT_MOUSE;
		input[0].mi.dx=(LONG)(p_initial.x*XSCALEFACTOR);
		input[0].mi.dy=(LONG)(p_initial.y*YSCALEFACTOR);
		input[0].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
		input[0].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
		input[0].mi.time=0;
		input[0].mi.dwExtraInfo=0;

		// 2. ������� ����� ������
		input[1].type=INPUT_MOUSE;
		input[1].mi.dx=(LONG)(p_initial.x*XSCALEFACTOR);
		input[1].mi.dy=(LONG)(p_initial.y*YSCALEFACTOR);
		input[1].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
		input[1].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN;
		input[1].mi.time=0;
		input[1].mi.dwExtraInfo=0;
		
		// 3. ������� �������� ������
		input[2].type=INPUT_MOUSE;
		input[2].mi.dx=(LONG)(p.x*XSCALEFACTOR);
		input[2].mi.dy=(LONG)(p.y*YSCALEFACTOR);
		input[2].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
		input[2].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE;
		input[2].mi.time=0;
		input[2].mi.dwExtraInfo=0;

	
		// 4. ���������� ����� ������
		input[3].type=INPUT_MOUSE;
		input[3].mi.dx=(LONG)(p.x*XSCALEFACTOR);
		input[3].mi.dy=(LONG)(p.y*YSCALEFACTOR);
		input[3].mi.mouseData=0; // ����� ��� ������ ���� ��������� 
		input[3].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP;
		input[3].mi.time=0;
		input[3].mi.dwExtraInfo=0;
	}

	// ��������� ������� � ���������� ������ ������ ����
	SendInput(4,input,sizeof(INPUT));
	return drag_in_progress;

}

//==============================================================================================
// ������ �� ��������, ���������������� ������� � ������� direction
//==============================================================================================
void Fixation::Scroll(uint64_t timelag, int direction)
{
	// ��� ������� � ��������� �������� timelag
			//char msgbuf[1024];
			//sprintf(msgbuf,"%llu\n",timelag);
			//OutputDebugString(msgbuf); 

	if(timelag>100000UL) timelag=100000UL;

	INPUT input;

	input.type=INPUT_MOUSE;
	input.mi.dx=0L;
	input.mi.dy=0L;
	input.mi.mouseData=direction*(timelag/2000UL); // ������ �� ��������, ���������������� �������
	//input.mi.mouseData=direction*3; // ��� �����
	input.mi.dwFlags=MOUSEEVENTF_WHEEL;
	input.mi.time=0;
	input.mi.dwExtraInfo=0;

	SendInput(1,&input,sizeof(INPUT));
}