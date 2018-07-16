//============================================================================
// Name        : smartthings_device.cpp
// Author      : dallago
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <cmath>
#include <unistd.h>
#include "bb_pwm.h"
#include <pthread.h>
#include "network/SocketServer.h"

using namespace std;
using namespace rd;
using namespace exploringBB;

void configure_IO(void);
string parse_incoming_command(string msg);
float get_temp(void);


bb_pwm my_pwm;
bb_pwm servo_pwm;

void *threadServer(void *value)
{
	char temp[100];
	SocketServer server(54321);
	string returnmsg;

	while(1)
	{
		server.listen();
		string rec = server.receive(1024);
		cout << "Received from the client [" << rec << "]" <<" at address "<< endl;
		string message("HTTP/1.0 200 OK\r\n");

		returnmsg = parse_incoming_command(rec);

		server.send("HTTP/1.0 200 OK\r\n");
		server.send("Content-Type: application/json\r\n");
		server.send("\r\n"); //need this for end of header
		//sprintf(temp, "{\"Temperature\":\"%s\"}\"","600");
		message = returnmsg;
		server.send(message);

		server.DisconnectClient();
		while(server.isClientConnected());
	}

}

int main() {

	int i;
	int period, ServoPeriod;

	pthread_t serverThread;

	if(pthread_create(&serverThread, NULL, &threadServer, &i))
	{
		cout<<"Could not create the tread"<<endl;
		return 1;
	}
	period = 100; //100 nA
	ServoPeriod = 20000000; //20mS

	cout << "Starting program" << endl;

	configure_IO();
	my_pwm.set_working_dir("/sys/devices/platform/ocp/48300000.epwmss/48300200.pwm/pwm/pwmchip1/pwm0/");
	servo_pwm.set_working_dir("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip3/pwm0/");

	my_pwm.set_period(period);
	my_pwm.set_duty_cycle(50);
	cout<< "enable PWM"<<endl;
	my_pwm.enable_pwm(1);

	servo_pwm.set_period(ServoPeriod);
	servo_pwm.set_duty_cycle(5);
	servo_pwm.enable_pwm(1);

	usleep (1000*2000);

	for(i=0;i<=4;i++)
	{
		my_pwm.set_duty_cycle(period/2 - 10*i);
		usleep(1000*50);
	}
#if 0
	usleep (1000*5000);

	cout<< "Disable PWM"<<endl;
	my_pwm.enable_pwm(0);
#endif
	cout<<"server Ready"<<endl;
	while(1)
	{
		usleep(1000*500);
	}

	return 0;
}

void configure_IO(void)
{
	string pwm_22_path = "/sys/devices/platform/ocp/ocp:P9_22_pinmux/state";
	string pwm0_export = "/sys/devices/platform/ocp/48300000.epwmss/48300200.pwm/pwm/pwmchip1/export";

	string pwm_14_path = "/sys/devices/platform/ocp/ocp:P9_14_pinmux/state";
	string pwm14_export = "/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip3/export";

	fstream fs;

	string gpio_dir_path = "/sys/class/gpio/gpio60/direction";

	//Configure P9_22 as pwm
	fs.open(pwm_22_path, fstream::out);
	fs<<"pwm";
	fs.close();
	//export the PWM0
	fs.open(pwm0_export, fstream::out);
	fs<<"0";
	fs.close();

	//Configure P9_14 as pwm
	fs.open(pwm_14_path, fstream::out);
	fs<<"pwm";
	fs.close();
	//export the PWM0
	fs.open(pwm14_export, fstream::out);
	fs<<"0";
	fs.close();

	//Configure GPIO60 as output.
	fs.open(gpio_dir_path, fstream::out);
	fs<<"out";
	fs.close();
}

