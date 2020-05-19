#include "pwm.h"
#include "stdlib.h"
#include <time.h>

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
	duty = MIN_DUTY;
	step = 25000;
	pmDoCloseTimer = 0;
	m_hMutex = 	new OwMutex();
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
	enable(1);
	do_open();
	// do_close();

	pmDoCloseTimer = new OwTimer(this,"PWMTimer");

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

		pmDoCloseTimer->cancel();
		do_open();
		pmDoCloseTimer->start( PWM_DOCLOSE_TIMEOUT );

		//sleep();
		//OwTask::sleep(2000);
		//pmDoCloseTimer->cancel();

		//pmDoCloseTimer->start( PWM_DOCLOSE_TIMEOUT );
		// do_close();
	}
}

void
PWM::do_open(void)
{
	// enable(1);
	//polarity("normal");
	PWMTakeMutex();
	OwTask::sleep(500);

	//printf("current duty cycle = %d\n", duty);
	// duty_cycle(duty);
	time_t T= time(NULL);
	struct  tm tm = *localtime(&T);
	printf("%02d:%02d:%02d do_open duty = %d!\n", tm.tm_hour, tm.tm_min, tm.tm_sec, duty);	
	while (duty <= MAX_DUTY)
	{
		//printf("open current duty cycle = %d\n", duty);
		duty += step;
		duty_cycle(duty);
		OwTask::sleep(100);
	}
	T= time(NULL);
	tm = *localtime(&T);
	printf("%02d:%02d:%02d end do_open duty = %d!\n", tm.tm_hour, tm.tm_min, tm.tm_sec, duty);		
	//enable(0);
	PWMGiveMutex();
}

void
PWM::do_close(void)
{
	//enable(1);
	//polarity("normal");
	PWMTakeMutex();
	OwTask::sleep(500);

	time_t T= time(NULL);
	struct  tm tm = *localtime(&T);
	printf("%02d:%02d:%02d do_close duty = %d!\n", tm.tm_hour, tm.tm_min, tm.tm_sec, duty);	
	while (duty >= MIN_DUTY)
	{
		// printf("close current duty cycle = %d\n", duty);
		duty -= step;
		duty_cycle(duty);
		OwTask::sleep(100);
	}
	T= time(NULL);
	tm = *localtime(&T);
	printf("%02d:%02d:%02d end do_close duty = %d!\n", tm.tm_hour, tm.tm_min, tm.tm_sec, duty);	
	PWMGiveMutex();
	//enable(0);
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

//=============================================================================
//handleTimeout

void
PWM::handleTimeout
(
	OwTimer*  timer
)
{

	do_close();

#if 0
	switch ( meInputVoltageState )
	{
		case  eOVER_VOLT:
		SPrintf( "eOVER_VOLT - alarm raised"NL );
		meInputVoltageState = eOVER_VOLT_ALARM;
		// raise Alarm
		setAlarmWithId( INPUT_VOLTAGE_FAILURE, RAISE );
		break;

		case  eUNDER_VOLT:
		SPrintf( "eUNDER_VOLT_ALARM - alarm raised"NL );
		meInputVoltageState = eUNDER_VOLT_ALARM;
		// raise Alarm
		setAlarmWithId( INPUT_VOLTAGE_FAILURE, RAISE );
		break;

		default:
		SPrintf( "Illegal state when timer elapsed"NL );
		break;
	}

	FlightRecorder::getInstance()->logError( "Input Voltage Failure - %7.3f"NL,
												mfLastInputVoltage );
#endif

}

int  PWM::PWMTakeMutex(void)
{
	int error = 0;

	m_hMutex->take( PI_FOREVER );

	return (error);
}

int  PWM::PWMGiveMutex(void)
{
	int error = 0;

	m_hMutex->give();

	return (error);
}
