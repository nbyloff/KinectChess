#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"
#include <GL/GL.H>
#include <gl/GLU.h>
#include <cstring>
#define DepthRatio 0.36397023426620236135104788277683f

class Camera
{
public:
    Camera(Vector position, Vector rotation, Vector pivot)
    : Position(position), Rotation(rotation), Pivot(pivot)
    {}
    Camera()
    : Position(0,0,200), Rotation(0,45)
    {}

    // The world is being moved around the camera, hence the minus
    void SetCamera()
    {
        ClearCamera();
        glTranslatef(-Position.x,-Position.y,-Position.z); // Move World from the camera
        glRotatef(Rotation.z, 0, 0, 1); // Roll
        glRotatef(Rotation.y, 1, 0, 0); // Horizontal
        glRotatef(Rotation.x, 0, 1, 0); // Vertical
        glTranslatef(-Pivot.x,-Pivot.y,-Pivot.z);   // Move to pivot
        glGetFloatv(GL_MODELVIEW_MATRIX, View);     // Snap shot of view matrix
    }

    // Reset the Model View Matrix
    void ClearCamera()
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    // Set the projection matrix with a Perspective view
    void Perspective(int x, int y, int w, int h)
    {
        glViewport(x, y, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.f, float(w)/float(h), 1.f, 1000.f);
    }

    // Set Projection Matrix with a Orthographic View
    void Orthographic(int x, int y, int w, int h)
    {
        glViewport(x, y, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(x, w, h, y);
    }
    // Pan the camera - relative
    void Pan(int x, int y)
    {
        Position.x -= float(x)*0.004*(Position.z*DepthRatio);
        Position.y += float(y)*0.004*(Position.z*DepthRatio);
    }
    // Orbit the camera - relative
    void Orbit(int x, int y)
    {
        Rotation.x += float(x);
        Rotation.y += float(y);
    }
    // Roll the camera - relative
    void Roll(int z)
    {
        Rotation.z += z;
    }
    // Zoom/Dolly the camera - relative
    void Zoom(int z)
    {
        Position.z += z;
    }
    // Zoom/Dolly the camera - percentage
    void Zoom(float percent)
    {
        Position.z *= percent;
    }

    void SetPosition(Vector position)
    { Position = position; }
    void SetRotation(Vector rotation)
    { Rotation = rotation; }
    void SetPivot(Vector pivot)
    { Pivot = pivot; }

    void GetCameraMatrix(float *f)
    {
        memcpy(f, View, 16);
    }

    Vector i()
    { return Vector(View[0],View[4],View[8]); }
    Vector j()
    { return Vector(View[1],View[5],View[9]); }
    Vector k()
    { return Vector(View[2],View[6],View[10]); }

protected:
    Vector Position, Rotation, Pivot;
    float View[16];

};

#endif
