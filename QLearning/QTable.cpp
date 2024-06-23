#include "QTable.h"

#undef min

float QTable::GetMaxQ(int x, int y)
{
	float max = std::numeric_limits<float>().lowest();
	int maxI = -1;
	for (int i = 0; i < 4; ++i)
	{
		if (table[y][x].dir[i] >= max)
		{
			maxI = i;
			max = table[y][x].dir[i];
		}
	}

	return table[y][x].dir[maxI];
}

void QTable::UpdateQValueRaw(int prevX, int prevY, int actionIdx, float reward)
{
	float& q = table[prevY][prevX].dir[actionIdx];
	float maxQ = GetMaxQ(prevX, prevY);

	// q = (1.0F - _a) * q + _a * (reward + _g * maxQ);
	q = reward;
}

int QTable::GetQValueIdx(int x, int y)
{
	float max = 0.0F;
	int maxI = 0;
	for (int i = 0; i < 4; ++i)
	{
		if (table[y][x].dir[i] > max)
		{
			maxI = i;
			max = table[y][x].dir[i];
		}
	}

	if (max > 0.0F)
		return maxI;

	for (;;)
	{
		int i = GetRandomValue(0, 3);
		if (table[y][x].dir[i] >= 0.0F)
			return i;
	}

	return 0;
}

XY QTable::GetIdx(int i)
{
	XY ret = { -1, 0 };

	if (i == 1)
		ret = { 0, -1 };
	else if (i == 2)
		ret = { 1, 0 };
	else if (i == 3)
		ret = { 0, 1 };

	return ret;
}

int QTable::GetRandomValue(int min, int max)
{
	static std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(min, max);

	return dis(gen);
}

void QTable::Create(int x, int y, float alpha, float gamma)
{
	table = new QState*[y];
	for (int i = 0; i < y; ++i)
	{
		table[i] = new QState[x];

		for (int j = 0; j < x; ++j)
		{
			table[i][j] = {};
		}
	}

	this->x = x;
	this->y = y;
	_a = alpha;
	_g = gamma;
}

void QTable::UpdateQValue(int prevX, int prevY, int x, int y, int actionIdx)
{
	float& q = table[prevY][prevX].dir[actionIdx];
	float maxQ = GetMaxQ(x, y);

	q = (1.0F - _a) * q + _a * (table[y][x].reward + _g * maxQ);
}
