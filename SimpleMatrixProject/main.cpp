#include<iostream>
#include<vector>
#include<string>
#include<chrono>
#include"SimpleTimer.h"
#include"SimpleMatrix.h"

using namespace std;

int main(int argc, char* argv[])
{
	my::SimpleTimer<chrono::microseconds> timer;

	timer.start();
	my::SimpleMatrix<int> mtx(3, 3, 1); // 3201 mc

	int counter = 0;
	for (int& i : mtx)
	{
		i = ++counter;
	}

	timer.stop();
	timer.log_curr_time();

	cout << mtx << endl;

	system("pause");
	return 0;
}