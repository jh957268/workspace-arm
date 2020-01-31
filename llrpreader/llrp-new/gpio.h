#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdio.h>

#define NUM_GPIO_PIN        2

class GPIO
{
    public:
		        GPIO(int pin_no);
				~GPIO();

        static  GPIO*	getInstance(int inst, int pin_no = 153);
        void    set_gpio_pin(int val);


    private:
        static  GPIO* spInstance[NUM_GPIO_PIN]; ///< Points to the instance
        FILE    *fp;
        int     gpio_no;
};


#endif // _GPIO_H_
