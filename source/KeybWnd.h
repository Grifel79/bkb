#ifndef __BKB_KEYBWND
#define __BKB_KEYBWND

#define BKB_KBD_NUM_CELLS 15

// ���� ������ �� ����������
typedef enum {undefined=0,scancode,unicode,shift,control,alt,leftkbd,rightkbd,fn
	} BKB_KEY_TYPE;

// ������� �� ����������
typedef struct
{
	BKB_KEY_TYPE bkb_keytype; // ��� �������
	WORD bkb_vscancode; // ������ (��� ������� ������)
	WORD bkb_unicode; // ������ (��� ���������)
	WORD bkb_unicode_uppercase; // ������ ����� � ������� ��������
	char *label; // ��������: ��� ������ �� �������
} BKB_key;


class BKBKeybWnd
{
public:
	static void Init();
	static bool FixPoint(POINT *pnt);
	static void OnPaint(HDC hdc=0);
	static bool IsItYours(POINT *p);
	static bool ProgressBar(POINT *p, int fixation_count, int percentage); // ���������� false, ���� ��������� � ������� (��� ��������)
	static void ProgressBarReset();
	static void Activate();
	static void DeActivate();

protected:
	static void ScanCodeButton(WORD scancode);
	static HWND Kbhwnd;
	static int screen_x, screen_y, start_y;
	static float cell_size;
	static int current_pane;

	static POINT start_point;
	static bool fixation_approved;
	static int row, column;

	static bool shift_pressed, ctrl_pressed, alt_pressed, caps_lock_pressed, Fn_pressed;
	static int screen_num;
};

#endif