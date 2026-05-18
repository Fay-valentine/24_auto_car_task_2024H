#ifndef _MODE4_H_
#define _MODE4_H_

#define POINT_A      1
#define POINT_C      2   //先到达C点，再到B点
#define POINT_B      3
#define POINT_D      4

void Mode4_Init(void);
void Mode4_Loop(void);
void Mode4_Tick(void);
void Mode4_Exit(void);

#endif
