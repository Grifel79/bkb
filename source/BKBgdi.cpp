﻿#include <Windows.h>
#include "BKBgdi.h"

HPEN red_pen, green_pen, dkyellow_pen, pink_pen; // перья и кисти для рисования
HBRUSH dkblue_brush, dkblue_brush2, blue_brush;
HFONT hfont;

int screenX, screenY, mouscreenX, mouscreenY;

double screen_scale=1.0;

void BKBgdiInit()
{
	//  Кисти создаём
	red_pen=CreatePen(PS_SOLID,1,RGB(255,100,100));
	green_pen=CreatePen(PS_SOLID,1,RGB(100,255,100));
	dkyellow_pen=CreatePen(PS_SOLID,1,RGB(227,198,2));
	pink_pen=CreatePen(PS_SOLID,5,RGB(255,156,255));

	dkblue_brush=CreateSolidBrush(RGB(45,62,90));
	dkblue_brush2=CreateSolidBrush(RGB(100,72,100));
	blue_brush=CreateSolidBrush(RGB(188,199,216));

	hfont = CreateFont( -48, 0, 0, 0, FW_BOLD, 0, 0, 0,
		RUSSIAN_CHARSET,
		0, 0, 0, 0, L"Arial");
	

	// Получим разрешение экрана
	screenX=GetSystemMetrics(SM_CXSCREEN);
	screenY=GetSystemMetrics(SM_CYSCREEN);

	// Козлиная система разрешений экрана в windows8.1...
	DEVMODE dm;
	ZeroMemory (&dm, sizeof (dm));
	EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dm);

	screen_scale=((double)screenX)/dm.dmPelsWidth;

	// В windows 8 при HighDPI координаты курсора отличаются от координат точки на экране
	mouscreenX=dm.dmPelsWidth;
	mouscreenY=dm.dmPelsHeight;
}

void BKBgdiHalt()
{
	// удаляем кисти
	DeleteObject(red_pen);
	DeleteObject(dkyellow_pen);
	DeleteObject(green_pen);
	DeleteObject(pink_pen);

	DeleteObject(dkblue_brush);
	DeleteObject(blue_brush);

	DeleteObject(hfont);
}