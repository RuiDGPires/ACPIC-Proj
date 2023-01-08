#ifndef __TL_CORE__
#define __TL_CORE__
//
// ------
//  UTIL
// ------

#define N2(n) n, n+1
#define N3(n) N2(n), n+2
#define N4(n) N3(n), n+3
#define N5(n) N4(n), n+4
#define N6(n) N5(n), n+5
#define N7(n) N6(n), n+6
#define N8(n) N7(n), n+7
#define N9(n) N8(n), n+8
#define N10(n) N9(n), n+9
#define N11(n) N10(n), n+10

void tl_setup(int, int, int, int, int, int);
void tl_loop();

#endif
