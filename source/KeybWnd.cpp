#include <Windows.h>
#include "KeybWnd.h"
#include "BKBRepErr.h"

extern int flag_using_airmouse;

// ��������� ��������� ������ � ����� KeybLayouts.cpp
extern BKB_key BKBKeybLayouts [3][3][BKB_KBD_NUM_CELLS];

extern HINSTANCE BKBInst;
static const char *wnd_class_name="BKBKeyb";

HWND  BKBKeybWnd::Kbhwnd;
int BKBKeybWnd::current_pane=0;
float BKBKeybWnd::cell_size=0;
int BKBKeybWnd::screen_x, BKBKeybWnd::screen_y, BKBKeybWnd::start_y;
POINT BKBKeybWnd::start_point;
bool BKBKeybWnd::fixation_approved=false;
int BKBKeybWnd::row, BKBKeybWnd::column, BKBKeybWnd::screen_num=0;
bool BKBKeybWnd::shift_pressed=false, BKBKeybWnd::ctrl_pressed=false,
	BKBKeybWnd::alt_pressed=false,BKBKeybWnd::caps_lock_pressed=false,
	BKBKeybWnd::Fn_pressed=false;

extern HBRUSH dkblue_brush, blue_brush;
extern HFONT hfont;


// ������� ��������� 
LRESULT CALLBACK BKBKeybWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc=BeginPaint(hwnd,&ps);
		BKBKeybWnd::OnPaint(hdc);
		EndPaint(hwnd,&ps);
		break;

	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // ����������, ��������� ���� �� break
}

//================================================================
// ������������� 
//================================================================
void BKBKeybWnd::Init()
{
	ATOM aresult; // ��� ������ ����� ��������
	
	// 1. ����������� ������ ����
	WNDCLASS wcl={CS_HREDRAW | CS_VREDRAW, BKBKeybWndProc, 0,
		//sizeof(LONG_PTR), // ���� �������� ������ �� ������
		0,
		BKBInst,
		LoadIcon( NULL, IDI_APPLICATION),
        LoadCursor(NULL, IDC_ARROW), 
		//���� �� ������� ���
        dkblue_brush,
		//0,
		NULL,
		TEXT(wnd_class_name)
	};

	aresult=::RegisterClass(&wcl); 


	if (aresult==0)
	{
		BKBReportError(__FILE__,"RegisterClass (",__LINE__);
		return;
	}

	screen_x=GetSystemMetrics(SM_CXSCREEN);
	screen_y=GetSystemMetrics(SM_CYSCREEN);
	cell_size=screen_x/(float)BKB_KBD_NUM_CELLS;

	start_y=screen_y-(int)(cell_size*3);

	Kbhwnd=CreateWindowEx(
	WS_EX_TOPMOST|WS_EX_CLIENTEDGE,
	TEXT(wnd_class_name),
	NULL, //TEXT(KBWindowName),
    //WS_VISIBLE|WS_POPUP,
	WS_POPUP,
	0,start_y,screen_x,screen_x-start_y, 
    0, 0, BKBInst, 0L );

	if(NULL==Kbhwnd)
	{
		BKBReportError(__FILE__,"CreateWindow",__LINE__);
	}

	// ShowWindow(Kbhwnd,SW_SHOWNORMAL);
}

