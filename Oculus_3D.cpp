/*
OneLoneCoder.com - 3D Graphics Part #3 - Cameras & Clipping
"Tredimensjonal Grafikk" - @Javidx9

License
~~~~~~~
One Lone Coder Console Game Engine  Copyright (C) 2018  Javidx9
This program comes with ABSOLUTELY NO WARRANTY.
This is free software, and you are welcome to redistribute it
under certain conditions; See license for details.
Original works located at:
https://www.github.com/onelonecoder
https://www.onelonecoder.com
https://www.youtube.com/javidx9
GNU GPLv3
https://github.com/OneLoneCoder/videos/blob/master/LICENSE

From Javidx9 :)
~~~~~~~~~~~~~~~
Hello! Ultimately I don't care what you use this for. It's intended to be
educational, and perhaps to the oddly minded - a little bit of fun.
Please hack this, change it and use it in any way you see fit. You acknowledge
that I am not responsible for anything bad that happens as a result of
your actions. However this code is protected by GNU GPLv3, see the license in the
github repo. This means you must attribute me if you use it. You can view this
license here: https://github.com/OneLoneCoder/videos/blob/master/LICENSE
Cheers!

Background
~~~~~~~~~~
3D Graphics is an interesting, visually pleasing suite of algorithms. This is the
first video in a series that will demonstrate the fundamentals required to
build your own software based 3D graphics systems.

Video
~~~~~
https://youtu.be/ih20l3pJoeU
https://youtu.be/XgMWc6LumG4
https://youtu.be/HXSuNxpCzdM

Author
~~~~~~
Twitter: @javidx9
Blog: http://www.onelonecoder.com
Discord: https://discord.gg/WhwHUMV


Last Updated: 14/08/2018
*/


#include "olcConsoleGameEngine.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <strstream>
#include <algorithm>
#include <string>
#include <sstream> 
#include "nurbs.h"
using namespace std;


void save(controlPoints s)
{
	ofstream myfile;
	myfile.open("example.txt");
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{

			myfile << (int)(s.points[i][j].x * 10000);
			myfile << ' ';
			myfile << (int)(s.points[i][j].y * 10000);
			myfile << ' ';
			myfile << (int)(s.points[i][j].z * 10000);
			myfile << ' ';
		}
	}
	myfile.close();
}

void load(controlPoints &l)
{
	stringstream geek("2");
	int temi;
	std::string linia;
	ifstream myfile("example.txt" , std::ios::in);
	int value;
	int i = 0;

	while (!myfile.eof())
	{
		myfile >> value;
		l.points[i / 4][i % 4].x = float(value/10000.0);
		myfile >> value;
		l.points[i / 4][i % 4].y = float(value / 10000.0);
   		myfile >> value;
		l.points[i / 4][i % 4].z = float(value / 10000.0);
		//l->points[i / 4][i % 4].w = 1.0;
		i++;

		
	}
	i++;

	l.choosePoint(0);
	l.setResolution(10);
}

struct mat4x4
{
	float m[4][4] = { 0 };
};

class olcEngine3D : public olcConsoleGameEngine
{
public:
	olcEngine3D()
	{
		m_sAppName = L"Oculus_3D";
	}


private:
	controlPoints temCp;
	controlPoints cp;
	controlPoints cp2;
	bool l, c, b1, b2, s, change, ud;
	float timeChangeBuffer;
	float zoom;

	mesh meshSurface;
	mat4x4 matProj;	// Matrix that converts from view space to screen space
	vec3d vCamera;	// Location of camera in world space
	vec3d vLookDir;	// Direction vector along the direction camera points
	float fYaw;		
	float xr = 0, yr = 0, zr = 0;	// Spins World transform

	vec3d Matrix_MultiplyVector(mat4x4& m, vec3d& i)
	{
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		return v;
	}

