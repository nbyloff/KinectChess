//////SOURCE/////////////Camera.cpp////////////////////////
///     Module de gestion des déplacements de camera    ///
///     Version 28/12/2002      Par Grégory Smialek     ///
///////////www.texel.fr.fm//////texel@fr.fm////////////////

#include"camera.h"

float cosX,sinX,cosY,sinY;
float step;
float Deg2Radian=0.01745329;

float mCamera_PosX=0.0;     // Position de la camera suivant X
float mCamera_PosY=15.0;    // Position de la camera suivant Y
float mCamera_PosZ=0.0;     // Position de la camera suivant Z

float mCamera_AngleX=0.0;
float mCamera_AngleY=0.0;

float mCamera_TargX=0.0;        // Cible de la camera suivant X
float mCamera_TargY=15.0;       // Cible de la camera suivant Y
float mCamera_TargZ= (-1.0);    // Cible de la camera suivant Z

float mCamera_UpX=0.0;      // Vecteur de roulis suivant X
float mCamera_UpY=1.0;      // Vecteur de roulis suivant Y
float mCamera_UpZ=0.0;      // Vecteur de roulis suivant Z

float mCamera_StrafeX=1.0;    // Cible de la camera suivant X
float mCamera_StrafeY=0.0;    // Cible de la camera suivant Y
float mCamera_StrafeZ=0.0;    // Cible de la camera suivant Z

int mCamera_Jump=0;  
float mCamera_YbeforeJump=0.0;  

///////////////////////////////////////////////
///     Fixe une position à la camera       ///
///////////////////////////////////////////////
void CameraSetPosition( float newX,   float newY,   float newZ,
                        float newTargX,  float newTargY,  float newTargZ,
                        float newUpX, float newUpY, float newUpZ)
{
   mCamera_PosX=newX;     // Position de la camera suivant X
   mCamera_PosY=newY;     // Position de la camera suivant Y
   mCamera_PosY=newZ;     // Position de la camera suivant Z

   mCamera_TargX=newTargX;    // Cible de la camera suivant X
   mCamera_TargY=newTargY;    // Cible de la camera suivant Y
   mCamera_TargZ=newTargZ;    // Cible de la camera suivant Z

   mCamera_UpX=newUpX;      // Vecteur de roulis suivant X
   mCamera_UpY=newUpY;      // Vecteur de roulis suivant Y
   mCamera_UpZ=newUpZ;      // Vecteur de roulis suivant Z 
}

///////////////////////////////////////////////
///    Avance  la camera                   ///
///////////////////////////////////////////////
void CameraMoveFront(float speed)
{
   step = (mCamera_TargX - mCamera_PosX)*speed;
             
   mCamera_PosX += step;
   mCamera_TargX += step;
         
   step = (mCamera_TargZ - mCamera_PosZ)*speed;
   mCamera_PosZ += step;
   mCamera_TargZ+= step;

}

///////////////////////////////////////////////
///    Recule la camera                     ///
///////////////////////////////////////////////
void CameraMoveBack(float speed)
{
   step = (mCamera_TargX - mCamera_PosX)*speed;
   mCamera_PosX -=  step;
   mCamera_TargX -=  step;
         
   step = (mCamera_TargZ - mCamera_PosZ)*speed;
   mCamera_PosZ -=   step;
   mCamera_TargZ -=  step;

}

///////////////////////////////////////////////
///    Strafe la camera à droite            ///
///////////////////////////////////////////////
void CameraStrafeRight(float speed)
{
     mCamera_PosX +=  (mCamera_StrafeX*speed);
     mCamera_PosY +=  (mCamera_StrafeY*speed);
     mCamera_PosZ +=  (mCamera_StrafeZ*speed);
		    
     mCamera_TargX +=  (mCamera_StrafeX*speed);
     mCamera_TargY +=  (mCamera_StrafeY*speed);
     mCamera_TargZ +=  (mCamera_StrafeZ*speed);

}

///////////////////////////////////////////////
///    Strafe la camera à gauche            ///
///////////////////////////////////////////////
void CameraStrafeLeft(float speed)
{
     mCamera_PosX -=  (mCamera_StrafeX*speed);
     mCamera_PosY -=  (mCamera_StrafeY*speed);
     mCamera_PosZ -=  (mCamera_StrafeZ*speed);
		    
     mCamera_TargX -=  (mCamera_StrafeX*speed);
     mCamera_TargY -=  (mCamera_StrafeY*speed);
     mCamera_TargZ -=  (mCamera_StrafeZ*speed);

}

///////////////////////////////////////////////
///    Rotation de la camera en haut        ///
///////////////////////////////////////////////
void CameraRotateUp(float speed)
{
    mCamera_AngleX = mCamera_AngleX +speed;
    if(mCamera_AngleX>360.0) mCamera_AngleX -= 360.0; 

    cosX = (float) cos(mCamera_AngleX*Deg2Radian);
    sinX = (float) sin(mCamera_AngleX*Deg2Radian);
    cosY = (float) cos(mCamera_AngleY*Deg2Radian);
    sinY = (float) sin(mCamera_AngleY*Deg2Radian);

	 		    
    mCamera_TargX = (-cosX*sinY) + mCamera_PosX;
    mCamera_TargY = sinX + mCamera_PosY;
    mCamera_TargZ = (-cosX*cosY) + mCamera_PosZ;
		     
    mCamera_StrafeX = cosY;
    mCamera_StrafeY = 0.0;
    mCamera_StrafeZ = -sinY;	
    
}
    