//================================================================
// ������ ���� (�� WM_PAINT ��� ����)
//================================================================
void BKBKeybWnd::OnPaint(HDC hdc)
{
	bool release_dc=false;

	if(0==hdc)
	{
		release_dc=true;
		hdc=GetDC(Kbhwnd);
	}

	HFONT old_font;

	SetTextColor(hdc,RGB(255,255,255));
	SetBkColor(hdc,RGB(45,62,90));


	// ����������, ���������
	// 1. ������� ��������� �������  ����. �������
	if(shift_pressed)
	{
		RECT rect={0,int(2*cell_size),int(cell_size),int(3*cell_size)};
		FillRect(hdc,&rect,blue_brush);
		if(caps_lock_pressed)	TextOut(hdc,int(cell_size/3), int(cell_size*2.8),"CAPS",4);
	}
	if((ctrl_pressed)&&(screen_num>0))
	{
		RECT rect={int(14*cell_size),int(2*cell_size),int(15*cell_size),int(3*cell_size)};
		FillRect(hdc,&rect,blue_brush);
	}
	if((alt_pressed)&&(screen_num>0))
	{
		RECT rect={int(14*cell_size),int(cell_size),int(15*cell_size),int(2*cell_size)};
		FillRect(hdc,&rect,blue_brush);
	}
	if((Fn_pressed)&&(2==screen_num))
	{
		RECT rect={0,int(cell_size),int(cell_size),int(2*cell_size)};
		FillRect(hdc,&rect,blue_brush);
		// ���������� �������, ������� ����� ��������������� (F1-F12)
		SelectObject(hdc,GetStockObject(WHITE_PEN));
		MoveToEx(hdc,int(cell_size)+2,int(2*cell_size-4),NULL);
		LineTo(hdc,int(12*cell_size-3),int(2*cell_size-4));
	}

	// 2. ������������ �����
	SelectObject(hdc,GetStockObject(WHITE_PEN));
	// 2.1. ��������������
	MoveToEx(hdc,0,int(cell_size),NULL);
	LineTo(hdc,screen_x-1,int(cell_size));
	MoveToEx(hdc,0,int(2*cell_size),NULL);
	LineTo(hdc,screen_x-1,int(2*cell_size));

	// 2.2. ������������
	int i,j;
	for(i=1;i<BKB_KBD_NUM_CELLS;i++)
	{
		MoveToEx(hdc,int(cell_size*i),0,NULL);
		LineTo(hdc,int(cell_size*i),int(cell_size*3));
		
		//TextOut(hdc,35,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],(int)strlen(tool_names[i]));
	}

	// 3. ����� �����
	// 3.1. ��������� ������ - ��, ��� ������� ������ �������
	for(j=0;j<3;j++)
		for(i=0;i<BKB_KBD_NUM_CELLS;i++)
		{
			if(NULL!=BKBKeybLayouts[screen_num][j][i].label) // ��������, ��� ��� �� NULL
			if(strlen(BKBKeybLayouts[screen_num][j][i].label)>1)
			TextOut(hdc,int(cell_size*0.4+i*cell_size) , int(cell_size/3+j*cell_size),
				BKBKeybLayouts[screen_num][j][i].label,strlen(BKBKeybLayouts[screen_num][j][i].label));
		}

	// 3.2. ����� ������ - �� ���������� ��������
	old_font=(HFONT)SelectObject(hdc, hfont);

	for(j=0;j<3;j++)
		for(i=0;i<BKB_KBD_NUM_CELLS;i++)
		{
			if(NULL!=BKBKeybLayouts[screen_num][j][i].label) // ��������, ��� ��� �� NULL
			if(1==strlen(BKBKeybLayouts[screen_num][j][i].label))
			TextOut(hdc,int(cell_size*0.4+i*cell_size) , int(cell_size/3+j*cell_size),
				BKBKeybLayouts[screen_num][j][i].label,1);
		}
	
	// ���������� ������ ����
	SelectObject(hdc, old_font);


	// ���� ���� DC - ����� ���
	if(release_dc) ReleaseDC(Kbhwnd,hdc);

}


