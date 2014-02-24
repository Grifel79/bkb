// ������ ��������� � (��������� � �����������) �������
#ifndef __BKB_REPERR
#define __BKB_REPERR

// ������������ ����� �� Tobii Gaze SDK
#include "tobiigaze_error_codes.h"
#include "tobiigaze.h"

void BKBReportError(char *SourceFile, char *FuncName, int LineNumber);
void BKBReportError(char *Error); // ��� ����������� ������ (�����������)
void BKBReportError(tobiigaze_error_code tbg_error_code, char *SourceFile, char *FuncName, int LineNumber); // ��� ������ Tobii Gaze SDK (�����������)


#endif