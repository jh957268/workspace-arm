#ifndef _PWM_H_
#define _PWM_H_

#include <stdio.h>
#include "OwTask.h"
#include "OwSemaphore.h"

class PWM:
	public OwTask
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
		void 	do_close();
	    static	int		semaphoreGive();
    private:
        static  PWM* spInstance; ///< Points to the instance
        FILE    *fp;

	    static	int		semaphoreTake(int timeout);

	    static OwSemaphore 		*m_hSem;
 };


#endif // _GPIO_H_
