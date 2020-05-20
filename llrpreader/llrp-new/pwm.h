#ifndef _PWM_H_
#define _PWM_H_

#include <stdio.h>
//#include "OwTimer.h"
#include "OwTask.h"
#include "OwTimer.h"
#include "OwSemaphore.h"
//#include "OwTimer.h"
#include "OwMutex.h"



#define MAX_DUTY	2500000
#define MIN_DUTY	1500000

#define  PWM_DOCLOSE_TIMEOUT  5000

class PWM:
	public OwTask,
	public OwTimer::Handler
{
    public:
		        PWM();
				~PWM();

		void	main(OwTask*);
        static  PWM*	getInstance();
        void    enable(int val);
		void 	period(int ns);
		void	duty_cycle(int ns);
		void  	polarity(char *pol);

		void 	do_open();
		void 	do_close(OwTimer*  timer);
	    static	int		semaphoreGive();
    private:
        static  PWM* spInstance; ///< Points to the instance
        FILE    *fp;
		int 	duty;
		int 	step;
		OwTimer		*pmDoCloseTimer;			// Timeout to perform closing the gate

	    static	int		semaphoreTake(int timeout);

	    static OwSemaphore 		*m_hSem;
		OwMutex					*m_hMutex;
		void	handleTimeout( OwTimer*  timer );
		int		PWMTakeMutex(void);
		int 	PWMGiveMutex(void);
 };


#endif // _GPIO_H_
