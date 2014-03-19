#include <Windows.h>
#include "KeybWnd.h"
#include "BKBRepErr.h"

#define WM_USER_INVALRECT (WM_USER + 100)

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
int BKBKeybWnd::row, BKBKeybWnd::column, BKBKeybWnd::screen_num=0, BKBKeybWnd::percentage;
bool BKBKeybWnd::shift_pressed=false, BKBKeybWnd::ctrl_pressed=false,
	BKBKeybWnd::alt_pressed=false,BKBKeybWnd::caps_lock_pressed=false,
	BKBKeybWnd::Fn_pressed=false;

HDC BKBKeybWnd::memdc1=0, BKBKeybWnd::memdc2=0, BKBKeybWnd::whitespot_dc=0;
HBITMAP BKBKeybWnd::hbm1=0, BKBKeybWnd::hbm2=0, BKBKeybWnd::whitespot_bitmap=0;

volatile LONG BKBKeybWnd::redraw_state=0;
int BKBKeybWnd::width, BKBKeybWnd::height;
POINT BKBKeybWnd::whitespot_point={-100,-100};

int BKBKeybWnd::row_pressed=-1, BKBKeybWnd::column_pressed=-1;

extern HBRUSH dkblue_brush, blue_brush;
extern HFONT hfont;


// ������� ��������� 
LRESULT CALLBACK BKBKeybWndProc(HWND hwnd,
						UINT message,
						WPARAM wparam,
						LPARAM lparam)
{
	HDC hdc;

	switch (message)
	{

	case WM_USER_INVALRECT: // ��� �������� �� ������� ������
		InvalidateRect(hwnd,NULL,TRUE);
		break;

	case WM_TIMER: // ������ ����� ���������
		BKBKeybWnd::OnTimer();
		break;

	case WM_SETCURSOR:
		// ������ ������ ������ ��� �����, ��� ���� ��������� ����� �����
		// � ������������ ���������� 
		SetCursor(NULL);
		break;

	case WM_CREATE:
		// ������ ����� �����
		BKBKeybWnd::CreateWhiteSpot(hwnd);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc=BeginPaint(hwnd,&ps);
		BKBKeybWnd::OnPaint(hdc);
		EndPaint(hwnd,&ps);
		break;

	case WM_SIZE:
		BKBKeybWnd::OnSize(hwnd, LOWORD(lparam), HIWORD(lparam));
		break;

		// !! �������� ���� ������ ���� memdc ��� ������ (WM_CLOSE) !!
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}

	return 0; // ����������, ��������� ���� �� break
}

