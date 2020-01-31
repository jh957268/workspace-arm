#include "gpio.h"
#include "stdlib.h"

using namespace std;

GPIO* GPIO::spInstance[NUM_GPIO_PIN] = {0, 0};

GPIO::GPIO(int pin_no)
{
	char device_name[40];
    gpio_no = pin_no;

    fp = fopen("/sys/class/gpio/export", "w");
    if( !fp )
    {
        perror("error open export file");
    }
    fprintf(fp, "%d", pin_no);
    fclose(fp);

    sprintf(device_name,"/sys/class/gpio/gpio%d/direction", pin_no);
    fp = fopen(device_name, "w");

    if( !fp )
    {
        perror("error open export file");
    }
    fprintf(fp, "%s", "out");
    fclose(fp);

    sprintf(device_name,"/sys/class/gpio/gpio%d/value", pin_no);
    fp = fopen(device_name, "w");

    if( !fp )
    {
        perror("error open export file");
    }
    fprintf(fp, "%d", 1);
    fflush(fp);				// or the data will not go out until the fp is closed
    
}

GPIO::~GPIO()
{}

void
GPIO::set_gpio_pin(int val)
{
    int ret;

    ret = fprintf(fp, "%d", val);
    if (ret < 0)
    {
        perror("set_gpio_val:");
    }
    fflush(fp);				// or the data will not go out until the fp is closed
}

GPIO
*GPIO::getInstance
(
	int inst, int pin_no
)
{
    if (inst >= NUM_GPIO_PIN)
    {
        return NULL;
    }
	if ( 0 == spInstance[inst] )
	{
		spInstance[inst] = new GPIO(pin_no);
	}

	return( spInstance[inst] );

} // LLRP_

