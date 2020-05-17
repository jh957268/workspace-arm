#include "pwm.h"
#include "stdlib.h"

using namespace std;

PWM* PWM::spInstance = 0;
OwSemaphore* PWM::m_hSem = 0;

PWM::PWM():
	OwTask( HIGH, 2048, "PWM")
{

    fp = fopen("/sys/class/pwm/pwmchip2/export", "w");
    if( !fp )
    {
        perror("error open export file");
    }
    fprintf(fp, "0");
    fclose(fp);

	m_hSem = new OwSemaphore(1);
}

PWM::~PWM()
{}

void
PWM::main( OwTask *)
{

	int retval;

	printf ("PWM Task is running\n");

	period(20000000);   // 20ms, 50Hz
	//PWM::getInstance()->duty_cycle(2400000);   // 2.4ms, 12% will drive to 180 degree
	duty_cycle(1500000);   // 0.4ms, 2% will drive to 0 degree
	polarity("normal");

#if 0
	enable(1);
	//polarity("normal");
	OwTask::sleep(1000);

	int duty = 1500000;
	for (int i = 0; i < 21; i++)
	{
		duty_cycle(duty + (i * 50000));
		OwTask::sleep(200);
	}

	duty = 2500000;
	for (int i = 0; i < 21; i++)
	{
		duty_cycle(duty - (i * 50000));
		OwTask::sleep(200);
	}
	enable(0);
#endif

	do_open();
	do_close();

	while (1)
	{
		// OSEvtPend( &executor_evt_flag, 0x01, &evt_flag, EVENT_ANY, OS_WAIT);
		retval = semaphoreTake(PI_FOREVER);
		if (OK != retval)
		{
			printf("PWM semaphoreTake error");
			OwTask::sleep(2);
			continue;
		}
		do_open();
		do_close();
	}
}

void
PWM::do_open(void)
{
	int duty = 1500000;

	enable(1);
	//polarity("normal");
	OwTask::sleep(500);

	for (int i = 0; i < 21; i++)
	{
		duty_cycle(duty + (i * 50000));
		OwTask::sleep(200);
	}
	enable(0);
}

void
PWM::do_close(void)
{
	int duty = 2500000;

	enable(1);
	//polarity("normal");
	OwTask::sleep(500);

	for (int i = 0; i < 21; i++)
	{
		duty_cycle(duty - (i * 50000));
		OwTask::sleep(200);
	}
	enable(0);
}

void
PWM::period(int val)
{
    int ret;

	fp = fopen("/sys/class/pwm/pwmchip2/pwm0/period", "w");
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
PWM::polarity(char *pol)
{
    int ret;

	fp = fopen("/sys/class/pwm/pwmchip2/pwm0/polarity", "w");
    if( !fp )
    {
        perror("error open polarity file");
    }
    ret = fprintf(fp, "%s", pol);
    if (ret < 0)
    {
        perror("set_polarity");
    }
	fclose(fp);
}

void
PWM::duty_cycle(int val)
{
    int ret;

	fp = fopen("/sys/class/pwm/pwmchip2/pwm0/duty_cycle", "w");
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

	fp = fopen("/sys/class/pwm/pwmchip2/pwm0/enable", "w");
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

int
PWM::semaphoreTake(int timeout)
{
	int error = 0;

	m_hSem->take( timeout );

	return (error);
}

int
PWM::semaphoreGive()
{
	int error = 0;

	error = m_hSem->give();

	return (error);
}