//================================================================
// ���������� �������� ������� ������
//================================================================
void BKBKeybWnd::OnTimer()
{
	//OutputDebugString("timer\n");
		KillTimer(Kbhwnd,4);
		row_pressed=-1; column_pressed=-1;
		redraw_state=0;
		InvalidateRect(Kbhwnd,NULL,TRUE); // ��� ������������ �����, ��� InvalidateRect ����� ���� ������, ��� ��� ����� - ����.
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
		NULL,
        //LoadCursor(NULL, IDC_ARROW), 
		// ��� �������� ������ � memdc1
        //dkblue_brush,
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
	LONG local_redraw_state;

	// ������ ����� �������� ��������� � ���������� redraw_state;
	local_redraw_state=InterlockedCompareExchange(&redraw_state,2,0);
	if(0!=local_redraw_state) // ������ �� ���������, ������� ��������� 1
	{
		local_redraw_state=InterlockedCompareExchange(&redraw_state,2,1);
		// ���� � ��� ������ �� �����, ������ ���������
		// ���� ��� ���� ������, ���� �������� ������ ���������� ����� ����� InterlockedCompareExchange
		// ���� ������ ����������, �� ����� ��� ���� WM_PAINT, � �� ��� �� ����� �������
		// ������ ������ ��� �������� BitBlt � ���� 2. ����� �� ����������.
		// ������-�� 0 �� 1 ���������� �� ����� � ��������. ������ ��� ���� ����� Interlocked... ������� ������������
		// ������, �� ��������.
		if(1!=local_redraw_state) local_redraw_state=2;
	}
	

	if(0==hdc)
	{
		release_dc=true;
		hdc=GetDC(Kbhwnd);
	}

	HFONT old_font;

	SetTextColor(memdc1,RGB(255,255,255));
	SetBkColor(memdc1,RGB(45,62,90));
	//SetBkMode(memdc1,TRANSPARENT);

	RECT fill_r={0,0,width,height};
	RECT rect_pressed={int(column_pressed*cell_size),int(row_pressed*cell_size),int((column_pressed+1)*cell_size),int((row_pressed+1)*cell_size)};

	/*
	switch(local_redraw_state)
	{
	case 0: // �������������� �� � ����� �������
		OutputDebugString("redraw 0\n");
		break;
	case 1:
		OutputDebugString("redraw 1\n");
		break;
	case 2:
		OutputDebugString("redraw 2\n");
		break;
	default:
		OutputDebugString("redraw XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		break;
	}

	*/
	

	switch(local_redraw_state)
	{
	case 0: // �������������� �� � ����� �������
		FillRect(memdc1,&fill_r,dkblue_brush);
		// ������������ ������� �������
		if(column_pressed!=-1)
		{
			//RECT rect_pressed={int(column_pressed*cell_size),int(row_pressed*cell_size),int((column_pressed+1)*cell_size),int((row_pressed+1)*cell_size)};
			FillRect(memdc1,&rect_pressed,blue_brush);
		}

		// ����������, ���������
		// 1. ������� ��������� �������  ����. �������
		if(shift_pressed)
		{
			RECT rect={0,int(2*cell_size),int(cell_size),int(3*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
			if(caps_lock_pressed)	TextOut(memdc1,int(cell_size/3), int(cell_size*2.8),"CAPS",4);
		}
		if((ctrl_pressed)&&(screen_num>0))
		{
			RECT rect={int(14*cell_size),int(2*cell_size),int(15*cell_size),int(3*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if((alt_pressed)&&(screen_num>0))
		{
			RECT rect={int(14*cell_size),int(cell_size),int(15*cell_size),int(2*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
		}
		if((Fn_pressed)&&(2==screen_num))
		{
			RECT rect={0,int(cell_size),int(cell_size),int(2*cell_size)};
			FillRect(memdc1,&rect,blue_brush);
			// ��������� �������, ������� ����� ��������������� (F1-F12)
			SelectObject(memdc1,GetStockObject(WHITE_PEN));
			MoveToEx(memdc1,int(cell_size)+2,int(2*cell_size-4),NULL);
			LineTo(memdc1,int(12*cell_size-3),int(2*cell_size-4));
		}

		// 2. ������������ �����
		SelectObject(memdc1,GetStockObject(WHITE_PEN));
		// 2.1. ��������������
		MoveToEx(memdc1,0,int(cell_size),NULL);
		LineTo(memdc1,screen_x-1,int(cell_size));
		MoveToEx(memdc1,0,int(2*cell_size),NULL);
		LineTo(memdc1,screen_x-1,int(2*cell_size));

		// 2.2. ������������
		int i,j;
		for(i=1;i<BKB_KBD_NUM_CELLS;i++)
		{
			MoveToEx(memdc1,int(cell_size*i),0,NULL);
			LineTo(memdc1,int(cell_size*i),int(cell_size*3));
			
			//TextOut(hdc,35,60+i*screen_y/BKB_NUM_TOOLS,tool_names[i],(int)strlen(tool_names[i]));
		}

		// 3. ����� �����
		// 3.1. ��������� ������ - ��, ��� ������� ������ �������
		for(j=0;j<3;j++)
			for(i=0;i<BKB_KBD_NUM_CELLS;i++)
			{
				if(NULL!=BKBKeybLayouts[screen_num][j][i].label) // ��������, ��� ��� �� NULL
				if(strlen(BKBKeybLayouts[screen_num][j][i].label)>1)
				TextOut(memdc1,int(cell_size*0.4+i*cell_size) , int(cell_size/3+j*cell_size),
					BKBKeybLayouts[screen_num][j][i].label,strlen(BKBKeybLayouts[screen_num][j][i].label));
			}
				
		// 3.2. ����� ������ - �� ���������� ��������
		old_font=(HFONT)SelectObject(memdc1, hfont);

		for(j=0;j<3;j++)
			for(i=0;i<BKB_KBD_NUM_CELLS;i++)
			{
				if(NULL!=BKBKeybLayouts[screen_num][j][i].label) // ��������, ��� ��� �� NULL
				if(1==strlen(BKBKeybLayouts[screen_num][j][i].label))
				TextOut(memdc1,int(cell_size*0.4+i*cell_size) , int(cell_size/3+j*cell_size),
					BKBKeybLayouts[screen_num][j][i].label,1);
			}
	
		// ���������� ������ ����
		SelectObject(memdc1, old_font);

		// ����� �� ����� break !!! ����� �������� ���� ������ ��� ������!!!

	case 1:
		BitBlt(memdc2,0,0,width,height,memdc1,0,0,SRCCOPY);

		// ��������, �������� Progress Bar
		if(fixation_approved)
		{
			RECT rect;
		
			rect.left=(LONG)(cell_size/20+column*cell_size);
			rect.right=(LONG)(rect.left+percentage*cell_size*90/100/100);
			rect.top=(LONG)(cell_size/20+cell_size*row);
			rect.bottom=(LONG)(rect.top+cell_size/20); 

			FillRect(memdc2,&rect,blue_brush);
		}

		// ������ ������ ����� �����
		BLENDFUNCTION bfn;
		bfn.BlendOp = AC_SRC_OVER;
		bfn.BlendFlags = 0;
		bfn.SourceConstantAlpha = 255;
		bfn.AlphaFormat = AC_SRC_ALPHA;

		//AlphaBlend(memdc2, 0, 0, 100, 64, BufferDC, 0, 0, 100, 64, bfn);
		AlphaBlend(memdc2,whitespot_point.x-50,whitespot_point.y-50,100,100,whitespot_dc,0,0,100,100,bfn);

		// ����� �� ����� break !!! ����� ������� ���� ������ ��� ������!!!
	case 2:
		BitBlt(hdc,0,0,width,height,memdc2,0,0,SRCCOPY);
		break;
	}

	// ���� ���� DC - ����� ���
	if(release_dc) ReleaseDC(Kbhwnd,hdc);

}


//===========================================================================
// ������, ��� ����� ������ �� ������, �������� �������� �������
//===========================================================================
bool BKBKeybWnd::ProgressBar(POINT *p, int fixation_count, int _percentage)
{
	percentage=_percentage;

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
		if(fixation_approved) 
		//if((2==flag_using_airmouse)&&fixation_approved) // �������� ���� ����. � ������ � ��������� (��������) �����������, 
		// ����� ������ ����� �� ������� �������, � ������� �������� ��������
		// ��� ��������� ������� ��������� ��� ��
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
		// ������ ��� �� � WM_PAINT
		// �������� ������ 2 �� 1
		LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);
		
	/*	if(redraw_state>1) 
		{
			redraw_state=1; //����� ��� ���. ����� �������� ����������� � ������� ����
			//OutputDebugString("rstate->1 PB\n");
		}	 */

		// �� ������� ������ ����� ������ �������� InvalidateRect
		// ������ �����������, ������ ���� ������ ��������� ���� �������
		if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
		//InvalidateRect(Kbhwnd,NULL,TRUE);
	}

	return true;
}

//===========================================================================
// �� �������� ������, ��� ������������� �� ������ �����
//===========================================================================
void BKBKeybWnd::ProgressBarReset()
{
	fixation_approved=false;
	// �������� ������ 2 �� 1
	LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);

	/* if(redraw_state>1) 
	{
		redraw_state=1; //����� ��� ���. ����� �������� ����������� � ������� ����
		OutputDebugString("rstate->1 PBreset\n");
	} */
	// �� ������� ������ ������ �������� InvalidateRect
	// ������ �����������, ������ ���� ������ ��������� ���� �������
	if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
	//InvalidateRect(Kbhwnd,NULL,TRUE);
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
			}
			break;

		case scancode: // ����� ����� ����� ������������ Alt,Shift,Ctrl; � ���� - ���
			ScanCodeButton(key_pressed.bkb_vscancode);
			break;

		case leftkbd: // ������ ����� ���������� (����� �� ��������)
			screen_num--;
			if(screen_num<0) screen_num=2;
			break;

		case rightkbd: // ������ ����� ���������� (������ �� ��������)
			screen_num++;
			if(screen_num>2) screen_num=0;
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
			break;

		case control: // ������ ������ Ctrl
			if(ctrl_pressed) ctrl_pressed=false; else ctrl_pressed=true;
			break;

		case alt: // ������ ������ Alt
			if(alt_pressed) alt_pressed=false; else alt_pressed=true;
			break;

		case fn: // ������ ������ Fn
			if(Fn_pressed) Fn_pressed=false; else Fn_pressed=true;
			break;
		}
		// ���� �����, �����, ��������, ������� � ��������� �������

		// ��������, ��� ������� ����� ��������� �������
		row_pressed=row; column_pressed=column;
		SetTimer(Kbhwnd,4,500,0); // ����������
		// ������ ������ �������������� ���������� ����� ������ �������
		redraw_state=0;
		// �� ������� ������ ������ �������� InvalidateRect
		PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
		//InvalidateRect(Kbhwnd,NULL,FALSE); // ������������ ���������� � ������ ������
		//OutputDebugString("row_pressed\n");
	} //fixation approved

	// ���������� ������, ��������� �� ��� ������� � ���, ���� ���� ������ �� ������
	if(p->y>=start_y) 	return true;
	else return false;
}

//===========================================================================
// ���������� ������ ����� �������
//===========================================================================
void BKBKeybWnd::WhiteSpot(POINT *p)
{
	// ��, �� �� ���. ���� ���������, ����� ���� ������� �����, ������� ���� ������
	//if(p->y<start_y-50) return; // ����� ����� �� ������� �� ����������

	whitespot_point=*p;
	whitespot_point.y-=start_y; // ����� ������� ������� �������� ��������� � �������

	LONG old_state=InterlockedCompareExchange(&redraw_state,1,2);

	// �� ������� ������ ������ �������� InvalidateRect
	// ������ �����������, ������ ���� ������ ��������� ���� �������
	if(2==old_state) PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
/*	if(redraw_state>1) 
	{
		redraw_state=1; //����� ��� ���. ����� �������� ����������� � ������� ����
		OutputDebugString("rstate->1 WS\n");
	} 
		// �� ������� ������ ������ �������� InvalidateRect
	PostMessage(Kbhwnd, WM_USER_INVALRECT, 0, 0);
	//InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ���������� */
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
	
	//if(redraw_reqired) InvalidateRect(Kbhwnd,NULL,TRUE); // ������������ ����������
}

//===================================================================
// ������ ������� ���� (����������, ���� ��� ��� �������� ����)
// ����������� ��� memdc � hbm
//===================================================================
void BKBKeybWnd::OnSize(HWND hwnd, int _width, int _height)
{
	HDC hdc=GetDC(hwnd);

	width=_width;
	height=_height;

	// ������� DC
	if(memdc1!=0) DeleteDC(memdc1);
	if(memdc2!=0) DeleteDC(memdc2);
	
	// ������� �������
	if(hbm1!=0) DeleteObject(hbm1);
	if(hbm2!=0) DeleteObject(hbm2);
	
	// ��������� DC � �������
	memdc1=CreateCompatibleDC(hdc);
	memdc2=CreateCompatibleDC(hdc);

	hbm1=CreateCompatibleBitmap(hdc,width,height);
	hbm2=CreateCompatibleBitmap(hdc,width,height);

	SelectObject(memdc1,hbm1);
	SelectObject(memdc2,hbm2);

	ReleaseDC(hwnd,hdc);

	redraw_state=0; // ��������� ��������� ���� ����
}

//===================================================================
// ������ ������ � ������, ��� � ��������
//===================================================================
void BKBKeybWnd::CreateWhiteSpot(HWND hwnd)
{
	HDC hdc=GetDC(hwnd);
	whitespot_dc=CreateCompatibleDC(hdc);
	whitespot_bitmap=CreateCompatibleBitmap(hdc,100,100);

	// ������ ������ � �����-�������

	BITMAPINFO BufferInfo;
	ZeroMemory(&BufferInfo,sizeof(BITMAPINFO));
	BufferInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BufferInfo.bmiHeader.biWidth = 100;
	BufferInfo.bmiHeader.biHeight = 100;
	BufferInfo.bmiHeader.biPlanes = 1;
	BufferInfo.bmiHeader.biBitCount = 32;
	BufferInfo.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* pArgb;

	whitespot_bitmap = CreateDIBSection(whitespot_dc, &BufferInfo, DIB_RGB_COLORS,
                                       (void**)&pArgb, NULL, 0);

	// �������� �����-�����
	int i,j,alpha,distance_squared;
	for(i=0;i<100;i++)
	{
		for(j=0;j<100;j++)
		{
			// ��� ������ �� ������, ��� ���������� (� ��������)
			distance_squared=(i-50)*(i-50)+(j-50)*(j-50);
			//alpha=255-distance_squared/10; if(alpha<0) alpha=0; 
			alpha=100-distance_squared/24; if(alpha<0) alpha=0; 
			pArgb[i*100+j].rgbBlue=alpha;
			pArgb[i*100+j].rgbGreen=alpha;
			pArgb[i*100+j].rgbRed=alpha;
			pArgb[i*100+j].rgbReserved = alpha;
		}
	}
		
	//============================

	SelectObject(whitespot_dc,whitespot_bitmap);

	SetBkMode(whitespot_dc,TRANSPARENT);
	//TextOut(whitespot_dc,20,20,"KKK",3);
	
}


