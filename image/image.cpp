/***********************************************************
image
����̨��ʾͼƬ Ver 3.5 by Byaidu
�������뼰��������:https://github.com/Byaidu/image
���ִ���ο�:https://github.com/YinTianliang/CAPIx
************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <windows.h>
#include <gdiplus.h>
#include <wchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <map>
#include "regionmgr.cpp"
using namespace std;
using namespace Gdiplus;

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define DLL_EXPORT __declspec(dllexport)
#define wtoi _wtoi
#define wcsicmp _wcsicmp
#define match(x,y) if (wstring(argv[x])==wstring(y)||wstring(argv[x]).substr(0,2)==wstring(y).substr(0,2))
#define matchclsid(x,y) if (wstring(argv[x]).substr(wstring(argv[x]).length()-3)==wstring(y))
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"GdiPlus.lib")

struct imageres { //�����ṹ��
	HDC dc;
	HBITMAP oldbmp;
	int w, h;
	BUF region; //ͼԪ������
	imageres() {};
	imageres(wchar_t *file) //��ʼ��������������ͼԪ��������
	{
		BITMAP bi;
		//Ϊ�˷�ֹ���hbmpͬʱ��һ��hdc������ͻ��������������е�hbmp������Ե�hdc
		dc = CreateCompatibleDC(nullptr);
		//HBITMAP bmp = (HBITMAP)LoadImageA(nullptr, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		HBITMAP bmp;
		Bitmap *bm = new Bitmap(file);
		bm->GetHBITMAP(0, &bmp);
		delete bm;
		oldbmp = (HBITMAP)SelectObject(dc, bmp);
		GetObject(bmp, sizeof(BITMAP), &bi);
		w = bi.bmWidth;
		h = bi.bmHeight;
	}
	void regioninit(wchar_t *tag,int w,int h) {region = BUF(tag, w - 1, h - 1);}
}*hTarget;
map<wstring, imageres> resmap; //����ӳ���
HWND hCMD; //����̨���ھ��
double scale; //У�����ű�

void image(wchar_t *); //������
void Init_image(); //��ʼ��

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//HookAPI(SetEnvironmentVariableW, SetCall_image);
		DisableThreadLibraryCalls(hModule);
		Init_image();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}
extern "C" DLL_EXPORT int WINAPI Init(void)//��������������,�������
{
	return 0;
}
extern "C" __declspec(dllexport) void call(wchar_t *varName, wchar_t *varValue)
{
	//�жϱ������Ƿ�Ϊimage, �������image
	if (!wcsicmp(varName, L"image")) image(varValue);
	return;
}

void Init_image() //��ʼ��
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	imageres hRes;
	//��ȡcmd��С�Լ���ͼ���
	hCMD = GetConsoleWindow();
	HDC hDC = GetDC(hCMD);
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
	int ax = dm.dmPelsWidth;
	int bx = GetSystemMetrics(SM_CXSCREEN);
	scale = (double)ax / bx;//У�����ű�
	RECT rc;
	GetClientRect(hCMD, &rc);
	hRes.dc = hDC;
	hRes.w = int(scale*(rc.right - rc.left));
	hRes.h = int(scale*(rc.bottom - rc.top));
	hRes.regioninit((wchar_t*)L"cmd", hRes.w, hRes.h);
	resmap[L"cmd"] = hRes; //��cmd��ӵ�����ӳ�����
	hTarget = &resmap[L"cmd"];//getres("cmd"); //��ͼĬ��ָ��cmd
	//��ȡdesktop��С�Լ���ͼ���
	hDC = GetDC(nullptr);
	hRes.dc = hDC;
	hRes.w = dm.dmPelsWidth;
	hRes.h = dm.dmPelsHeight;
	hRes.regioninit((wchar_t*)L"desktop", hRes.w, hRes.h);
	resmap[L"desktop"] = hRes; //��desktop��ӵ�����ӳ�����

	TextOutA(hTarget->dc, 0, 0, 0, 0);//��һ��ʹ��TextOutA��Ч������Ǹ�bug
	return;
}
imageres * getres(wchar_t *tag) //���һ���
{
	if (!resmap.count(tag)) //����ڻ���ӳ������Ҳ��������ȼ���ͼԪ���������ٽ�������ӵ�����ӳ���
	{
		imageres hRes(tag);
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes;
	}
	return &resmap[tag];
}
void delres(wchar_t *tag) //����ԭ���Ļ�������ֹ�ڴ�й©
{
	imageres * hRes = getres(tag);
	HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
	DeleteObject(bmp);
	DeleteDC(hRes->dc);
	resmap.erase(tag);
	return;
}
//������SelectObject��ȡcmd�����⻭����hbitmap������Ҫ����һ�ݳ�����ע��ʹ��֮��ҪDeleteObject
HBITMAP copyhbitmap(imageres *hSrc)
{
	imageres hRes;
	hRes.dc = CreateCompatibleDC(hSrc->dc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hSrc->dc, hSrc->w, hSrc->h);
	hRes.oldbmp = (HBITMAP)SelectObject(hRes.dc, hBitmap);
	BitBlt(hRes.dc, 0, 0, hSrc->w, hSrc->h, hSrc->dc, 0, 0, SRCCOPY);
	SelectObject(hRes.dc, hRes.oldbmp);
	DeleteDC(hRes.dc);
	return hBitmap;
}
void rotateres(wchar_t **argv)
{
	imageres * hRes = getres(argv[1]);
	HBITMAP hSrc = copyhbitmap(hRes);
	Rect rect(0, 0, hRes->w, hRes->h);
	//���ڼ��ؾ�λͼ
	Bitmap bitmap(hSrc, nullptr);
	BitmapData bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);
	byte* pixels = (byte*)bitmapData.Scan0;
	//���ڼ�����λͼ
	Bitmap bitmap2(hSrc, nullptr);
	BitmapData bitmapData2;
	bitmap2.LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData2);
	byte* pixels2 = (byte*)bitmapData2.Scan0;
	//��ת
	double pi = 3.1415926;
	double angle = -(double)wtoi(argv[2]) / 180 * pi;
	double sina = sin(angle), cosa = cos(angle);
	int cx = hRes->w / 2, cy = hRes->h / 2;
	for (int i = 0; i<hRes->w; i++)
		for (int j = 0; j<hRes->h; j++)
		{
			int x = (int)(cx + (i - cx)*cosa - (j - cy)*sina), y = (int)(cy + (i - cx)*sina + (j - cy)*cosa);//ԭ����
			if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels2[j*bitmapData2.Stride + 3 * i + k] = pixels[y*bitmapData.Stride + 3 * x + k];
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels2[j*bitmapData2.Stride + 3 * i + k] = 0xFF;
			}
		}
	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	//������ʱ������Ŀ�껭��
	HDC hDCMem = CreateCompatibleDC(hRes->dc);
	HBITMAP hBitmap;
	bitmap2.GetHBITMAP(0, &hBitmap);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
	BitBlt(hRes->dc, 0, 0, hRes->w, hRes->h, hDCMem, 0, 0, SRCCOPY);
	//������ʱ���ƵĻ���
	DeleteObject(hSrc);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}
void alphares(wchar_t **argv)
{
	double alpha = (double)wtoi(argv[5])/255;
	//���ڼ���Դλͼ
	imageres * hRes = getres(argv[1]);
	HBITMAP hSrc = copyhbitmap(hRes);
	Rect rect(0, 0, hRes->w, hRes->h);
	Bitmap bitmap(hSrc, nullptr);
	BitmapData bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);
	byte* pixels = (byte*)bitmapData.Scan0;
	//���ڼ���Ŀ��λͼ
	//����SelectObject��ȡcmd�����⻭����hbitmap������Ҫ����һ�ݳ�����ע��ʹ��֮��ҪDeleteObject
	HBITMAP hSrc2 = copyhbitmap(hTarget);
	Rect rect2(0, 0, hTarget->w, hTarget->h);
	Bitmap bitmap2(hSrc2, nullptr);
	BitmapData bitmapData2;
	bitmap2.LockBits(&rect2, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData2);
	byte* pixels2 = (byte*)bitmapData2.Scan0;
	//���ڼ�����λͼ
	Rect rect3(0, 0, hTarget->w, hTarget->h);
	Bitmap bitmap3(hSrc2, nullptr);
	BitmapData bitmapData3;
	bitmap3.LockBits(&rect3, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData3);
	byte* pixels3 = (byte*)bitmapData3.Scan0;
	//alpha���
	int cx = wtoi(argv[2]), cy = wtoi(argv[3]);
	for (int i = 0; i<hTarget->w; i++)
		for (int j = 0; j<hTarget->h; j++)
		{
			int x = i - cx, y = j - cy;//Դ����
			if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels3[j*bitmapData3.Stride + 3 * i + k] =
					(byte)((1 - alpha) * pixels2[j*bitmapData2.Stride + 3 * i + k] +
						alpha * pixels[y*bitmapData.Stride + 3 * x + k]);
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels3[j*bitmapData3.Stride + 3 * i + k] = pixels2[j*bitmapData2.Stride + 3 * i + k];
			}
		}
	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	bitmap3.UnlockBits(&bitmapData3);
	//������ʱ������Ŀ�껭��
	HDC hDCMem = CreateCompatibleDC(hTarget->dc);
	HBITMAP hBitmap;
	bitmap3.GetHBITMAP(0, &hBitmap);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
	BitBlt(hTarget->dc, 0, 0, hTarget->w, hTarget->h, hDCMem, 0, 0, SRCCOPY);
	//������ʱ���ƵĻ���
	DeleteObject(hSrc);
	DeleteObject(hSrc2);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}

void image(wchar_t *CmdLine)
{
	//wcout << CmdLine << endl;
	int argc;
	wchar_t **argv;
	argv = CommandLineToArgvW(CmdLine, &argc);
	match(0, L"help")
	{
		printf(
			"image\n"
			"����̨��ʾͼƬ Ver 3.5 by Byaidu\n"
		);
	}
	match(0, L"load") //����ͼԪ��ͬ���������ٽ�����������ӳ���
	{
		wchar_t *tag = argv[1]; //��������
		//����ԭ���Ļ�������ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes(argc > 2 ? argv[2] : argv[1]);
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes;
	}
	match(0, L"unload") //ж�ػ���
	{
		//����ԭ���Ļ�������ֹ�ڴ�й©
		delres(argv[1]);
	}
	match(0, L"save") //����ΪͼƬ
	{
		imageres * hRes = getres(argv[1]);
		HBITMAP hSrc = copyhbitmap(hRes);
		Rect rect(0, 0, hRes->w, hRes->h);
		Bitmap bitmap(hSrc, nullptr);
		//https://stackoverflow.com/questions/1584202/gdi-bitmap-save-problem
		CLSID Clsid;
		matchclsid(2, L"bmp") CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid(2, L"jpg") CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid(2, L"png") CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		bitmap.Save(argv[2], &Clsid, nullptr);
		DeleteObject(hSrc);
	}
	match(0, L"target") //���Ļ�ͼĿ��
	{
		hTarget = getres(argv[1]);
	}
	match(0, L"buffer") //�½�һ��buffer����
	{
		wchar_t *tag = argv[1];
		//����ԭ���Ļ�������ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		hRes.dc = CreateCompatibleDC(hTarget->dc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hTarget->dc,argc>2?wtoi(argv[2]):hTarget->w,argc>3?wtoi(argv[3]):hTarget->h);
		hRes.oldbmp = (HBITMAP)SelectObject(hRes.dc, hBitmap);
		int color = argc>6?RGB(wtoi(argv[4]),wtoi(argv[5]),wtoi(argv[6])):RGB(255,255,255);
		colorregion(hRes.dc, color, 0, 0, hTarget->w - 1, hTarget->h - 1);
		hRes.w = hTarget->w;
		hRes.h = hTarget->h;
		//��buffer��ӵ��������ñ���
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes;
	}
	match(0, L"resize") //����
	{
		imageres * hRes = getres(argv[1]);
		match(1,L"cmd")
		{
			//��ֹ���ٱ༭����ˢ��ͼ��
			// ��ȡ��׼��������豸���  
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
			DWORD oldConMode;
			GetConsoleMode(hIn, &oldConMode); // ����
			SetConsoleMode(hIn, (oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)&(~ENABLE_QUICK_EDIT_MODE));
			//���ع��
			CONSOLE_CURSOR_INFO cursor_info = { (DWORD)25, FALSE };
			SetConsoleCursorInfo(hOut, &cursor_info);

			RECT rc,rc2;
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			GetClientRect(hCMD, &rc);
			GetWindowRect(hCMD, &rc2);
			int w = (rc2.right - rc2.left) - (rc.right - rc.left) + int((wtoi(argv[2])) / scale);
			int h = (rc2.bottom - rc2.top) - (rc.bottom - rc.top) + int((wtoi(argv[3])) / scale);
			//printf("scale:%f\n", scale);
			//printf("C:%dx%d\n", rc.right - rc.left, rc.bottom - rc.top);
			//printf("W:%dx%d\n", rc2.right - rc2.left, rc2.bottom - rc2.top);
			MoveWindow(hCMD, rc2.left, rc2.top, w, h, 0);
			Sleep(10);
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			Sleep(10);
			hRes->w = (int)wtoi(argv[2]);
			hRes->h = (int)wtoi(argv[3]);
		}else{
			HDC hDCMem = CreateCompatibleDC(hRes->dc);
			HBITMAP hBitmap = CreateCompatibleBitmap(hRes->dc, wtoi(argv[2]), wtoi(argv[3]));
			HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
			StretchBlt(hDCMem, 0, 0, wtoi(argv[2]), wtoi(argv[3]), hRes->dc, 0, 0, hRes->w, hRes->h, SRCCOPY);
			//����ԭ���Ļ�������ֹ�ڴ�й©
			HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
			DeleteObject(bmp);
			DeleteDC(hRes->dc);
			//�滻ԭ���Ļ���
			hRes->oldbmp = oldbmp;
			hRes->dc = hDCMem;
			hRes->w = wtoi(argv[2]);
			hRes->h = wtoi(argv[3]);
		}
		hRes->regioninit(argv[1], hRes->w, hRes->h);
	}
	match(0, L"cls")
	{
		//����������cmdͼ���ͼԪ������
		imageres * hRes = getres((wchar_t*)L"cmd");
		hRes->regioninit((wchar_t*)L"cmd", hRes->w, hRes->h);
		InvalidateRect(hCMD, nullptr, true);
		UpdateWindow(hCMD);
		Sleep(10);
	}
	match(0, L"rotate")
	{
		rotateres(argv);
	}
	match(0, L"draw")
	{
		//ֱ����Ŀ���ϻ�ͼ
		imageres * hRes = getres(argv[1]);
		//��ǰ���Ŀ�������ͼԪ�������ṹ���Ա�������ͼԪ������������һ��ʹͼԪ��������ø�����
		BUF clearbuf(L"", hRes->region.p->x2, hRes->region.p->y2);
		complexupdate(clearbuf.p, 0, 0, hRes->region.p->x2, hRes->region.p->y2, wtoi(argv[2]), wtoi(argv[3]), hTarget->region.p);
		complexupdate(hRes->region.p, 0, 0, hRes->region.p->x2, hRes->region.p->y2, wtoi(argv[2]), wtoi(argv[3]), hTarget->region.p);
		if (argc == 4)
		{
			BitBlt(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, SRCCOPY);
		}
		else
		{
			match(4, L"trans")
				TransparentBlt(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, hRes->w, hRes->h, argc==8?RGB(wtoi(argv[5]),wtoi(argv[6]),wtoi(argv[7])):RGB(255, 255, 255));
			match(4, L"alpha")
				alphares(argv);
		}
	}
	match(0, L"text")
	{
		//��ʾ���βŻ�ˢ�³���������Ǹ�bug
		for (int i = 0; i < 2;i++) TextOutW(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), argv[1], wcslen(argv[1]));
	}
	match(0, L"font")
	{
		SetBkMode(hTarget->dc, TRANSPARENT);
		SetTextColor(hTarget->dc, RGB(wtoi(argv[1]), wtoi(argv[2]), wtoi(argv[3])));
		if (argc > 4)
		{
			HFONT hFont = CreateFontW(
				argc > 5 ? wtoi(argv[5]) : 0,
				argc > 4 ? wtoi(argv[4]) : 0,
				argc > 6 ? wtoi(argv[6]) : 0/*���ù�*/,
				argc > 7 ? wtoi(argv[7]) : 0/*���ù�*/,
				argc > 8 ? wtoi(argv[8]) : 400/*һ�����ֵ��Ϊ400*/,
				argc > 9 ? wtoi(argv[9]) : 0/*����б��*/,
				argc > 10 ? wtoi(argv[10]) : 0/*�����»���*/,
				argc > 11 ? wtoi(argv[11]) : 0/*����ɾ����*/,
				DEFAULT_CHARSET, //��������ʹ��Ĭ���ַ��������������� _CHARSET ��β�ĳ�������
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, //���в������ù�
				DEFAULT_QUALITY, //Ĭ���������
				FF_DONTCARE, //��ָ��������*/
				argc > 12 ? argv[12] : L"������" //������
			);
			SelectObject(hTarget->dc, hFont);
		}
	}
	match(0, L"sleep")
	{
		Sleep(wtoi(argv[1]));
	}
	match(0, L"info")
	{
		wchar_t info[100];
		imageres * hRes = getres(argv[1]);
		swprintf(info, L"%d %d", hRes->w, hRes->h);
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"export")
	{
		wchar_t info[100];
		swprintf(info, L"%d", (int)hCMD);
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"import")
	{
		wchar_t *tag = argv[1];
		//����ԭ���Ļ�������ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		//��ȡcmd��С�Լ���ͼ���
		HWND hCMD2 = (HWND)wtoi(argv[2]);
		HDC hDC = GetDC(hCMD2);
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
		int ax = dm.dmPelsWidth;
		int bx = GetSystemMetrics(SM_CXSCREEN);
		double scale = (double)ax / bx;//У�����ű�
		RECT rc;
		GetClientRect(hCMD2, &rc);
		hRes.dc = hDC;
		hRes.w = (int)ceil(scale*(rc.right - rc.left));
		hRes.h = (int)ceil(scale*(rc.bottom - rc.top));
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes; //��cmd��Ϊ������ӵ����ñ���
	}
	match(0, L"getpix")
	{
		wchar_t info[100];
		COLORREF color=GetPixel(hTarget->dc, wtoi(argv[1]), wtoi(argv[2]));
		swprintf(info, L"%d %d %d", GetRValue(color), GetGValue(color), GetBValue(color));
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"setpix")
	{
		SetPixel(hTarget->dc, wtoi(argv[1]), wtoi(argv[2]), RGB(wtoi(argv[3]), wtoi(argv[4]), wtoi(argv[5])));
	}
	match(0, L"list")
	{
		bool skip = 0;
		if (argc > 2) skip = 1;
		ifstream in(argv[1]);
		string str;
		wchar_t wstr[100];
		while (!in.eof())
		{
			getline(in, str);
			MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, sizeof(wstr));
			if (skip&&L":" + wstring(argv[2]) == wstring(wstr)) { skip = 0; continue; }
			if (skip) continue;
			if (wstring(L"exit") == wstring(wstr)) break;
			//wcout << wstring(wstr) << endl;
			image(wstr);
		}
		in.close();
	}
	match(0, L"mouse")
	{
		imageres *hRes = getres((wchar_t*)L"cmd");
		wchar_t info[100];
		POINT mosPos;
		int x, y;
		int timer = wtoi(argv[1]);
		//����Ҫ��������һ�Σ�Ҫ��ȻReadConsoleInput�Ῠס
		// ��ȡ��׼��������豸���  
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		DWORD oldConMode;
		GetConsoleMode(hIn, &oldConMode); // ����
		SetConsoleMode(hIn, (oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)&(~ENABLE_QUICK_EDIT_MODE));
		if (timer < 0)
		{
			INPUT_RECORD	Rec;
			DWORD			res;
			while (1)
			{
				ReadConsoleInputW(hIn, &Rec, 1, &res);
				if (Rec.EventType == MOUSE_EVENT)
				{
					if (Rec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						GetCursorPos(&mosPos);
						ScreenToClient(hCMD, &mosPos);
						x = min(max((int)scale*mosPos.x, 0), hRes->w);
						y = min(max((int)scale*mosPos.y, 0), hRes->h);
						break;
					}
				}
				/*
				if (Rec.EventType == KEY_EVENT)
				{
					if (Rec.Event.KeyEvent.bKeyDown == 1)
					{

					}
				}
				*/
			}
		}
		if (timer >= 0)
		{
			DWORD tstart = GetTickCount();
			while (!(KEYDOWN(VK_LBUTTON) || int(GetTickCount() - tstart) >= timer)) Sleep(10);
			GetCursorPos(&mosPos);
			ScreenToClient(hCMD, &mosPos);
			x = min(max((int)scale*mosPos.x, 0), hRes->w);
			y = min(max((int)scale*mosPos.y, 0), hRes->h);
		}
		if (argc >= 3)
		{
			//��ָ����region�б��в���
			int ret = 0;
			for (int i = 2; i < argc; i++)
			{
				int x1, y1, x2, y2;
				swscanf(argv[i], L"%d,%d,%d,%d", &x1, &y1, &x2, &y2);
				if (x >= x1 && x <= x2 && y >= y1 && y <= y2) ret = i - 1;
			}
			swprintf(info, L"%d %d %d", x, y, ret);
			SetEnvironmentVariableW(L"image", info);
			swprintf(info, L"%d", ret);
			SetEnvironmentVariableW(L"errorlevel", info);
		}else{
			//��ͼԪ�������в���
			wstring ret = query(resmap[L"cmd"].region.p, x, y);
			swprintf(info, L"%d %d %s", x, y, ret.c_str());
			SetEnvironmentVariableW(L"image", info);
			swprintf(info, L"%s", ret.c_str());
			SetEnvironmentVariableW(L"errorlevel", info);
		}
		SetConsoleMode(hIn, oldConMode);
	}
	match(0, L"debug")
	{
		imageres *hRes = getres((wchar_t*)L"cmd");
		show(hRes->region.p);
	}
	match(0, L"union")
	{
		//�ϲ�ͼ���е�����ͼԪ��һ����ͼ��ͬ����ͼԪ��������ͼ���ͼԪ������
		imageres * hRes = getres(argv[1]);
		hRes->regioninit(argv[1], hRes->w, hRes->h);
	}
	match(0, L"cmd")
	{
		_wsystem(argv[1]);
	}
	//todo:֧��������ͬʱ����
	LocalFree(argv);
	return;
}
