/*
	Program : Thermocouple Calibration, Voltage to Temperature estimator
	Author : Siddharth Goswami (siddharth1024)
	Date : 26th May 2017
*/

#include <iostream>
#include <cmath>
#include <assert.h>
#include <fstream>
#include <ctime>
#include <string.h>
#include <conio.h>

#define t0 0
#define v0 1
#define p1 2
#define p2 3
#define p3 4
#define p4 5
#define q1 6
#define q2 7
#define q3 8
#define vMin 9
#define vMax 10
#define vStep 0.0001
#define debugDetailLevel 1000000
#define maxAnswers 5
#define invalidInputRange 5000
#define invalidOutputRange 10000

using namespace std;

int compareDoubles(double d1, double d2, int precision) {
	return (abs(d1 - d2) < pow(10, -precision));
}

double diff(double d1, double d2) {
	return abs(d1 - d2);
}

double isBetterValue(double d1, double d2, double actual) {
	return abs(d1 - actual) < abs(d2 - actual);
}

int main() {
	
	double coe[3][11] = {
    //        t0 (0),         v0 (1),          p1(2),         p2 (3),         p3 (4),         p4 (5),         q1 (6),         q2 (7),         q3 (8), vMin(9), vMax(10)
	 {-1.2147164e+02, -4.1790858e+00, +3.6069513e+01, +3.0722076e+01, +7.7913860e+00, +5.2593991e-01, +9.3939547e-01, +2.7791285e-01, +2.5163349e-02, -06.404, -03.554} // -250 to -100C
	,{-8.7935962e+00, -3.4489914e-01, +2.5678719e+01, -4.9887904e-01, -4.4705222e-01, -4.4869203e-02, +2.3893439e-04, -2.0397750e-02, -1.8424107e-03, -03.554, +04.096} // -100 to 100C
	,{+3.1018976e+02, +1.2631386e+01, +2.4061949e+01, +4.0158622e+00, +2.6853917e-01, -9.7188544e-03, +1.6995872e-01, +1.1413069e-02, -3.9275155e-04, -04.096, +16.397} // 100 to  400C	
	};
	
	assert( compareDoubles(0.39540, 0.39549, 4) == 1 ); assert( compareDoubles(0.39540, 0.39549, 5) == 0 );
	assert( compareDoubles(-220.004, -220, 3) == 0 ); assert( compareDoubles(-220.004, -220, 2) == 1 );
	
	std::time_t time = std::time(0);
	
	int n = 0; char useFile = 0;
	
	cout << "\nDo you want to read inputs from input.txt (Y/N)? : ";
	cin >> useFile;
	useFile = (useFile == 'y' || useFile == 'Y') ? 1 : 0;
	
	cout << "\nEnter number of inputs : ";
	cin >> n;
	
	assert(("Invalid input for N", n > 0));
	
	if (useFile) {		
		freopen("input.txt", "r", stdin);
	}	
	
	double(*in) = new double[n];
	
	int k = 0;	
	
	for(k = 0; k < n; k++) {
		cin >> in[k];
		if (in[k] > invalidInputRange || in[k] < -invalidInputRange) {
			cout << "ERROR : "<< in[k] << " is an invalid input, quitting..";
			exit(0);
		}
	}
	
	cout << "\nObtained " << k << " inputs | firstInput : " << in[0] << " | lastInput : " << in[k-1] << "\n";
	
	double(*out)[maxAnswers] = new double[n][maxAnswers];

	for(int k = 0; k < n; k++)
	{
		int i = 0, rIndex = -1;  
		double t = in[k], lastPredictedT = -1e12, lastV = -1e12;
			
		if(t >= -250.0 && t <= -100.000) rIndex = 0;
		if(t >= -100.1 && t <= +100.000) rIndex = 1;
		if(t >= +100.1 && t <= +400.000) rIndex = 2;
		//if(t >= +400.1 && t <= +800.000) rIndex = 3;
		//if(t >= +800.1 && t <= +1200.000) rIndex = 4;
		
		assert(rIndex != -1);
		
		int y = 1;
		for(double v = coe[rIndex][vMin]; v < coe[rIndex][vMax] ; v += vStep)
		{	
			double 	vv0_1 = ( v - coe[rIndex][v0] ), 
					vv0_2 = pow(vv0_1, 2),
					vv0_3 = pow(vv0_1, 3),
					vv0_4 = pow(vv0_1, 4);
			
			double predictedT = ((coe[rIndex][p1] * vv0_1 + 
								 coe[rIndex][p2] * vv0_2 + 
								 coe[rIndex][p3] * vv0_3 + 
								 coe[rIndex][p4] * vv0_4
								) 
								/ 
								(1 + 
								   (coe[rIndex][q1] * vv0_1) + 
								   (coe[rIndex][q2] * vv0_2) + 
								   (coe[rIndex][q3] * vv0_3)
								)) + coe[rIndex][t0];
								
			if( compareDoubles(predictedT, t, 2) )
			{
				if(isBetterValue(predictedT, lastPredictedT, t))
				{
					out[k][i] = 0.0;			
				}
				if (diff(v, lastV) > 0.5 && lastV != -1e12) 
				{ 
					i++;
				}
				lastPredictedT = predictedT;
				lastV = v;
			}
			if( i == maxAnswers - 1) 
				break;
		}
		
		if( i != 0 ) {
			for(int u = i; u < maxAnswers; u++) {
				out[k][u] = 0;
			}
		}		
	}
	
	char writeToFile = 1;
	
	if(writeToFile) {
		ofstream outFile;
		time = std::time(0);
		string fileName = "out-" + std::to_string(time) + ".csv";
		outFile.open (fileName);
		outFile << "Input T, Output V1, Output V2, Output V3, Output V4" << "\n";
		
		for(int k = 0; k < n; k++)
		{
			outFile << in[k] << ", " << out[k][0] << ", " << out[k][1] << ", " << out[k][2] << ", "	<< out[k][3] << "\n";
		}
		cout << "\n\nSUCCESS: Output written to : " << fileName << "\n\n";
		outFile.close();
	}
	
	cout << "\n\nPress any key to exit...";	
	getch();
	
	return 0;
}