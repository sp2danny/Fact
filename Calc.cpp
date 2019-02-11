
#include <iostream>
#include <cmath>

using namespace std;

const int width  = 1280;
const int height = 960;

const float zoomf = 1.01;

const float ipt_1 = 1.0f / (width*height);

float test_one(int n)
{
	float zf = pow(zoomf, n-1);
	float neww = width  * zf;
	float newh = height * zf;
	float ipt = n / (neww * newh);
	return ipt;
}

int main()
{
	float last = ipt_1, bst = ipt_1;
	bool isup = false;
	int over = 0, best = 1;
	for (int i=5; i<1000; ++i)
	{
		float ipt = test_one(i);
		cout << " num " << i << " ipt " << (ipt/ipt_1) << endl;
		if (ipt > bst)
		{
			bst = ipt;
			best = i;
		}
		if (ipt > last)
		{
			isup = true;
		} else {
			if (isup)
				over += 1;
		}
		last = ipt;
		if (over > 150) break;
	}
	cout << " Best : " << best << endl;
}

