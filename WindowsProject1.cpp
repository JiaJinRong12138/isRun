// WindowsProject1.cpp : 定义应用程序的入口点。
//

#define  _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <string>
#include <tlhelp32.h>
#include <stdint.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#define IN

using namespace std;
using namespace cv;

//声明函数
int GetProcess(IN char* lpName);
int isSimilar(string strSrcImageName, string strSrcImageName1);
void ShootScreen(const char* filename, HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	int count = 0;
	
	//判断是否存在该进程
	/*
	*@IN :进程名称
	*/
	if (GetProcess((char*)("chrome.exe"))) {
		
		if (MessageBox(NULL, TEXT("该程序在执行"), TEXT("Test"), MB_OKCANCEL) == IDCANCEL) {
			MessageBeep(MB_ICONASTERISK);
			return 0;
		}
		//如果count == 4 则表示一段时间内画面没有变化，进行提示
		while (TRUE) {
			if (count == 4) {
				if (MessageBox(NULL, TEXT("当前程序可能出现假死状态，是否进行执行"), TEXT("Test"), MB_OKCANCEL) == IDCANCEL) {
					return 0;
				}
			}
			HWND hWnd = FindWindow(TEXT("Intermediate D3D Window"), NULL);
			for (int i = 0; i < 3; i++) {
				char fileName[100];
				sprintf(fileName, "D:\\img\\img%d.bmp", i);
				ShootScreen(fileName, hWnd);
				Sleep(20 * 1000);
				//对图片进行比较
			}
			for (int i = 0; i < 2; i++) {
				string path1, path2;
				path1 = "D:\\img\\img" + to_string(i) + ".bmp";
				path2 = "D:\\img\\img" + to_string(i + 1) + ".bmp";
				int a = isSimilar(path1, path2);
				if (a <= 5) {
					//非常相似，判断是否为同一张图片
					MessageBox(NULL, TEXT("两张图片很相似"), TEXT("Test"), MB_OK);
					count++;
				}
				else if (a > 5 && a <= 10) {
					MessageBox(NULL, TEXT("两张图片部分相似"), TEXT("Test"), MB_OK);
				}
				else {
					MessageBox(NULL, TEXT("两张图片不一样"), TEXT("Test"), MB_OK);
				}
			}
		}
		
	}
	else {
		MessageBox(NULL, TEXT("未发现该程序"), TEXT("Test"), MB_OK);
	}

	return 0;
}

int isSimilar(string strSrcImageName, string strSrcImageName1) {
	/*string strSrcImageName = "D:\\img\\b.jpg";
	string strSrcImageName1 = "D:\\img\\a.jpg";*/
	cv::Mat matSrc, matSrc0, matSrc1, matSrc2;

	//加载两张图片
	matSrc = cv::imread(strSrcImageName, IMREAD_COLOR);
	matSrc0 = cv::imread(strSrcImageName1, IMREAD_COLOR);

	//将图片缩放
	cv::resize(matSrc, matSrc1, cv::Size(1920, 1080), 0, 0, cv::INTER_NEAREST);
	cv::resize(matSrc0, matSrc2, cv::Size(1920, 1080), 0, 0, cv::INTER_LANCZOS4);

	//Test --- 显示两张图片
	/*imshow(strSrcImageName, matSrc1);
	waitKey(0);
	imshow(strSrcImageName1, matSrc2);
	waitKey(0);*/

	//定义图片数据
	cv::Mat matDst1, matDst2;

	//再次将图片进行缩放
	cv::resize(matSrc1, matDst1, cv::Size(16, 16), 0, 0, cv::INTER_CUBIC);
	cv::resize(matSrc2, matDst2, cv::Size(16, 16), 0, 0, cv::INTER_CUBIC);

	cv::cvtColor(matDst1, matDst1, CV_BGR2GRAY);
	cv::cvtColor(matDst2, matDst2, CV_BGR2GRAY);

	int iAvg1 = 0, iAvg2 = 0;
	int arr1[64], arr2[64];

	for (int i = 0; i < 8; i++) {
		uchar* data1 = matDst1.ptr<uchar>(i);
		uchar* data2 = matDst2.ptr<uchar>(i);

		int tmp = i * 8;

		for (int j = 0; j < 8; j++) {
			int tmp1 = tmp + j;

			arr1[tmp1] = data1[j] / 4 * 4;
			arr2[tmp1] = data2[j] / 4 * 4;

			iAvg1 += arr1[tmp1];
			iAvg2 += arr2[tmp1];
		}
	}

	iAvg1 /= 64;
	iAvg2 /= 64;

	for (int i = 0; i < 64; i++) {
		arr1[i] = (arr1[i] >= iAvg1) ? 1 : 0;
		arr2[i] = (arr2[i] >= iAvg2) ? 1 : 0;
	}

	int iDiffNum = 0;

	for (int i = 0; i < 64; i++)
		if (arr1[i] != arr2[i])
			++iDiffNum;

	/*
	*	0-5：相似
	*	5-10：部分相似
	*	>10：不相同
	*/
	return iDiffNum;

}



