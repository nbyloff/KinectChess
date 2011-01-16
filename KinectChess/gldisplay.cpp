//////SOURCE/////////////Gldisplay.cpp/////////////////////
///     Module d'affichage OpenGL                       ///
///     Version 28/12/2002      Par Grégory Smialek     ///
///////////www.texel.fr.fm//////texel@fr.fm////////////////

#include"gldisplay.h"
#include"camera.h"
#include"mytime.h"

float rtri=0.0;



void An8Display2(An8Object *TheAn8Obj)
{
	int i_mesh;
	int i=0;
	int in=0;

	for(i_mesh=0;i_mesh<An8Obj.NbrMesh;++i_mesh)
	{
		
		if(!TheAn8Obj->Material[TheAn8Obj->Mesh[i_mesh].MatNumber].IsTexture)
		{
			glBegin(GL_TRIANGLES);	
			glColor3ub(An8Obj.Material[TheAn8Obj->Mesh[i_mesh].MatNumber].RGB[0],
					   An8Obj.Material[TheAn8Obj->Mesh[i_mesh].MatNumber].RGB[1],
					   An8Obj.Material[TheAn8Obj->Mesh[i_mesh].MatNumber].RGB[2]);	

		
			for(i=0,in=0;i<3*TheAn8Obj->Mesh[i_mesh].NbrFaces;i=i+3,in=in+9)
			{
				
				
				glNormal3f(TheAn8Obj->Mesh[i_mesh].Normals[in],TheAn8Obj->Mesh[i_mesh].Normals[in+1],TheAn8Obj->Mesh[i_mesh].Normals[in+2]);
		
				glVertex3f(TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i]], 
							TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i]+1], 
							TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i]+2]
				);
	
				glNormal3f(TheAn8Obj->Mesh[i_mesh].Normals[in+3],TheAn8Obj->Mesh[i_mesh].Normals[in+4],TheAn8Obj->Mesh[i_mesh].Normals[in+5]);
				glVertex3f(TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+1]], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+1]+1], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+1]+2]
				);
			
				glNormal3f(TheAn8Obj->Mesh[i_mesh].Normals[in+6],TheAn8Obj->Mesh[i_mesh].Normals[in+7],TheAn8Obj->Mesh[i_mesh].Normals[in+8]);
				glVertex3f(TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+2]], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+2]+1], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+2]+2]
				);
			}
			glEnd();
			
		}
		else 
		{
			
			glColor3ub(255,255,255);		
			glBindTexture(GL_TEXTURE_2D,TheAn8Obj->Material[TheAn8Obj->Mesh[i_mesh].MatNumber].MatID);
			
		
			glBegin(GL_TRIANGLES);	
			
			
			for(i=0,in=0;i<3*TheAn8Obj->Mesh[i_mesh].NbrFaces;i=i+3,in=in+9)
			{
				
				glNormal3f(TheAn8Obj->Mesh[i_mesh].Normals[in],TheAn8Obj->Mesh[i_mesh].Normals[in+1],TheAn8Obj->Mesh[i_mesh].Normals[in+2]);
	
				glTexCoord2f(TheAn8Obj->Mesh[i_mesh].TexCoords[2*TheAn8Obj->Mesh[i_mesh].uvIndices[i]], 
							 TheAn8Obj->Mesh[i_mesh].TexCoords[2*TheAn8Obj->Mesh[i_mesh].uvIndices[i]+1]);

				glVertex3f(TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i]], 
							TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i]+1], 
							TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i]+2]
				);
				
				glNormal3f(TheAn8Obj->Mesh[i_mesh].Normals[in+3],TheAn8Obj->Mesh[i_mesh].Normals[in+4],TheAn8Obj->Mesh[i_mesh].Normals[in+5]);
				glTexCoord2f(TheAn8Obj->Mesh[i_mesh].TexCoords[2*TheAn8Obj->Mesh[i_mesh].uvIndices[i+1]], 
							 TheAn8Obj->Mesh[i_mesh].TexCoords[2*TheAn8Obj->Mesh[i_mesh].uvIndices[i+1]+1]);
				glVertex3f(TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+1]], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+1]+1], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+1]+2]
				);
			
				glNormal3f(TheAn8Obj->Mesh[i_mesh].Normals[in+6],TheAn8Obj->Mesh[i_mesh].Normals[in+7],TheAn8Obj->Mesh[i_mesh].Normals[in+8]);
				glTexCoord2f(TheAn8Obj->Mesh[i_mesh].TexCoords[2*TheAn8Obj->Mesh[i_mesh].uvIndices[i+2]], 
							 TheAn8Obj->Mesh[i_mesh].TexCoords[2*TheAn8Obj->Mesh[i_mesh].uvIndices[i+2]+1]);
				glVertex3f(TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+2]], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+2]+1], 
						   TheAn8Obj->Mesh[i_mesh].points[3*TheAn8Obj->Mesh[i_mesh].faceIndices[i+2]+2]
				);
			}
			glEnd();	
		}

	
		
	}



}



int DrawGLScene(GLvoid)									
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	glLoadIdentity();
	
	 gluLookAt(mCamera_PosX, mCamera_PosY, mCamera_PosZ,	
			  mCamera_TargX,mCamera_TargY,mCamera_TargZ,	
			  0.0, 1.0,0.0);	
	

	glColor3f(1.0f,1.0f,1.0f);
	glFrontFace(GL_CCW);
	DrawSkyBox(0.0f, 0.0f,0.0f, 3000.0f,3000.0f,3000.0f);
	glFrontFace(GL_CW);
	


	if(IsLight)
	{
		glEnable(GL_LIGHTING);								
		glEnable(GL_LIGHT1);								
	}
	An8Display2(&An8Obj);
	if(IsLight)
	{
		glDisable(GL_LIGHTING);							
		glDisable(GL_LIGHT1);							
	}

	TempsPresent2();




//////////////////////////////////////
	 glFrontFace(GL_CCW);
		//glColor3f(0.0f,0.0f,1.0f);							
		glPrint2D(40,30,1,"ABYSSE");						
		glPrint2DJV(400,30,1,"TIME");	
		
		glPrint2DJO(470,30,1,"%d%d:%d%d:%d%d",
			mTime_Minute_10,mTime_Minute_1,
			mTime_Second_10,mTime_Second_1,
			mTime_MilliSecond_10,mTime_MilliSecond_1
		);	


		glPrint2D(495,425,1,"FPS= %d",DonneFps());	
		glFrontFace(GL_CW);
		glEnable(GL_TEXTURE_2D);
//////////////////	





	
	return TRUE;	
}


GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		
{



	if (height==0)										
	{
		height=1;										
	}

	glViewport(0,0,width,height);						

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();									

	
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,3000.0f);

	glMatrixMode(GL_MODELVIEW);						
	glLoadIdentity();									
}