	mat4x4 Matrix_MakeIdentity()
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationX(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[1][2] = sinf(fAngleRad);
		matrix.m[2][1] = -sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationY(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][2] = sinf(fAngleRad);
		matrix.m[2][0] = -sinf(fAngleRad);
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationZ(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[0][1] = sinf(fAngleRad);
		matrix.m[1][0] = -sinf(fAngleRad);
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeTranslation(float x, float y, float z)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

	mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}

	mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
	{
		// Calculate new forward direction
		vec3d newForward = Vector_Sub(target, pos);
		newForward = Vector_Normalise(newForward);

		// Calculate new Up direction
		vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
		vec3d newUp = Vector_Sub(up, a);
		newUp = Vector_Normalise(newUp);

		// New Right direction is easy, its just cross product
		vec3d newRight = Vector_CrossProduct(newUp, newForward);

		// Construct Dimensioning and Translation Matrix	
		mat4x4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;

	}

	mat4x4 Matrix_QuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	vec3d Vector_Add(vec3d& v1, vec3d& v2)
	{
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}

	vec3d Vector_Sub(vec3d& v1, vec3d& v2)
	{
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}

	vec3d Vector_Mul(vec3d& v1, float k)
	{
		return { v1.x * k, v1.y * k, v1.z * k };
	}

	vec3d Vector_Div(vec3d& v1, float k)
	{
		return { v1.x / k, v1.y / k, v1.z / k };
	}

	float Vector_DotProduct(vec3d& v1, vec3d& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	float Vector_Length(vec3d& v)
	{
		return sqrtf(Vector_DotProduct(v, v));
	}

	vec3d Vector_Normalise(vec3d& v)
	{
		float l = Vector_Length(v);
		return { v.x / l, v.y / l, v.z / l };
	}

	vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2)
	{
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}






	// !!!
	CHAR_INFO GetColour(float lum) 
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}

public:
	bool OnUserCreate() override
	{

		matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		cp.setBasic();
		cp.calculateLines();
		cp.setResolution(20);
		cp2 = cp;
		change = 0;
		l = 1; c = 1; b1 = 1; b2 = 1; s = 1; ud = 1;
		zoom = 3.0;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		
		mat4x4 matRotZ, matRotX, matRotY;
		mat4x4 matTrans;


		if (GetKey(VK_F7).bHeld)
		{
			save(cp);
		}

		if (GetKey(VK_F8).bHeld)
		{
			//shared_ptr<controlPoints> cps = make_shared<controlPoints>(cp);
			
			load(cp);
		
		}
		

		matTrans = Matrix_MakeTranslation(0.0f, 0.0f, zoom);

		float movement = 1.0f * fElapsedTime;

		if (GetKey(VK_F12).bHeld)
			zoom -= movement;
		if (GetKey(VK_F11).bHeld)
			zoom += movement;

		if (GetKey(VK_UP).bHeld)
			xr += 1.0f * fElapsedTime;
		if (GetKey(VK_DOWN).bHeld)
			xr -= 1.0f * fElapsedTime;
		if (GetKey(VK_LEFT).bHeld)
			yr += 1.0f * fElapsedTime;
		if (GetKey(VK_RIGHT).bHeld)
			yr -= 1.0f * fElapsedTime;

		if (GetKey('V').bHeld)
			cp.choosePoint(0);
		if (GetKey('C').bHeld)
			cp.choosePoint(1);
		if (GetKey('X').bHeld)
			cp.choosePoint(2);
		if (GetKey('Z').bHeld)
			cp.choosePoint(3);
		if (GetKey('F').bHeld)
			cp.choosePoint(4);
		if (GetKey('D').bHeld)
			cp.choosePoint(5);
		if (GetKey('S').bHeld)
			cp.choosePoint(6);
		if (GetKey('A').bHeld)
			cp.choosePoint(7);
		if (GetKey('R').bHeld)
			cp.choosePoint(8);
		if (GetKey('E').bHeld)
			cp.choosePoint(9);
		if (GetKey('W').bHeld)
			cp.choosePoint(10);
		if (GetKey('Q').bHeld)
			cp.choosePoint(11);
		if (GetKey('4').bHeld)
			cp.choosePoint(12);
		if (GetKey('3').bHeld)
			cp.choosePoint(13);
		if (GetKey('2').bHeld)
			cp.choosePoint(14);
		if (GetKey('1').bHeld)
			cp.choosePoint(15);

		if (GetKey('I').bHeld)
			cp.moveChosen(0.0, 0.0, movement);
		if (GetKey('K').bHeld)
			cp.moveChosen(0.0, 0.0, -1 * movement);
		if (GetKey('J').bHeld)
			cp.moveChosen( movement, 0.0, 0.0 );
		if (GetKey('L').bHeld)
			cp.moveChosen(-1 * movement, 0.0, 0.0 );
		if (GetKey('U').bHeld)
			cp.moveChosen(  0.0, movement, 0.0 );
		if (GetKey('O').bHeld)
			cp.moveChosen(  0.0, -1 * movement, 0.0 );

		if (GetKey('M').bHeld)
			cp.addResolution(1);

		if (GetKey('N').bHeld)
			cp.addResolution(-1);

		if (GetKey(VK_F1).bHeld && change == 0)
		{
			change = 1;
			if (l == 0)
				l = 1;
			else
				l = 0;
		}
		else
		if (GetKey(VK_F2).bHeld && change == 0)
		{
			change = 1;
			if (c == 0)
				c = 1;
			else
				c = 0;
		}
		else
		if (GetKey(VK_F3).bHeld && change == 0)
		{
			change = 1;
			if (b1 == 0)
				b1 = 1;
			else
				b1 = 0;
		}
		else
		if (GetKey(VK_F4).bHeld && change == 0)
		{
			change = 1;
			if (b2 == 0)
				b2 = 1;
			else
				b2 = 0;
		}
		else
		if (GetKey(VK_F5).bHeld && change == 0)
		{
			change = 1;
			if (s == 0)
				s = 1;
			else
				s = 0;
		}
		else
		if (GetKey(VK_SPACE).bHeld && change == 0)
		{
			change = 1;
			if (ud == 0)
			{
				cp.upsideDown();
				ud = 1;
			}
			else
			{
				cp.upsideDown();
				ud = 0;
			}
		}
		else
		if (GetKey(VK_F9).bHeld)
		{
			change = 1;
			cp2 = cp;
		}
		else
		if (GetKey(VK_F10).bHeld)
		{
			change = 1;
			temCp = cp;
			cp = cp2;
			cp2 = temCp;
		}
		else
		{
			timeChangeBuffer += fElapsedTime;
			if (timeChangeBuffer > 0.4)
			{
				change = 0;
				timeChangeBuffer = 0;
			}
		}

		cp.draw( l, c, b1, b2);

		matRotY = Matrix_MakeRotationY(yr);
		matRotX = Matrix_MakeRotationX(xr);
		matRotZ = Matrix_MakeRotationZ(zr);
		mat4x4 matWorld;
		matWorld = Matrix_MakeIdentity();	// Form World Matrix
		matWorld = Matrix_MultiplyMatrix(matRotY, matRotX); // Transform by rotation
		matWorld = Matrix_MultiplyMatrix(matWorld, matRotZ);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation, moving from viewer

		// Create "Point At" Matrix for camera
		vec3d vUp = { 0,1,0 };
		vec3d vTarget = { 0,0,1 };
		vCamera = { 0,0,0 };
		mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		// Make view matrix from camera
		mat4x4 matView = Matrix_QuickInverse(matCamera);

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		if (s)
		{
			// Store triagles for rastering later
			vector<triangle> vecTrianglesToRaster;

			// Draw Triangles
			cp.generateMesh();
			meshSurface = cp.surface1;
			for (auto tri : meshSurface.tris)
			{
				triangle triProjected, triTransformed, triViewed;

				// World Matrix Transform
				triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
				triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
				triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

				// Calculate triangle Normal
				vec3d normal, line1, line2;

				// Get lines either side of triangle
				line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
				line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

				// Take cross product of lines to get normal to triangle surface
				normal = Vector_CrossProduct(line1, line2);
				/*normal.x *= -1;
				normal.y *= -1;
				normal.z *= -1;*/

				// Normalise a normal
				normal = Vector_Normalise(normal);

				// Get ray from triangle to camera
				vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

				// If ray is aligned with normal, then triangle is visible
				if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
				{
					// Illumination
					vec3d light_direction = { -0.5f, 0.5f, -0.5f };
					light_direction = Vector_Normalise(light_direction);

					// How "aligned" are light direction and triangle surface normal?
					float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

					// !!!
					// Choose console colours as required (much easier with RGB)
					CHAR_INFO c = GetColour(dp);
					triTransformed.col = c.Attributes;
					triTransformed.sym = c.Char.UnicodeChar;

					// Convert World Space --> View Space
					triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
					triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
					triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
					triViewed.sym = triTransformed.sym;
					triViewed.col = triTransformed.col;


					// Project triangles from 3D --> 2D
					triProjected.p[0] = Matrix_MultiplyVector(matProj, triViewed.p[0]);
					triProjected.p[1] = Matrix_MultiplyVector(matProj, triViewed.p[1]);
					triProjected.p[2] = Matrix_MultiplyVector(matProj, triViewed.p[2]);
					triProjected.col = triViewed.col;
					triProjected.sym = triViewed.sym;

					// Scale into view, we moved the normalising into cartesian space
					// out of the matrix
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					// X/Y are inverted
					triProjected.p[0].x *= -1.0f;
					triProjected.p[1].x *= -1.0f;
					triProjected.p[2].x *= -1.0f;
					triProjected.p[0].y *= -1.0f;
					triProjected.p[1].y *= -1.0f;
					triProjected.p[2].y *= -1.0f;

					// !!!
					// Offset verts into visible normalised space
					vec3d vOffsetView = { 1,1,0 }; // to the middle of the screen
					triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
					triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
					triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					// Store triangle for sorting
					vecTrianglesToRaster.push_back(triProjected);

				}
			}

			// Sort triangles from back to front
			sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
			});


