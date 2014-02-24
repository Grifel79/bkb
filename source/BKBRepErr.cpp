/*
*	�������� �� ������� ������� ����������
*/

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include "BKBRepErr.h"
#include "resource.h"

extern HINSTANCE	BKBInst;

typedef TOBIIGAZE_API const char* (TOBIIGAZE_CALL *type_tobiigaze_get_error_message)(tobiigaze_error_code error_code);
extern type_tobiigaze_get_error_message fp_tobiigaze_get_error_message;

//============================================================================================
// ���� message box � ���������
//============================================================================================
static char *header;
static char *body;

static char *timeout_chars[5]={"00","01","02","03","04"};


static BOOL CALLBACK DlgSettingsWndProc(HWND hdwnd,
						   UINT uMsg,
						   WPARAM wparam,
						   LPARAM lparam )
{
	static int timeout_counter;

	if (uMsg==WM_COMMAND)
	{
	switch (LOWORD(wparam))
		{
		case IDOK:
		case IDCANCEL:
			KillTimer(hdwnd,3);
			EndDialog(hdwnd,0);
			return 1;
		} // switch WM_COMMAND
	}// if WM_COMMAND 

	if (uMsg==WM_INITDIALOG)
	{
		SetWindowPos(hdwnd,NULL,100,100,0,0,SWP_NOSIZE);
		SetWindowText(hdwnd,header);
		SendDlgItemMessage(hdwnd,IDC_BODY, WM_SETTEXT, 0L, (LPARAM)body);
		SetTimer(hdwnd,3,1000,0);
		timeout_counter=5;
	}

	if (uMsg==WM_TIMER)
	{
		timeout_counter--;
		SendDlgItemMessage(hdwnd,IDC_TIMEOUT, WM_SETTEXT, 0L, (LPARAM)(timeout_chars[timeout_counter]));
		if(0>=timeout_counter)
		{
			KillTimer(hdwnd,3);
			EndDialog(hdwnd,0);
			return 1;
		}
	}
return 0;
}

int BKBMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	header=(char *)lpCaption;
	body=(char *)lpText;
	return DialogBox(BKBInst,MAKEINTRESOURCE(IDD_DIALOG_MB),hWnd,(DLGPROC)DlgSettingsWndProc);
}

//============================================================================================
// �������� � ��������� ������� ������� ����������
//============================================================================================
void BKBReportError(char *SourceFile, char *FuncName, int LineNumber)
{
	DWORD res;				// ��������� ������� FormatMessage
	void *BKBStringError;	// ��������� �� ������ ��� ��������� ��������� ������
	char BKBMessage[1024];	// ��� ������, � ������� ����������� ��������� �� ������
	DWORD BKBLastError=GetLastError(); // �������� ��� ��������� ������
	
	
	if (BKBLastError!=(DWORD)0) // �������� ������, ���� ��� �� ����� ����
	{
		res=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL, BKBLastError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &BKBStringError, 0, NULL );

		if(res==(DWORD)0) BKBStringError=(void *)"��������� �� ������ �� �������";
	}
	else
	{
		BKBStringError=(void *)"��� ��������� ������";
	}
	
	// ������������ ������ � ������ ��������� ������
	sprintf_s(BKBMessage, sizeof(BKBMessage),
			"Module: %s\nFunction: %s\nLine number: %d\nSysErr: %d (%s)",
			SourceFile, FuncName, LineNumber,
			BKBLastError, (char *)BKBStringError);


	//����������� ������, ������� �������� ������� FormatMessage
	if (BKBLastError!=(DWORD)0)
	{
			LocalFree( BKBStringError );
	}

	//�������� ��������� �� ������ (���� ��������, �� �����)
	BKBMessageBox(NULL,BKBMessage,"BKB: ��������� �� ������",MB_OK|MB_ICONINFORMATION );

	//� ����� � ���� 
	FILE *fout;
	fout=fopen("reperr.log","a");

	time_t mytime = time(0); /* not 'long' */

	fprintf(fout,"****\n%s%s\n", ctime(&mytime), BKBMessage);
	fflush(fout);
	fclose(fout);
}

//============================================================================================
// ��� ����������� ������ (�����������)
// ������� �� �����, ��� �� ��� ��������������
//============================================================================================
void BKBReportError(char *Error) 
{
	//�������� ��������� �� ������ (���� ��������, �� �����)
	BKBMessageBox(NULL,Error,"���������!",MB_OK|MB_ICONINFORMATION );
}

//============================================================================================
// ��� ������ Tobii Gaze SDK (�����������)
//============================================================================================
void BKBReportError(tobiigaze_error_code tbg_error_code, char *SourceFile, char *FuncName, int LineNumber)
{
	char BKBMessage[1024];	// ��� ������, � ������� ����������� ��������� �� ������

	if (tbg_error_code)
    {
		const char *tmp_char;
		if(fp_tobiigaze_get_error_message) tmp_char=(*fp_tobiigaze_get_error_message)(tbg_error_code);
		else tmp_char="����������";

		// ������������ ������ � ������ ��������� ������
		sprintf_s(BKBMessage, sizeof(BKBMessage),
			"Module: %s\nFunction: %s\nLine number: %d\n������ Tobii Gaze SDK: %d (%s)",
			SourceFile, FuncName, LineNumber,
			tbg_error_code, 
			tmp_char);


		//�������� ��������� �� ������ (���� ��������, �� �����)
		BKBMessageBox(NULL,BKBMessage,"BKB:Gaze SDK: ��������� �� ������",MB_OK|MB_ICONINFORMATION );

		//� ����� � ���� 
		FILE *fout;
		fout=fopen("reperr.log","a");

		time_t mytime = time(0); /* not 'long' */

		fprintf(fout,"****\n%s%s\n", ctime(&mytime), BKBMessage);
		fflush(fout);
		fclose(fout);
	}
}
