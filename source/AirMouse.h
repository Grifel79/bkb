// �������������� (���������������) ����� ������ Tobii REX
#ifndef __BKB_AIRMOUSE
#define __BKB_AIRMOUSE

class BKBAirMouse
{
public:
	static int Init(HWND hwnd); // ������������� ������ � �����������
	static int Halt(HWND hwnd); // ���������� ������ � �����������
	static void OnTimer();
protected:
	static bool initialized;
};

#endif