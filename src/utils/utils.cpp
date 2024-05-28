#include "./utils.h"
#include <time.h>
#include <string>

double getTemperature()
{
    const long max = 1000000L;
    double low = -100;
    double up = 100;
    srandom(time(NULL));

    return low + (up - low) * (random() % max) / max;
};