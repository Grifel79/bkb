#include <Windows.h>
#include <process.h>
#include "BKBRepErr.h"
#include "TobiiREX.h"
#include "Smooth.h"
#include "Fixation.h"
#include "TranspWnd.h"
#include "KeybWnd.h"

#define DISPERSION_LIMIT 100.0 // ��� ������������ ��������
#define DISPERSION_HIGH_LIMIT 300.0 // ��� ������������ ������� �����������
#define FIXATION_LIMIT 30 // ������� ���������������� ����� � ������ ���������� ������� ���������
#define POSTFIXATION_SKIP 30 // ������� ����� ���������� ����� ��������, ����� ������ ������� ����� ��������
#define CURSOR_SMOOTHING 7; // ����������� �������� ������� �������� ������ ��� � CURSOR_SMOOTHING ��������

// ������������ ����� �� Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"
#include "tobiigaze_config.h"

// ��� ������������ ��������� ���������
typedef  TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_config_init)(tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_config_get_default_eye_tracker_url)(char *url, uint32_t url_size, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API tobiigaze_eye_tracker* (TOBIIGAZE_CALL *type_tobiigaze_create)(const char *url, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_connect)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_start_tracking)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_gaze_listener gaze_callback, tobiigaze_error_code *error_code, void *user_data);


typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_stop_tracking)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_error_code *error_code);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_disconnect)(tobiigaze_eye_tracker *eye_tracker);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_break_event_loop)(tobiigaze_eye_tracker *eye_tracker);
typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_destroy)(tobiigaze_eye_tracker *eye_tracker);

typedef TOBIIGAZE_API const char* (TOBIIGAZE_CALL *type_tobiigaze_get_error_message)(tobiigaze_error_code error_code);

typedef TOBIIGAZE_API void (TOBIIGAZE_CALL *type_tobiigaze_run_event_loop)(tobiigaze_eye_tracker *eye_tracker, tobiigaze_error_code *error_code);

HMODULE TobiiConfigDLL=0, TobiiCoreDLL=0;

// ��������� �� ������� �� DLL
type_tobiigaze_config_init fp_tobiigaze_config_init;
type_tobiigaze_config_get_default_eye_tracker_url fp_tobiigaze_config_get_default_eye_tracker_url;
type_tobiigaze_create fp_tobiigaze_create;
type_tobiigaze_connect fp_tobiigaze_connect;
type_tobiigaze_start_tracking fp_tobiigaze_start_tracking;
type_tobiigaze_stop_tracking fp_tobiigaze_stop_tracking;
type_tobiigaze_disconnect fp_tobiigaze_disconnect;
type_tobiigaze_break_event_loop fp_tobiigaze_break_event_loop;
type_tobiigaze_destroy fp_tobiigaze_destroy;
type_tobiigaze_get_error_message fp_tobiigaze_get_error_message=0;
type_tobiigaze_run_event_loop fp_tobiigaze_run_event_loop;

// ������ ���������� ��� ������ � Tobii Gaze SDK ����� ������ �������� (static)
static tobiigaze_error_code tbg_error_code;
static char url[64];
static tobiigaze_eye_tracker* eye_tracker=0;

static uintptr_t tobii_thread_handler; // ������� ������ ��� Gaze SDK
extern int screenX, screenY;


static int fixation_count=0; // ���������� �����, ����� ���� ����� �� ���������
static int skip_count=0; // ������� ����� �������� ���������� ����� ��������, ����� ������ ������� ����� ��������

bool BKBTobiiREX::initialized(false);

//extern HWND	BKBhwnd;