			for (auto& t : vecTrianglesToRaster)
			{
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);
				//DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_WHITE);
			}
		}

		if (s)
		{
			// Store triagles for rastering later
			vector<triangle> vecTrianglesToRaster;
			// Draw Triangles
			meshSurface = cp2.surface1;
			for (auto tri : meshSurface.tris)
			{
				triangle triProjected, triTransformed, triViewed;

				// World Matrix Transform
				triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
				triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
				triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

				// Calculate triangle Normal
				vec3d normal, line1, line2;

				// Get lines either side of triangle
				line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
				line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

				// Take cross product of lines to get normal to triangle surface
				normal = Vector_CrossProduct(line1, line2);
				normal.x *= -1;
				normal.y *= -1;
				normal.z *= -1;

				// Normalise a normal
				normal = Vector_Normalise(normal);

				// Get ray from triangle to camera
				vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

				// If ray is aligned with normal, then triangle is visible
				if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
				{
					// Illumination
					vec3d light_direction = { -0.5f, 0.5f, -0.5f };
					light_direction = Vector_Normalise(light_direction);

					// How "aligned" are light direction and triangle surface normal?
					float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

					// Choose console colours as required (much easier with RGB)
					CHAR_INFO c = GetColour(dp);
					triTransformed.col = c.Attributes;
					triTransformed.sym = c.Char.UnicodeChar;

					// Convert World Space --> View Space
					triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
					triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
					triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
					triViewed.sym = triTransformed.sym;
					triViewed.col = triTransformed.col;


					// Project triangles from 3D --> 2D
					triProjected.p[0] = Matrix_MultiplyVector(matProj, triViewed.p[0]);
					triProjected.p[1] = Matrix_MultiplyVector(matProj, triViewed.p[1]);
					triProjected.p[2] = Matrix_MultiplyVector(matProj, triViewed.p[2]);
					triProjected.col = triViewed.col;
					triProjected.sym = triViewed.sym;

					// Scale into view, we moved the normalising into cartesian space
					// out of the matrix
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					// X/Y are inverted
					triProjected.p[0].x *= -1.0f;
					triProjected.p[1].x *= -1.0f;
					triProjected.p[2].x *= -1.0f;
					triProjected.p[0].y *= -1.0f;
					triProjected.p[1].y *= -1.0f;
					triProjected.p[2].y *= -1.0f;

					// Offset verts into visible normalised space
					vec3d vOffsetView = { 1,1,0 }; // to the middle of the screen
					triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
					triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
					triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					// Store triangle for sorting
					vecTrianglesToRaster.push_back(triProjected);

				}
			}

			// Sort triangles from back to front
			sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
				{
					float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
					float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
					return z1 > z2;
				});


			for (auto& t : vecTrianglesToRaster)
			{
				// !!!
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);
			}
		}

		if (1)
		{
			// Clear Screen
			std::vector<line> lineShape;
			std::vector<line> linesToDraw;
			line lineTem;

			std::vector<line> linesTem;

			vec3d p1, p2, p3, p4;





			lineShape = cp.lines;

			for (auto lin : lineShape)
			{
				line linProjected, linTransformed, linViewed;

				// World Matrix Transform
				linTransformed.p[0] = Matrix_MultiplyVector(matWorld, lin.p[0]);
				linTransformed.p[1] = Matrix_MultiplyVector(matWorld, lin.p[1]);

				// Convert World Space --> View Space
				linViewed.p[0] = Matrix_MultiplyVector(matView, linTransformed.p[0]);
				linViewed.p[1] = Matrix_MultiplyVector(matView, linTransformed.p[1]);



				// Project lines from 3D --> 2D
				linProjected.p[0] = Matrix_MultiplyVector(matProj, linViewed.p[0]);
				linProjected.p[1] = Matrix_MultiplyVector(matProj, linViewed.p[1]);


				// Scale into view, we moved the normalising into cartesian space
				// out of the matrix
				linProjected.p[0] = Vector_Div(linProjected.p[0], linProjected.p[0].w);
				linProjected.p[1] = Vector_Div(linProjected.p[1], linProjected.p[1].w);


				// X/Y are inverted
				linProjected.p[0].x *= -1.0f;
				linProjected.p[1].x *= -1.0f;
				linProjected.p[0].y *= -1.0f;
				linProjected.p[1].y *= -1.0f;

				// Offset verts into visible normalised space
				vec3d vOffsetView = { 1,1,0 }; // to the middle of the screen
				linProjected.p[0] = Vector_Add(linProjected.p[0], vOffsetView);
				linProjected.p[1] = Vector_Add(linProjected.p[1], vOffsetView);
				linProjected.p[0].x *= 0.5f * (float)ScreenWidth();
				linProjected.p[0].y *= 0.5f * (float)ScreenHeight();
				linProjected.p[1].x *= 0.5f * (float)ScreenWidth();
				linProjected.p[1].y *= 0.5f * (float)ScreenHeight();


				// Store lines for drawing
				linesToDraw.push_back(linProjected);


			}

			for (auto& t : linesToDraw)
			{
				// !!!
				DrawLine(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, PIXEL_SOLID, FG_GREEN);
			}


		}

		return true;
	}

};




int main()
{
	olcEngine3D demo;
	if (demo.ConstructConsole(400, 325, 2, 2))
	{
		demo.Start();
	}
		
	return 0;
}