string parse_incoming_command(string message)
{
	string light;
	string pwmval;
	string freqval;

	float temperature;
	string returnmsg;
	double dbltemp;
	int int_pwmVal, int_freqVal, int_angle;
	int baseperiod = 100; //100nS

	int location, loc2;
	fstream fs;
	string gpio_value_path = "/sys/class/gpio/gpio60/value";

	location = 0;
	location = message.find("LIGHT=");
	returnmsg="NULL";

	if(location > 0)
	{
		fs.open(gpio_value_path,fstream::out);
		loc2 = message.find("=ON");
		if((int)message.find("=ON") >= 0)
		{
			cout<<"Turn the Light On "<< endl;
			returnmsg = "{\"LIGHT\":\"ON\"}" ;
			fs<<"1";

		}
		else
		{
			cout<<"Turn the light off "<< endl;
			fs<<"0";
			returnmsg = "{\"LIGHT\":\"OFF\"}" ;
		}
		fs.close();
	}
	location = message.find("PWM=");
	if(location > 0)
	{
		location = location+4;
		loc2 = message.find("@DONE");
		pwmval = message.substr(location, loc2-location);
		cout<<"Set the duty cycle to "<<pwmval<<endl;

		int_pwmVal = stoi(pwmval);
		my_pwm.set_duty_cycle(int_pwmVal);
		returnmsg = "{\"PWM_duty\":\""+ pwmval +"\"}" ;
		cout<<returnmsg;

	}
	location = message.find("FREQ=");
	if(location > 0)
	{
		location = location+5;
		loc2 = message.find("@DONE");
		freqval = message.substr(location, loc2-location);
		cout<<"Set the Period to "<<freqval<<endl;

		int_freqVal = stoi(freqval);

		dbltemp = (double)(1.0/int_freqVal);
		cout<<"double vairale is: "<<dbltemp<<endl;

		int_pwmVal = (int) (dbltemp/1e-9);
		cout<<"truncated variable is: "<<int_pwmVal<<endl;

		cout<<"Desired frequency: "<<1/int_freqVal<<" actual freq: "<<1/int_pwmVal<<endl;
		my_pwm.set_period(int_pwmVal);

		returnmsg = "{\"PWM_period\":\""+ std::to_string(int_freqVal) +"\"}" ;
		cout<<returnmsg;

	}
	location = message.find("PWMEN=");
	if(location > 0)
	{
		cout<<"The PWM has been modified"<<endl;
		location = location+6;
		loc2 = message.find("@DONE");
		pwmval= message.substr(location, loc2-location);

		if((int)message.find("=ON") >= 0)
		{
			cout<<"Turn the PWM On "<< endl;
			my_pwm.enable_pwm(1);

		}
		else
		{
			cout<<"Turn the PWM Off "<< endl;
			my_pwm.enable_pwm(0);
		}

		returnmsg = "{\"PWM_en\":\""+ pwmval +"\"}" ;
		cout<<returnmsg;

	}
	location = message.find("SERVO=");
	if(location > 0)
	{


		location = location+6;
		loc2 = message.find("@DONE");
		pwmval = message.substr(location, loc2-location);
		cout<<"Set the servo duty cycle to "<<pwmval<<endl;

		int_pwmVal = stoi(pwmval);

		//Convert the PWM value back into an angle +/-90.
		int_angle = ((int_pwmVal-600000)/10000)-90;
		cout<<"Servo Command found Int_PWMval="<<int_pwmVal<<endl;
		int_pwmVal = (int)(((double)(int_pwmVal))/20000000.0 *100);


		cout<<"The converted PWMvalue is:  "<<int_pwmVal<<endl;
		cout<<"The calculate angle is:  "<<int_angle<<endl;

		servo_pwm.set_duty_cycle(int_pwmVal);
		returnmsg = "{\"SERVO_duty\":\""+ to_string(int_angle) +"\"}" ;
		cout<<returnmsg;

	}
	location = message.find("TEMP=");
	if(location >0)
	{
		location = location+4;
		loc2 = message.find("@DONE");
		temperature = get_temp();
		returnmsg = "{\"TEMP\":\""+ std::to_string(temperature) +"\"}" ;
	}
	location = message.find("PLAY=");
	if(location >0)
	{
		location = location+4;
		loc2 = message.find("@DONE");
		system("mplayer -ao alsa:device=hw=1 /home/debian/bin/res/TestAudio.MP3" ); //Copy the file to this locaion.
		returnmsg = "{\"PLAY\":\"FILEPLAYED\"}" ;
	}

	return returnmsg;

}