//=====================================================================================
// �������, ������������ ���� ������ �����
//=====================================================================================
inline long signum(long x)
{
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

//=====================================================================================
// �������, ������� �������� REX, ����� �������� ������ � ������
// 01.02.04 Ÿ ����� �������� � ��������
//=====================================================================================
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data)
{
	HDC hdc;
	static POINT point_left={0,0}, point_right={0,0}, point={0,0}; //, last_point={0,0}, tmp_point;
	double disp1,disp2; // ��������� � ��������� �������� ������ � ������� �����
	static POINT  screen_cursor_point; //cursor_position={0,0};
	static double cursor_position_x, cursor_position_y;
	static int cursor_linear_move_counter=CURSOR_SMOOTHING; // ������� �������� ������ ����� ��������� ������� 
	static double cursor_speed_x=0.0, cursor_speed_y=0.0; 
	static uint64_t last_timestamp=0;
	
		// ��� �������� ������ ����� �� ������
	// �� ������ ���� ��������� ��� �����!!
	if (gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED)
	{
		//hdc=GetDC(BKBhwnd);

		// ������� ������ ����� 
		point_left.x=screenX*gazedata->left.gaze_point_on_display_normalized.x;
		point_left.y=screenY*gazedata->left.gaze_point_on_display_normalized.y;
		disp1=BKBSmooth(&point_left, 0);
		
		// ������� ������� ����� 
		point_right.x=screenX*gazedata->right.gaze_point_on_display_normalized.x;
		point_right.y=screenY*gazedata->right.gaze_point_on_display_normalized.y;
		disp2=BKBSmooth(&point_right, 1);
		
		point.x=(point_right.x+point_left.x)/2;
		point.y=(point_right.y+point_left.y)/2;
		
		//=================================================================================
		// ������ � ������������ �������
		if((disp1>DISPERSION_HIGH_LIMIT)&&(disp2>DISPERSION_HIGH_LIMIT))
		{
			// ������ ���������� ������
			//cursor_position=point;
			cursor_position_x=point.x;
			cursor_position_y=point.y;
			cursor_linear_move_counter=0; // � ��������� ����� ����� ����� ����������� ��������
		}
		else // ������ ���������� ����, ���� ������ ������ CURSOR_SMOOTHING (�����) ������� �����
		{
			// ���� �� ��������� ����� ����������� �������� �������?
			if(cursor_linear_move_counter>0) // ���, �� ����
			{
				cursor_linear_move_counter--;
			}
			else // ����� ����������� ��������
			{
				cursor_linear_move_counter=CURSOR_SMOOTHING;
				cursor_speed_x=(point.x-cursor_position_x)/(double)CURSOR_SMOOTHING;
				cursor_speed_y=(point.y-cursor_position_y)/(double)CURSOR_SMOOTHING;
			}
			
			// �������-���� ������ ����������, �������� cursor_position �� double
			cursor_position_x+=cursor_speed_x; // ����� ����� ��������� ����������, ���� ������
			cursor_position_y+=cursor_speed_y; 
		} // ����� ����������� �������


		// ��� ���������?
		screen_cursor_point.x=cursor_position_x+0.5; // ������ ��������� � POINT
		screen_cursor_point.y=cursor_position_y+0.5; // 0.5 ������������ ������ ���������� ��� �������� � �����	

		//===============================================================================================
		// ��������� �������
		if(BKB_MODE_SCROLL==Fixation::CurrentMode())
		{
			if(screen_cursor_point.y>screenY*3/4) // ������ ����
			{
				Fixation::Scroll(gazedata->timestamp-last_timestamp,-1);
			}
			else if(screen_cursor_point.y<screenY/4) // ������ �����
			{
				Fixation::Scroll(gazedata->timestamp-last_timestamp,1);
			}
			last_timestamp=gazedata->timestamp;
		}
		

		if((BKB_MODE_SCROLL!=Fixation::CurrentMode())&&(BKB_MODE_KEYBOARD!=Fixation::CurrentMode()))
				BKBTranspWnd::Move(screen_cursor_point.x,screen_cursor_point.y); // �������� ��� ������� �� �������
		
		if(BKB_MODE_KEYBOARD==Fixation::CurrentMode()) BKBKeybWnd::WhiteSpot(&screen_cursor_point);

		//===============================================================================================
		// ������ ��������, ������ ���� ��� ���������� �� ���������� ��������, ����� ��������� skip_count
		if(skip_count<=0)
		{
			if((disp1<DISPERSION_LIMIT)&&(disp2<DISPERSION_LIMIT)) 
			{
				fixation_count++;
				if(BKB_MODE_KEYBOARD==Fixation::CurrentMode())
				{
					// ���������� �������� ������� �� ����������
					POINT point_screen=point;
					//ClientToScreen(BKBhwnd,&point_screen); �������
					//BKBKeybWnd::ProgressBar(&point_screen,fixation_count,100*fixation_count/FIXATION_LIMIT);
					if(!BKBKeybWnd::ProgressBar(&point_screen,fixation_count,100*fixation_count/FIXATION_LIMIT))
					{
						fixation_count=0; // ������-������, �� ���, ���������
						BKBKeybWnd::ProgressBarReset();
					}
				}
			}
			else
			{
				fixation_count=0; // ������-������, �� ���, ���������
				if(BKB_MODE_KEYBOARD==Fixation::CurrentMode()) BKBKeybWnd::ProgressBarReset();
			}
		}
		else
		{
			skip_count--;
		}
		// �������� ������� �������� �������
		if(fixation_count>=FIXATION_LIMIT) 
		{
			fixation_count=0; // ������ ����� ������� ��� ����������
			skip_count=POSTFIXATION_SKIP;

			// ����� ������������ �������� � ����������� �� �������� ������.
			Fixation::Fix(screen_cursor_point);
			BKBTranspWnd::ToTop(); // ����� �������� ����� ������� ���� ����� ������� � z-order'e
		}


		
	}
}

