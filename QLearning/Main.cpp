#pragma comment(linker,"/entry:WinMainCRTStartup") 
#pragma warning(disable: 4996)

#include <windows.h>
#include <atlimage.h>
#include <tchar.h>

#include <iostream>
#include <random>
#include <vector>
#include <string>

#include "resource.h"
#include "GameTimer.h"

#include "QTable.h"

#define SCREEN_WIDTH (1200)
#define SCREEN_HEIGHT (1000)

GameTimer timer;

struct Obstable
{
	int j, i;
	int x0, y0, x1, y1;
};

struct Player
{
	float radius;
	int x, y;
};

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Q Learing Practice";
LPCTSTR lpszWindowName = L"Q Learing Practice";

int G_ScreenWidth = 0;
int G_ScreenHeight = 0;
int ix, iy;
int size = 5;
float currTime, maxTime;
std::vector<Obstable> obstables = {};
Player player;
CImage playerImg;

QTable qTable;

SIZE SetText(HDC hdc, char* string, int fontWidth, int fontHeight, COLORREF textColor, COLORREF backColor, RECT rect, int flag)
{
	SIZE maxSize = {};

	std::vector<std::string> tokens;
	{
		char* result = strtok(string, "\n");
		while (result != NULL)
		{
			tokens.push_back(result);
			result = strtok(NULL, "\n");
		}

		/*if (tokens.size() > 1)
			flag = DT_LEFT | DT_TOP | DT_WORDBREAK;*/
	}

	HFONT hFont = CreateFontA(fontHeight, fontWidth, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, "¸¼Àº °íµñ");
	HFONT OldFont = (HFONT)SelectObject(hdc, hFont);

	SetTextColor(hdc, textColor);
	SetBkColor(hdc, backColor);

	int count = 0;
	int oldTop = rect.top;

	for (int i = 0; i < tokens.size(); ++i)
	{
		rect.top = oldTop + count;
		DrawTextA(hdc, tokens[i].data(), tokens[i].size(), &rect, flag);

		SIZE s = {};
		GetTextExtentPointA(hdc, tokens[i].data(), tokens[i].size(), &s);
		count += s.cy;

		maxSize.cx = max(maxSize.cx, s.cx);
		maxSize.cy = max(maxSize.cy, s.cy);
	}

	SelectObject(hdc, OldFont);
	DeleteObject(hFont);

	return maxSize;
}

bool IsInList(int x, int y)
{
	for (int i = 0; i < obstables.size(); ++i)
	{
		if (obstables[i].j == x && obstables[i].i == y)
			return true;
	}

	return false;
}

void AddObstable(int x, int y)
{
	Obstable t = {};

	t.j = x;
	t.i = y;
	t.x0 = x * ix;
	t.y0 = y * iy;
	t.x1 = (x + 1) * ix;
	t.y1 = (y + 1) * iy;
	qTable.table[y][x].reward = -1.0F;

	obstables.push_back(t);
}

void Initialize(HWND hwnd)
{
	RECT r = {};
	GetClientRect(hwnd, &r);

	G_ScreenWidth = r.right - r.left;
	G_ScreenHeight = r.bottom - r.top;
	ix = G_ScreenWidth / size;
	iy = G_ScreenHeight / size;

	player.x = ix * 0.5F;
	player.y = iy * 0.5F;
	player.radius = 50.0F;

	playerImg.Load(L"airplane.png");
	qTable.Create(size, size, 0.4F, 0.75F);
	
	currTime = 0.0F;
	maxTime = 1.0F;

	AddObstable(1, 2);
	AddObstable(1, 1);
	AddObstable(3, 2);
	AddObstable(3, 3);

	qTable.table[size - 1][size - 1].reward = 1.0F;

	timer.Reset();
	timer.Start();
}

void Update()
{
	timer.Tick();
	const float dt = timer.DeltaTime();
	currTime += dt;

	int px = player.x / ix;
	int py = player.y / iy;

	if (currTime >= maxTime)
	{
		currTime = 0.0F;

		int newIdx = qTable.GetQValueIdx(px, py);
		XY newVel = qTable.GetIdx(newIdx);

		XY newPos = { px + newVel.x, py + newVel.y };
		float reward = 0.0F;

		if (newPos.x == size - 1 && newPos.y == size - 1)
		{
			qTable.UpdateQValueRaw(px, py, newIdx, 1.0F);
			player.x = ix * 0.5F;
			player.y = iy * 0.5F;
			return;
		}
		else if (newPos.x < 0 || newPos.x > size - 1 || newPos.y < 0 || newPos.y > size - 1)
		{
			qTable.UpdateQValueRaw(px, py, newIdx, -1.0F);
			player.x = ix * 0.5F;
			player.y = iy * 0.5F;
			return;
		}
		else
		{
			qTable.UpdateQValue(px, py, newPos.x, newPos.y, newIdx);

			if (qTable.table[newPos.y][newPos.x].reward < 0.0F)
			{
				player.x = ix * 0.5F;
				player.y = iy * 0.5F;
				return;
			}
		}

		player.x = newPos.x * ix + (ix * 0.5F);
		player.y = newPos.y * iy + (iy * 0.5F);
	}
}

void Draw(HDC hdc)
{
	HPEN pen = CreatePen(PS_SOLID, 1, 0x00);
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	HBRUSH brush = CreateSolidBrush(0x5bb3dc);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);

	HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH red = CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH green = CreateSolidBrush(RGB(0, 255, 0));

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			SelectObject(hdc, white);
			if (IsInList(j, i))
				SelectObject(hdc, red);
			if (j == size - 1 && i == size - 1)
				SelectObject(hdc, green);

			Rectangle(hdc, j * ix, i * iy, (j + 1) * ix, (i + 1) * iy);
		}
	}

	playerImg.Draw(hdc, player.x - player.radius, player.y - player.radius, player.radius * 2, player.radius * 2);

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			for (int k = 0; k < 4; ++k)
			{
				float dx = ix * 0.25F;
				float dy = iy * 0.25F;

				RECT r = { j * ix + dx, i * iy + dy, (j + 1) * ix - dx, (i + 1) * iy - dy };
				XY vel = qTable.GetIdx(k);

				r.left += vel.x * ix * 0.25F;
				r.right += vel.x * ix * 0.25F;
				r.bottom += vel.y * iy * 0.25F;
				r.top += vel.y * iy * 0.25F;

				char buf[0x10] = "";
				sprintf_s(buf, "%.2f", qTable.table[i][j].dir[k]);

				SetText(hdc, buf, 15, 15, RGB(0, 0, 255), RGB(255, 255, 255), r, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
			}
		}
	}

	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
	DeleteObject(pen);
	DeleteObject(brush);
	DeleteObject(white);
	DeleteObject(red);
	DeleteObject(green);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	HBITMAP hBitmap;

	switch (uMsg)
	{
	case WM_CREATE:
		SetTimer(hWnd, 0, 1, 0);
		Initialize(hWnd);
		break;
	case WM_TIMER:
		if (wParam == 0)
		{
			Update();
			InvalidateRect(hWnd, nullptr, FALSE);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_40001:
			maxTime = 1.0F;
			break;
		case ID_40002:
			maxTime = 0.1F;
			break;
		case ID_40003:
			maxTime = 0.01F;
			break;
		}
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, G_ScreenWidth, G_ScreenHeight);
		SelectObject(mDC, (HBITMAP)hBitmap);
		Draw(mDC);
		BitBlt(hDC, 0, 0, G_ScreenWidth, G_ScreenHeight, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0);
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}