//===========================================================================
// ������, ��� ����� ������ �� ������, �������� �������� �������
//===========================================================================
bool BKBKeybWnd::ProgressBar(POINT *p, int fixation_count, int percentage)
{

	if(1==fixation_count) // ������ ��������, �������� � �������� ��� �����
	{
		start_point=*p;
		if(start_point.y>=start_y) 
		{
			fixation_approved=true;

			// �������� � ������� ������
			if(start_point.y>=screen_y) start_point.y=screen_y-2; // ����� ��� �� ������ ������, ����� �� ���� ������� ������� row/column 
			if(start_point.x<0) start_point.x=0;
			if(start_point.x>=screen_x) start_point.x=screen_x-2;

			row=(int)((start_point.y-start_y)/cell_size);
			column=(int)(start_point.x/cell_size);
		}
		else fixation_approved=false;
	}
	else // ��� ����������� ��������. 
	{
		if((2==flag_using_airmouse)&&fixation_approved) // �������� ���� ����. � ������ � ��������� (��������) �����������, ����� ������ ����� �� ������� �������, � ������� �������� ��������
		{
			POINT p_tmp=*p;
			int row_tmp=(int)((p_tmp.y-start_y)/cell_size);
			int column_tmp=(int)(p_tmp.x/cell_size);

			if((row_tmp!=row)||(column_tmp!=column)) return false;
		}
	}


	// ������ ��������-��� �� ������
	if(fixation_approved)
	{
		HDC hdc=GetDC(Kbhwnd);
		RECT rect;
		
		rect.left=(LONG)(cell_size/20+column*cell_size);
		
		rect.right=(LONG)(rect.left+percentage*cell_size*90/100/100);
		rect.top=(LONG)(cell_size/20+cell_size*row);
		rect.bottom=(LONG)(rect.top+cell_size/20); 


		FillRect(hdc,&rect,blue_brush);

		ReleaseDC(Kbhwnd,hdc);
	}

	return true;
}

//===========================================================================
// �� �������� ������, ��� ������������� �� ������ �����
//===========================================================================
void BKBKeybWnd::ProgressBarReset()
{
	if(fixation_approved) // ��������, ��� �����������
	{
		HDC hdc=GetDC(Kbhwnd);
		RECT rect;
		
		rect.left=(LONG)(cell_size/20+column*cell_size);
		
		rect.right=(LONG)(rect.left+cell_size*90/100);
		rect.top=(LONG)(cell_size/20+cell_size*row);
		rect.bottom=(LONG)(rect.top+cell_size/20); 


		FillRect(hdc,&rect,dkblue_brush);

		ReleaseDC(Kbhwnd,hdc);
	}

	fixation_approved=false;
}

//===========================================================================
// ��������� ��������, �������� �� ����������
//===========================================================================
bool BKBKeybWnd::IsItYours(POINT *p)
{
	INPUT input[2]={0},shift_input={0},ctrl_input={0},alt_input={0};

	if(fixation_approved)
	{
		// �������� Progress Bar
		ProgressBarReset();
		fixation_approved=false;

		// ��������� �� ������� ������
		// ��������� ������� ������
		BKB_key key_pressed=BKBKeybLayouts[screen_num][row][column];
		if((Fn_pressed)&&(2==screen_num)&&(1==row)&&(0<column)&&(12>=column)) // ��� ���� �� 12 �������������� ������
		{
			ScanCodeButton(VK_F1+column-1);
				
			Fn_pressed=false; // ���������� ������� ������� Fn
			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
		}
		else // ��� �� �������������� �������
		switch (key_pressed.bkb_keytype)
		{
		case unicode: // ����� �������
					
			// ������� ������
			input[0].type=INPUT_KEYBOARD;
			input[0].ki.dwFlags =  KEYEVENTF_UNICODE;
		
			if(shift_pressed) input[0].ki.wScan=key_pressed.bkb_unicode_uppercase;
			else input[0].ki.wScan=key_pressed.bkb_unicode;

			// ���������� ������
			input[1].type=INPUT_KEYBOARD;
			input[1].ki.dwFlags = KEYEVENTF_KEYUP |  KEYEVENTF_UNICODE;
			if(shift_pressed) input[1].ki.wScan=key_pressed.bkb_unicode_uppercase;
			else input[1].ki.wScan=key_pressed.bkb_unicode;
			
			SendInput(2,input,sizeof(INPUT));

			if((shift_pressed)&&(!caps_lock_pressed))  // ���������� shift, ���� �� �� ������������ caps_lock'��
			{
				shift_pressed=false;
				InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			}
			break;

		case scancode: // ����� ����� ����� ������������ Alt,Shift,Ctrl; � ���� - ���
			ScanCodeButton(key_pressed.bkb_vscancode);
			break;

		case leftkbd: // ������ ����� ���������� (����� �� ��������)
			screen_num--;
			if(screen_num<0) screen_num=2;
			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			break;

		case rightkbd: // ������ ����� ���������� (������ �� ��������)
			screen_num++;
			if(screen_num>2) screen_num=0;
			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			break;

		case shift: // ������ ������ Shift
			if(shift_pressed) 
			{
				if(caps_lock_pressed) // ��� ����� caps_lock
				{
					shift_pressed=false;
					caps_lock_pressed=false;
				}
				else // ��������� ������� �� Shift ������� CapsLock
				{
					caps_lock_pressed=true;
				}
			}
			else shift_pressed=true; // ������ ������� shift

			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			break;

		case control: // ������ ������ Ctrl
			if(ctrl_pressed) ctrl_pressed=false; else ctrl_pressed=true;
			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			break;

		case alt: // ������ ������ Alt
			if(alt_pressed) alt_pressed=false; else alt_pressed=true;
			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			break;

		case fn: // ������ ������ Fn
			if(Fn_pressed) Fn_pressed=false; else Fn_pressed=true;
			InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
			break;
		}
		// ���� �����, �����, ��������, ������� � ��������� �������

	} //fixation approved

	// ���������� ������, ��������� �� ��� ������� � ���, ���� ���� ������ �� ������
	if(p->y>=start_y) 	return true;
	else return false;
}

