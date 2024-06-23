#pragma once

#include <Windows.h>

#include <random>
#include <string>
#include <limits>

struct QState
{
	// left, up, right, down
	float dir[4];
	float reward;
};

struct XY
{
	int x, y;
};

class QTable
{
public:
	void Create(int x, int y, float alpha, float gamma);
	XY GetIdx(int i);
	int GetRandomValue(int min, int max);

	float GetMaxQ(int x, int y);
	int GetQValueIdx(int x, int y);

	void UpdateQValue(int prevX, int prevY, int x, int y, int actionIdx);
	void UpdateQValueRaw(int prevX, int prevY, int actionIdx, float reward);

public:
	int x, y;
	QState** table = nullptr;

private:
	float _a, _g;

};