BOOL GetProcess(IN char* lpName) {
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	//打开指定的进程
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcess == INVALID_HANDLE_VALUE) {
		MessageBox(NULL, TEXT("Error at CreateToolhelp32Snapshot"), TEXT("Test"), MB_OK);
		return 0;
	}
	//枚举列表里的第一个进程
	BOOL codition = Process32First(hProcess, &pe32);
	while (codition) {
		char sz[1024];
		//将WCHAR* 转换为 char*
		sprintf_s(sz, "%ws", pe32.szExeFile);
		if (strcmp(sz, lpName) == 0) {
			CloseHandle(hProcess);
			return TRUE;
		}
		//读取下一个进程
		codition = Process32Next(hProcess, &pe32);
	}
	//关闭句柄
	CloseHandle(hProcess);
	return FALSE;
}


void ShootScreen(const char* filename, HWND hWnd)
{
	HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	int32_t ScrWidth = 0, ScrHeight = 0;
	RECT rect = { 0 };
	if (hWnd == NULL)
	{
		ScrWidth = GetDeviceCaps(hdc, HORZRES);
		ScrHeight = GetDeviceCaps(hdc, VERTRES);
	}
	else
	{
		GetWindowRect(hWnd, &rect);
		ScrWidth = rect.right - rect.left;
		ScrHeight = rect.bottom - rect.top;
	}
	HDC hmdc = CreateCompatibleDC(hdc);

	HBITMAP hBmpScreen = CreateCompatibleBitmap(hdc, ScrWidth, ScrHeight);
	HBITMAP holdbmp = (HBITMAP)SelectObject(hmdc, hBmpScreen);

	BITMAP bm;
	GetObject(hBmpScreen, sizeof(bm), &bm);

	BITMAPINFOHEADER bi = { 0 };
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bm.bmWidth;
	bi.biHeight = bm.bmHeight;
	bi.biPlanes = bm.bmPlanes;
	bi.biBitCount = bm.bmBitsPixel;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = bm.bmHeight * bm.bmWidthBytes;
	// 图片的像素数据
	char* buf = new char[bi.biSizeImage];
	BitBlt(hmdc, 0, 0, ScrWidth, ScrHeight, hdc, rect.left, rect.top, SRCCOPY);
	GetDIBits(hmdc, hBmpScreen, 0L, (DWORD)ScrHeight, buf, (LPBITMAPINFO)& bi, (DWORD)DIB_RGB_COLORS);
	


	BITMAPFILEHEADER bfh = { 0 };
	bfh.bfType = ((WORD)('M' << 8) | 'B');
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD dwWrite;
	WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWrite, NULL);
	WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWrite, NULL);
	WriteFile(hFile, buf, bi.biSizeImage, &dwWrite, NULL);
	CloseHandle(hFile);
	hBmpScreen = (HBITMAP)SelectObject(hmdc, holdbmp);
}



