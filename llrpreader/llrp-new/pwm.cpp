#include "pwm.h"
#include "stdlib.h"

using namespace std;

PWM* PWM::spInstance = 0;

PWM::PWM()
{

    fp = fopen("/sys/class/pwm/pwmchip1/export", "w");
    if( !fp )
    {
        perror("error open export file");
    }
    fprintf(fp, "0");
    fclose(fp);   
}

PWM::~PWM()
{}

void
PWM::period(int val)
{
    int ret;

	fp = fopen("/sys/class/pwm/pwmchip1/pwm0/period", "w");
    if( !fp )
    {
        perror("error open period file");
    }
    ret = fprintf(fp, "%d", val);
    if (ret < 0)
    {
        perror("set_gpio_val:");
    }
	fclose(fp);
}

void
PWM::duty_cycle(int val)
{
    int ret;

	fp = fopen("/sys/class/pwm/pwmchip1/pwm0/duty_cycle", "w");
    if( !fp )
    {
        perror("error open duty_cycle file");
    }
    ret = fprintf(fp, "%d", val);
    if (ret < 0)
    {
        perror("duty_cycle:");
    }
	fclose(fp);
}

void
PWM::enable(int val)
{
    int ret;

	fp = fopen("/sys/class/pwm/pwmchip1/pwm0/enable", "w");
    if( !fp )
    {
        perror("error open enable file");
    }
    ret = fprintf(fp, "%d", val);
    if (ret < 0)
    {
        perror("enable:");
    }
	fclose(fp);
}

PWM
*PWM::getInstance
(
)
{
	if ( 0 == spInstance)
	{
		spInstance = new PWM();
	}

	return( spInstance);

} // PWM

