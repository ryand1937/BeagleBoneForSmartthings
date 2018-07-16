#ifndef BB_PWM_H_
#define BB_PWM_H_


#include <string>


namespace rd
{

class bb_pwm {
private:
	int	period;
    int	duty_cycle;
    int duty_cycle_percent;
    int	enable;
    std::string pwm_dir;

public:
   void set_working_dir(std::string dir);
   void set_period(int period);
   void set_duty_cycle(int duty_cycle);
   void enable_pwm(int enable);
   ~bb_pwm();
};

}

#endif /* BB_PWM_H_ */