//===========================================================================
// ��������� ����������
//===========================================================================
void BKBKeybWnd::Activate()
{
	ShowWindow(Kbhwnd,SW_SHOWNORMAL);
	fixation_approved=false;
}

//===========================================================================
// ����������� ����������
//===========================================================================
void BKBKeybWnd::DeActivate()
{
	ShowWindow(Kbhwnd,SW_HIDE);
	ProgressBarReset();
}

//===========================================================================
// ������������ ������� ������, �������� ���������, � �� ��������
// ����� ��������� �������������� �������� ����-������ Shift,Alt,Ctrl
//===========================================================================
void BKBKeybWnd::ScanCodeButton(WORD scancode)
{
	bool redraw_reqired=false;

	INPUT input={0};
	input.type=INPUT_KEYBOARD;

	// ����-�������
	if(shift_pressed)
	{
		input.ki.dwFlags =  0;
		input.ki.wVk=VK_SHIFT;
		SendInput(1,&input,sizeof(INPUT));		
	}
	if(ctrl_pressed)
	{
		input.ki.dwFlags =  0;
		input.ki.wVk=VK_CONTROL;
		SendInput(1,&input,sizeof(INPUT));		
	}
	if(alt_pressed)
	{
		input.ki.dwFlags =  0;
		input.ki.wVk=VK_MENU;
		SendInput(1,&input,sizeof(INPUT));		
	}

	// ���� ������� ������
	// ������� ������
	input.ki.dwFlags =  0;
	input.ki.wVk=scancode;
	SendInput(1,&input,sizeof(INPUT));		
	// ���������� ������
	input.ki.dwFlags = KEYEVENTF_KEYUP ;
	SendInput(1,&input,sizeof(INPUT));		
	
	// ����-�������
	if(shift_pressed)
	{
		// ��������� ������ Shift
		input.ki.wVk=VK_SHIFT;
		input.ki.dwFlags = KEYEVENTF_KEYUP ;
		SendInput(1,&input,sizeof(INPUT));

		// ��������� �� shift � ������?
		if((shift_pressed)&&(!caps_lock_pressed))  // ���������� shift, ���� �� �� ������������ caps_lock'��
		{
			shift_pressed=false;
			redraw_reqired=true;
		}
	}
	if(ctrl_pressed)
	{
		// ��������� ������ Ctrl
		input.ki.wVk=VK_CONTROL;
		input.ki.dwFlags = KEYEVENTF_KEYUP ;
		SendInput(1,&input,sizeof(INPUT));

		redraw_reqired=true;
		ctrl_pressed=false; // ��������� Ctrl
	}
	if(alt_pressed)
	{
		// ��������� ������ Alt
		input.ki.wVk=VK_MENU;
		input.ki.dwFlags = KEYEVENTF_KEYUP ;
		SendInput(1,&input,sizeof(INPUT));

		redraw_reqired=true;
		alt_pressed=false; // ��������� Alt
	}
	
	if(redraw_reqired) InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
}