#include <fstream>
#include <string>
#include <iostream>

#include "bb_pwm.h"
using namespace std;

namespace rd {

void bb_pwm::set_working_dir(string path)
{
	this->pwm_dir = path;
}

void bb_pwm::enable_pwm(int value)
{
	fstream fs;
	string filepath = this->pwm_dir + "enable";
	fs.open(filepath, fstream::out);
	fs<< value;
	fs.close();
	this->enable = value;

}
void bb_pwm::set_period(int value)
{
	fstream fs;
	bool test;

	string filepath = this->pwm_dir + "period";
	this->period = value;

	test = false; //check whether the new frequency will be less than the current duty cycle
	cout<<"current duty: "<<this->duty_cycle<<" Current period: "<<this->period<<endl;
	if(value < this->duty_cycle)
	{
		test = true;
		this->set_duty_cycle(this->duty_cycle_percent);
	}
	fs.open(filepath, fstream::out);
	fs<< std::to_string(this->period);
	fs.close();

	if(!test){
		this->set_duty_cycle(this->duty_cycle_percent);
	}

}
void bb_pwm::set_duty_cycle(int value)
{
	fstream fs;
	double temp;
	string filepath = this->pwm_dir + "duty_cycle";
	this->duty_cycle_percent = value;
	temp = (double)value/100;
	this->duty_cycle =(int)(this->period * temp);

	fs.open(filepath, fstream::out);
	fs << std::to_string(this->duty_cycle);
	fs.close();

}
bb_pwm::~bb_pwm() {

}

}//namespace
