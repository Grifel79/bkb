// �������������� � �������� Tobii REX
#ifndef __BKB_TOBIIREX
#define __BKB_TOBIIREX

class BKBTobiiREX
{
public:
	static int Init(); // ������������� ������ � �����������
	static int Halt(); // ���������� ������ � �����������
	//static int GetCoordinates();
protected:
	static bool initialized;
};

#endif