//=====================================================================================
// �����, ��� ������� - ��������� ���� ��������� � Tobii Gaze SDK
//=====================================================================================
unsigned __stdcall TobiiREXThread(void *p)
{
	tobiigaze_error_code my_error_code;

    (*fp_tobiigaze_run_event_loop)((tobiigaze_eye_tracker*)eye_tracker, &my_error_code);
    if(my_error_code)
	{
		BKBReportError(my_error_code, __FILE__,"tobiigaze_run_event_loop",__LINE__);
	}
	return 0;
}


//=====================================================================================
// ������ ������ � �����������
//=====================================================================================
int BKBTobiiREX::Init()
{
	if(initialized) return 1; // ��� ����������������

	// -1. �������� DLL
	TobiiConfigDLL = LoadLibrary("TobiiGazeConfig32.dll");
	if(0==TobiiConfigDLL)
	{
		BKBReportError("�� ������� ��������� ���������� TobiiGazeConfig32.dll\r\n���������� � � ������� ������� ���������");
		return 1;
	}
	
	TobiiCoreDLL = LoadLibrary("TobiiGazeCore32.dll");
	if(0==TobiiCoreDLL)
	{
		BKBReportError("�� ������� ��������� ���������� TobiiGazeCore32.dll\r\n���������� � � ������� ������� ���������");
		return 1;
	}


	// 0. �������� ������� �� DLL
	// �������� ����� ��� ������ �������
	fp_tobiigaze_config_init=(type_tobiigaze_config_init)GetProcAddress(TobiiConfigDLL,"tobiigaze_config_init");
	fp_tobiigaze_config_get_default_eye_tracker_url=(type_tobiigaze_config_get_default_eye_tracker_url)GetProcAddress(TobiiConfigDLL,"tobiigaze_config_get_default_eye_tracker_url");
	fp_tobiigaze_create=(type_tobiigaze_create)GetProcAddress(TobiiCoreDLL,"tobiigaze_create");
	fp_tobiigaze_connect=(type_tobiigaze_connect)GetProcAddress(TobiiCoreDLL,"tobiigaze_connect");
	fp_tobiigaze_start_tracking=(type_tobiigaze_start_tracking)GetProcAddress(TobiiCoreDLL,"tobiigaze_start_tracking");
	fp_tobiigaze_stop_tracking=(type_tobiigaze_stop_tracking)GetProcAddress(TobiiCoreDLL,"tobiigaze_stop_tracking");
	fp_tobiigaze_disconnect=(type_tobiigaze_disconnect)GetProcAddress(TobiiCoreDLL,"tobiigaze_disconnect");
	fp_tobiigaze_break_event_loop=(type_tobiigaze_break_event_loop)GetProcAddress(TobiiCoreDLL,"tobiigaze_break_event_loop");
	fp_tobiigaze_destroy=(type_tobiigaze_destroy)GetProcAddress(TobiiCoreDLL,"tobiigaze_destroy");
	fp_tobiigaze_get_error_message=(type_tobiigaze_get_error_message)GetProcAddress(TobiiCoreDLL,"tobiigaze_get_error_message");
	fp_tobiigaze_run_event_loop=(type_tobiigaze_run_event_loop)GetProcAddress(TobiiCoreDLL,"tobiigaze_run_event_loop");

	if(!fp_tobiigaze_config_init||!fp_tobiigaze_config_get_default_eye_tracker_url||!fp_tobiigaze_create||
		!fp_tobiigaze_connect||!fp_tobiigaze_start_tracking||!fp_tobiigaze_stop_tracking||
		!fp_tobiigaze_disconnect||!fp_tobiigaze_break_event_loop||!fp_tobiigaze_destroy||
		!fp_tobiigaze_get_error_message||!fp_tobiigaze_run_event_loop)
	{
		BKBReportError("�� ������� �������� ����������� ������� �� TobiiGazeCore32.dll ��� TobiiGazeConfig32.dll");
		return 1;
	}

	// 1. ������� �� Tobii Gaze SDK 
	// 1.1.
	(*fp_tobiigaze_config_init)(&tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_config_init",__LINE__);
		return 1;
	}

	// 1.2.
	(*fp_tobiigaze_config_get_default_eye_tracker_url)(url, 64, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_config_get_default_eye_tracker_url",__LINE__);
		return 1;
	}

	// 1.3.
	eye_tracker = (*fp_tobiigaze_create)(url, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_create",__LINE__);
		return 1;
	}

	// 1.4. ����� ������ ����� ��� Gaze SDK
	tobii_thread_handler=_beginthreadex(NULL,0,TobiiREXThread,NULL,0,NULL);
	if(1>tobii_thread_handler)
	{
		BKBReportError(__FILE__,"start Tobii Gaze SDK loop thread",__LINE__);
		return 1;
	}

	// 1.5.
	(*fp_tobiigaze_connect)(eye_tracker, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_connect",__LINE__);
		return 1;
	}

	// � ������� �� SDK ����������� ��� ���������� ������ ����������. ������� �� �����.
    // print_device_info(eye_tracker)

	// 1.6.
    (*fp_tobiigaze_start_tracking)(eye_tracker, &on_gaze_data, &tbg_error_code, 0);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_start_tracking",__LINE__);
		return 1;
	}

	initialized=true;
	return 0; // ��������� ����������
}

