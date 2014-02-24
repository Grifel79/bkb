#include <Windows.h>
#include "AirMouse.h"
#include "TranspWnd.h"

// ������������ ����� �� Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"
#include "tobiigaze_config.h"

// �������� callback-������� �� TobiiREX.cpp
void on_gaze_data(const tobiigaze_gaze_data* gazedata, void *user_data);

extern int screenX, screenY;
//============================================================================
// ��������� ������, �� �������� ����� �������� ���������� ������� �� Tobii
// 10 ��� � �������
//============================================================================
int BKBAirMouse::Init(HWND hwnd)
{
	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	SetTimer(hwnd,1,25,NULL);

	// �� ���������� ���������� ����, ��� ������ ��� ��������
	BKBTranspWnd::flag_show_transp_window=false;
	return 0;
}

//============================================================================
// ������� ������, �� �������� ����� �������� ���������� ������� �� Tobii
// 10 ��� � �������
//============================================================================
int BKBAirMouse::Halt(HWND hwnd)
{
	KillTimer(hwnd,1);
	return 0;
}

//============================================================================
// ��� ������������ ������� ��������� ������ �� Tobii REX
//============================================================================
void BKBAirMouse::OnTimer()
{
	tobiigaze_gaze_data gd;

	POINT p;
	GetCursorPos(&p);

	gd.tracking_status = TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED;
	gd.left.gaze_point_on_display_normalized.x=p.x/(double)screenX;
	gd.left.gaze_point_on_display_normalized.y=p.y/(double)screenY;

	gd.right.gaze_point_on_display_normalized.x=gd.left.gaze_point_on_display_normalized.x;
	gd.right.gaze_point_on_display_normalized.y=gd.left.gaze_point_on_display_normalized.y;

	gd.timestamp=1000UL*timeGetTime(); // ������������ ��������

	on_gaze_data(&gd, NULL);
}