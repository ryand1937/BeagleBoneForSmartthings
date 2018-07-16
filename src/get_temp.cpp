#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>


using namespace std;

#define LDR_PATH "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

int readthermister(void)
{

	fstream fs;
	int number;

	fs.open(LDR_PATH, fstream::in);
	fs >>number;
	fs.close();
	return number;
}

float calc_temp(float res)
{
	float B = 3984;
	float T0n, Bn,R, ln_term;

	/* 1/T = 1/T0+1/B*ln(R/R0) */
	/* T0=298.15, */

	T0n = 1/298.15;
	Bn = 1/B;
	R = res/10000;
	ln_term = log(R);

	//cout<<"R "<<R<<" T0n "<<T0n<<" Bn "<<Bn<<" LNterm "<<ln_term<<endl;

	return (1/(T0n+Bn*ln_term))-273.15;

}
//Returns the Temperature, a 10K thermister is placed on Analog1
float get_temp(void)
{
	int i,j;
	int value, num_samples;
	float avg_value, avg_res;
	float Ftemp;

	//cout<<"Starting the readLDR program"<<endl;

	value = 0;
	num_samples = 100;

	value = 0;

	for(i=0;i<num_samples;i++)
	{
		value += readthermister();
	//	cout<<"The Value was " <<value<<endl;
	}

	avg_value = value/num_samples;
	//cout<<"Average ADC value "<<avg_value<<endl;
	avg_res = (avg_value*10000)/(4096-avg_value);
	//cout<<"Average Resistance "<<avg_res<<endl;

	Ftemp = calc_temp(avg_res)*9/5 +32;

return Ftemp;
}
