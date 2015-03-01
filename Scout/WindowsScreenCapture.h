#pragma once

#include <Windows.h>

HBITMAP CopyBitmap(HBITMAP hbm, HDC hdcSrc, int yStart) {
	HDC hdcDst = CreateCompatibleDC(NULL);
	HBITMAP hbmOld, hbmOld2, hbmNew;
	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);
	hbmOld = (HBITMAP)SelectObject(hdcSrc, hbm);
	hbmNew = CreateBitmap(bm.bmWidth, bm.bmHeight, bm.bmPlanes,
		bm.bmBitsPixel,
		NULL);
	hbmOld2 = (HBITMAP)SelectObject(hdcDst, hbmNew);
	BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, yStart, SRCCOPY);
	SelectObject(hdcSrc, hbmOld);
	ReleaseDC(NULL, hdcSrc);
	DeleteDC(hdcDst);
	return hbmNew;
}

void* ScreenGrabBlt(const char* windowName, int& outSize, double leadingIngore, double trailingIgnore)
{
	RECT rc;
	HWND hwnd = FindWindow(NULL, windowName);    //the window can't be min
	if (hwnd == NULL)
	{
		return 0;
	}
	GetClientRect(hwnd, &rc);

	LONG clientHeight = rc.bottom - rc.top;
	LONG skipTop = static_cast<LONG>(leadingIngore * clientHeight);
	LONG skipBottom = static_cast<LONG>(trailingIgnore * clientHeight);

	rc.top = rc.top + skipTop;
	rc.bottom = rc.bottom - skipBottom;

	//create
	HDC hdcScreen = GetDC(NULL);
	HDC hdc = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmp = CreateCompatibleBitmap(hdcScreen,
		rc.right - rc.left, rc.bottom - rc.top);

	HBITMAP obmp = CopyBitmap(hbmp, GetDC(hwnd), rc.top);

	BITMAP bmp;
	if (0 != GetObject(obmp, sizeof(BITMAP), &bmp))
	{
		int bigBps = bmp.bmBitsPixel / 8;
		outSize = bmp.bmHeight * bmp.bmWidth * bigBps;
		char* bigBytes = new char[outSize];
		GetBitmapBits(obmp, outSize, bigBytes);

		//copy to clipboard
		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, obmp);
		CloseClipboard();

		//release
		DeleteObject(hbmp);
		DeleteObject(obmp);
		DeleteDC(hdc);
		DeleteObject(obmp);
		ReleaseDC(NULL, hdcScreen);
		return bigBytes;
	}
	else
	{
		//release
		DeleteObject(hbmp);
		DeleteObject(obmp);
		DeleteDC(hdc);
		DeleteObject(obmp);
		ReleaseDC(NULL, hdcScreen);

		return nullptr;
	}
}