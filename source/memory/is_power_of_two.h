#ifndef _IS_POWER_OF_TWO_H_
#define _IS_POWER_OF_TWO_H_

#define is_power_of_two(n) ( n && ((n & (n - 1)) == 0) ? true : false )

#endif