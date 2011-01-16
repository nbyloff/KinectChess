//#include <stdio.h>
#include "mytime.h"


clock_t temps_precedent;

float Temps_Depuis_PreFrame;
float Temps_Demo_Initial;
float Temps_Demo_Present;

int mTime_MilliSecond_1;
int mTime_Second_1;
int mTime_Minute_1;

int mTime_MilliSecond_10;
int mTime_Second_10;
int mTime_Minute_10;

int fps_count = 0;	
float tps = 0.0;
int Myfps = 0;			// contient le nbr de fps


int DonneFps(void)
{
	tps += Temps_Depuis_PreFrame;
	if(tps < 1.0)
		fps_count += 1;
	else
	{
		Myfps = fps_count;
		fps_count = 0;
		tps = 0.0;
	}
	return Myfps;
}



float TempsPresent(void)
{
	clock_t nouveau_temps;

	nouveau_temps = clock();

	///
	Temps_Depuis_PreFrame= (float) nouveau_temps- temps_precedent;
	temps_precedent = nouveau_temps;
	Temps_Depuis_PreFrame=(Temps_Depuis_PreFrame / (float)CLOCKS_PER_SEC);
	///

	Temps_Demo_Present=(nouveau_temps / (float)CLOCKS_PER_SEC);
//	Temps_Demo_Present=Temps_Demo_Present;

	return Temps_Demo_Present;
}


float TempsPresent2(void)
{
	clock_t nouveau_temps;
	int mTime_MilliSecond;
	int mTime_Second;
	int mTime_Minute;

	nouveau_temps = clock();

	///
	Temps_Depuis_PreFrame= (float) nouveau_temps- temps_precedent;
	temps_precedent = nouveau_temps;
	Temps_Depuis_PreFrame=(Temps_Depuis_PreFrame / (float)CLOCKS_PER_SEC);
	///

	Temps_Demo_Present=(nouveau_temps / (float)CLOCKS_PER_SEC);
//	Temps_Demo_Present=Temps_Demo_Present;

	mTime_MilliSecond=(int)100*(Temps_Demo_Present-((int)Temps_Demo_Present));
	mTime_MilliSecond_10=(int)mTime_MilliSecond/10; 
	mTime_MilliSecond_1 =mTime_MilliSecond-(mTime_MilliSecond_10*10);

	mTime_Second= (int)Temps_Demo_Present;	

	mTime_Minute= (int)Temps_Demo_Present/60;
	mTime_Minute_10=mTime_Minute/10;
	mTime_Minute_1=mTime_Minute-(mTime_Minute_10*10);

	mTime_Second= (int)mTime_Second-(mTime_Minute*60);
	mTime_Second_10=(int)mTime_Second/10;
	mTime_Second_1=(int)mTime_Second-(mTime_Second_10*10);

	return Temps_Demo_Present;
}