//=====================================================================================
// ���������� ������ � �����������
//=====================================================================================
int BKBTobiiREX::Halt()
{
	if(!initialized) return 1; // ��� ��������� ������


	// ������� �� Tobii Gaze SDK
	(*fp_tobiigaze_stop_tracking)(eye_tracker, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_stop_tracking",__LINE__);
		initialized=false; // �� ������ ������
		return 1;
	}

    (*fp_tobiigaze_disconnect)(eye_tracker);
   
    (*fp_tobiigaze_break_event_loop)(eye_tracker);
   
    (*fp_tobiigaze_destroy)(eye_tracker);

	initialized=false;

	// ��������� DLL
	if(TobiiConfigDLL) FreeLibrary(TobiiConfigDLL);
	if(TobiiCoreDLL) FreeLibrary(TobiiCoreDLL);

	return 0; // ��������� ����������
}


/*
// ������ ������, �� ����������� ������������ DLL

//=====================================================================================
// ������ ������ � �����������
//=====================================================================================
int BKBTobiiREX::Init()
{
	if(initialized) return 1; // ��� ����������������

	// 1. ������� �� Tobii Gaze SDK 
	// 1.1.
	tobiigaze_config_init(&tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_config_init",__LINE__);
		return 1;
	}

	// 1.2.
	tobiigaze_config_get_default_eye_tracker_url(url, 64, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_config_get_default_eye_tracker_url",__LINE__);
		return 1;
	}

	// 1.3.
	eye_tracker = tobiigaze_create(url, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_create",__LINE__);
		return 1;
	}

	// 1.4. ����� ������ ����� ��� Gaze SDK
	tobii_thread_handler=_beginthreadex(NULL,0,TobiiREXThread,NULL,0,NULL);
	if(1>tobii_thread_handler)
	{
		BKBReportError(__FILE__,"start Tobii Gaze SDK loop thread",__LINE__);
		return 1;
	}

	// 1.5.
	tobiigaze_connect(eye_tracker, &tbg_error_code);
    if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_connect",__LINE__);
		return 1;
	}

	// � ������� �� SDK ����������� ��� ���������� ������ ����������. ������� �� �����.
    // print_device_info(eye_tracker)

	// 1.6.
    tobiigaze_start_tracking(eye_tracker, &on_gaze_data, &tbg_error_code, 0);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_start_tracking",__LINE__);
		return 1;
	}

	initialized=true;
	return 0; // ��������� ����������
}

//=====================================================================================
// ���������� ������ � �����������
//=====================================================================================
int BKBTobiiREX::Halt()
{
	if(!initialized) return 1; // ��� ��������� ������

	// ������� �� Tobii Gaze SDK
	tobiigaze_stop_tracking(eye_tracker, &tbg_error_code);
	if(tbg_error_code)
	{
		BKBReportError(tbg_error_code, __FILE__,"tobiigaze_stop_tracking",__LINE__);
		initialized=false; // �� ������ ������
		return 1;
	}

    tobiigaze_disconnect(eye_tracker);
   
    tobiigaze_break_event_loop(eye_tracker);
   
    tobiigaze_destroy(eye_tracker);

	initialized=false;
	return 0; // ��������� ����������
}
*/