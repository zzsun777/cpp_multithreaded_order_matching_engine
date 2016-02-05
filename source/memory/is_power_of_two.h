#ifndef __IS_POWER_OF_TWO__
#define __IS_POWER_OF_TWO__

#define is_power_of_two(n) ( n && ((n & (n - 1)) == 0) ? true : false )

#endif