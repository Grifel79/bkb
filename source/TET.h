// �������������� c TheEyeTribe 
#ifndef __BKB_TET
#define __BKB_TET

class BKBTET
{
public:
	static int Init(); // ������������� ������ � �����������
	static int Halt(); // ���������� ������ � �����������
protected:
	static bool initialized;
};

#endif