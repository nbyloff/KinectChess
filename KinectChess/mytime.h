#ifdef __cplusplus
extern "C" {
#endif



//#ifndef _MYTIME_H
//#define _MYTIME_H


#include <windows.h>
#include <time.h>



extern clock_t temps_precedent;
extern float Temps_Depuis_PreFrame;
extern float Temps_Demo_Initial;
extern float Temps_Demo_Present;

extern int mTime_MilliSecond_1;
extern int mTime_Second_1;
extern int mTime_Minute_1;

extern int mTime_MilliSecond_10;
extern int mTime_Second_10;
extern int mTime_Minute_10;

extern int fps_count;	
extern float tps;
extern int Myfps;	// contient le nbr de fps


int DonneFps(void);
float TempsPresent(void);
float TempsPresent2(void);


//#endif	/* _MYTIME_H */

#ifdef __cplusplus
};
#endif
