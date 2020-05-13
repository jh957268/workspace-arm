#ifndef _PWM_H_
#define _PWM_H_

#include <stdio.h>

class PWM
{
    public:
		        PWM();
				~PWM();

        static  PWM*	getInstance();
        void    enable(int val);
		void 	period(int ns);
		void	duty_cycle(int ns);

    private:
        static  PWM* spInstance; ///< Points to the instance
        FILE    *fp;
};


#endif // _GPIO_H_