///////////////////////////////////////////////
///    Rotation de la camera en bas         ///
///////////////////////////////////////////////
void CameraRotateDown(float speed)
{
   mCamera_AngleX = mCamera_AngleX -speed;
   if(mCamera_AngleX<360.0) mCamera_AngleX += 360.0; 
			
   cosX = (float) cos(mCamera_AngleX*Deg2Radian);
   sinX = (float) sin(mCamera_AngleX*Deg2Radian);
   cosY = (float) cos(mCamera_AngleY*Deg2Radian);
   sinY = (float) sin(mCamera_AngleY*Deg2Radian);
		    		    
   mCamera_TargX = (-cosX*sinY)+ mCamera_PosX;
   mCamera_TargY = sinX+ mCamera_PosY;
   mCamera_TargZ = (-cosX*cosY) + mCamera_PosZ;
		     
   mCamera_StrafeX = cosY;
   mCamera_StrafeY = 0.0;
   mCamera_StrafeZ = -sinY;
    
}
    
///////////////////////////////////////////////
///    Rotation de la camera à droite       ///
///////////////////////////////////////////////
void CameraRotateRight(float speed)
{
    mCamera_AngleY = mCamera_AngleY -speed;
    if(mCamera_AngleY<360.0) mCamera_AngleY += 360.0; 
			
    cosX = (float) cos(mCamera_AngleX*Deg2Radian);
    sinX = (float) sin(mCamera_AngleX*Deg2Radian);
    cosY = (float) cos(mCamera_AngleY*Deg2Radian);
    sinY = (float) sin(mCamera_AngleY*Deg2Radian);
		    
		    
    mCamera_TargX = (-cosX*sinY)+ mCamera_PosX;
    mCamera_TargY = sinX + mCamera_PosY;
    mCamera_TargZ = (-cosX*cosY) + mCamera_PosZ;
		     
    mCamera_StrafeX = cosY;
    mCamera_StrafeY = 0.0;
    mCamera_StrafeZ = -sinY;
    
}
    
///////////////////////////////////////////////
///    Rotation de la camera à gauche       ///
///////////////////////////////////////////////
void CameraRotateLeft(float speed)
{
    mCamera_AngleY = mCamera_AngleY +speed;
    if(mCamera_AngleY>360.0) mCamera_AngleY -= 360.0; 
		  
    cosX = (float) cos(mCamera_AngleX*Deg2Radian);
    sinX = (float) sin(mCamera_AngleX*Deg2Radian);
    cosY = (float) cos(mCamera_AngleY*Deg2Radian);
    sinY = (float) sin(mCamera_AngleY*Deg2Radian);
		    
		    
    mCamera_TargX = (-cosX*sinY)+ mCamera_PosX;
    mCamera_TargY = sinX+ mCamera_PosY;
    mCamera_TargZ = (-cosX*cosY) + mCamera_PosZ;
		     
    mCamera_StrafeX = cosY;
    mCamera_StrafeY = 0.0;
    mCamera_StrafeZ = -sinY;
    
}

///////////////////////////////////////////////
///    Saut de la camera                    ///
///////////////////////////////////////////////
void CameraJump(void)
{
   if (mCamera_Jump==0) 
   {  
      mCamera_Jump=1;
      mCamera_YbeforeJump=mCamera_PosY;
   }
       
}

///////////////////////////////////////////////
///    Saut de la camera                    ///
///////////////////////////////////////////////
void CameraUpDateJump(void)
{
   static float JumpHeight=0.0;
   static float gravite=0.1;
   float temp;
  
   
   if (mCamera_Jump==1 && JumpHeight<3.0)   
   {
      gravite+=0.1;
      JumpHeight+=0.2/gravite;
      mCamera_PosY+=0.2/gravite;
      mCamera_TargY+=0.2/gravite;
   }
   else if( mCamera_Jump==1 && JumpHeight>=3.0)
   {
      mCamera_Jump=2;     
   }
   else if( mCamera_Jump==2 &&  JumpHeight>0.0)
   {
      gravite-=0.1;
      JumpHeight-=0.2/gravite;
      if(  mCamera_PosY-(0.2/gravite)>=mCamera_YbeforeJump) 
      {     
            mCamera_PosY-=0.2/gravite;
            mCamera_TargY-=0.2/gravite;
      }
   }
   else if(mCamera_Jump==2 && JumpHeight<=0.0)
   {
      gravite=0.1;
      
      JumpHeight=0.0;  
      temp=mCamera_YbeforeJump - mCamera_PosY;
      if(temp<0.0) temp=(-temp);
      mCamera_TargY= mCamera_TargY - temp;    
      mCamera_PosY=mCamera_YbeforeJump;  
      mCamera_Jump=0; 
      
   }       
}






