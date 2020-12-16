#include "Render.h"

#include <sstream>
#include <windows.h>
#include <time.h>
#include <iostream>
#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h";

SYSTEMTIME tm;

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;
bool setToStart = false;
bool PlanetsMovement = true;
bool PlanetsRot = true;
bool ShowTraj = false;
bool SunType = false;

int CameraPos = 0;



#define SUN Vector3(0,0,0)

Vector3 EarthPos, MercuryPos, VenusPos, MarsPos, JupiterPos, SaturnPos, UranPos, NeptunPos;
Vector3 CamLookPoint = SUN;

ObjFile Sun, Sun2, Earth, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uran, Neptun, SatCircle;

Texture SunText, EarthText, MoonText, MercuryText, VenusText, MarsText, JupiterText, SaturnText, UranText, NeptunText, SatCircleText;

//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(CamLookPoint.X(), CamLookPoint.Y(), CamLookPoint.Z());

		pos.setCoords(CamLookPoint.X() + camDist*cos(fi2)*cos(fi1),
			CamLookPoint.Y() + camDist*cos(fi2)*sin(fi1),
			CamLookPoint.Z() + camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}

	void ChangLP() 
	{
		pos.setCoords(15, 15, 15);
		lookPoint.setCoords(13, 13, 13);
		normal.setCoords(0, 0, 1);
		LookAt();
	}

}  camera;   //создаем объект камеры


//класс недоделан!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		//pos = Vector3(1, 1, 3);
		pos = Vector3(0, 0, 0);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G') && SunType)
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.5, 0.5, 0.5, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON) && SunType)
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON) && SunType)
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}


void ChangeCamPosition() 
{
	switch (CameraPos) 
	{
	case 0: { CamLookPoint = SUN; break; }
	case 1: { CamLookPoint = MercuryPos; break; }
	case 2: { CamLookPoint = VenusPos; break; }
	case 3: { CamLookPoint = EarthPos; break; }
	case 4: { CamLookPoint = MarsPos; break; }
	case 5: { CamLookPoint = JupiterPos; break; }
	case 6: { CamLookPoint = SaturnPos; break; }
	case 7: { CamLookPoint = UranPos; break; }
	case 8: { CamLookPoint = NeptunPos; break; }
	}
}


//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL *ogl, int key)
{
	if (OpenGL::isKeyPressed('F') && SunType)
	{
		light.pos = camera.pos;
	}

	if (key == 'Q')
		Time = 0;

	if (OpenGL::isKeyPressed('Z'))
		PlanetsMovement = !PlanetsMovement;

	if (OpenGL::isKeyPressed('C'))
		PlanetsRot = !PlanetsRot;

	if (OpenGL::isKeyPressed('X'))
		setToStart = true;
	else
		setToStart = false;

	if (OpenGL::isKeyPressed('N')) 
	{
		CameraPos--;
		CameraPos < 0 ? CameraPos = 8 : CameraPos;
	}

	if (OpenGL::isKeyPressed('M')) 
	{
		CameraPos++;
		CameraPos > 8 ? CameraPos = 0 : CameraPos;
	}

	if (OpenGL::isKeyPressed('B'))
	{
		ShowTraj = !ShowTraj;
	}

	if (OpenGL::isKeyPressed('V'))
	{
		SunType = !SunType;
		if (!SunType) light.pos.setCoords(0.01, 0.01, 0.01);
		if (SunType) light.pos.setCoords(0, 0, 15);
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{

}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel,monkey;

Texture monkeyTex;



//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{

	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	//camera.ChangLP();

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	

	 //так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	 // создающимися во время компиляции, я переименовал модели в *.obj_m

	loadModel("models\\sun_wnorm.obj_m", &Sun2);
	SunText.loadTextureFromFile("textures//sun.bmp");

	loadModel("models\\sun.obj_m", &Sun);
	SunText.loadTextureFromFile("textures//sun.bmp");

	loadModel("models\\earth.obj_m", &Earth);
	EarthText.loadTextureFromFile("textures//earth.bmp");

	loadModel("models\\moon.obj_m", &Moon);
	MoonText.loadTextureFromFile("textures//moon.bmp");

	loadModel("models\\mercury.obj_m", &Mercury);
	MercuryText.loadTextureFromFile("textures//mercury.bmp");

	loadModel("models\\venus.obj_m", &Venus);
	VenusText.loadTextureFromFile("textures//venus.bmp");

	loadModel("models\\mars.obj_m", &Mars);
	MarsText.loadTextureFromFile("textures//mars.bmp");

	loadModel("models\\jup_sat.obj_m", &Jupiter);
	JupiterText.loadTextureFromFile("textures//jupiter.bmp");

	loadModel("models\\jup_sat.obj_m", &Saturn);
	SaturnText.loadTextureFromFile("textures//saturn.bmp");

	loadModel("models\\ur_nep.obj_m", &Uran);
	UranText.loadTextureFromFile("textures//uran.bmp");

	loadModel("models\\ur_nep.obj_m", &Neptun);
	NeptunText.loadTextureFromFile("textures//neptune.bmp");

	loadModel("models\\sat_circle.obj_m", &SatCircle);
	SatCircleText.loadTextureFromFile("textures//saturn.bmp");

	glActiveTexture(GL_TEXTURE0);


	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);

	std::stringstream ss;
	ss << "Z - Вкл/выкл движение планет" << std::endl;
	ss << "X - Посставить планеты в начальную точку" << std::endl;
	ss << "C - Вкл/выкл вращение планет" << std::endl;
	ss << "N - Предыдущая планета" << std::endl;
	ss << "M - Следущая планета" << std::endl;
	ss << "V - Солнце - источник света вкл/выкл" << std::endl;
	ss << "B - Вкл/выкл траектории движения" << std::endl;
	ss << "F - Переместить источник света к камере" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертикали" << std::endl;

	rec.setText(ss.str().c_str());
}

void DrawSun(double A)
{
	PUSH;
	SunText.bindTexture();
	glRotated(A, 0, 0, 1);
	if (SunType)
		Sun.DrawObj();
	else
		Sun2.DrawObj();
	POP;
}

Vector3 DrawEarth(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	EarthText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Earth.DrawObj();
	POP;

	return pos;
}

Vector3 DrawMoon(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	MoonText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Moon.DrawObj();
	POP;

	return pos;
}

Vector3 DrawMercury(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	MercuryText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Mercury.DrawObj();
	POP;

	return pos;
}

Vector3 DrawVenus(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	VenusText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Venus.DrawObj();
	POP;

	return pos;
}

Vector3 DrawMars(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	MarsText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Mars.DrawObj();
	POP;

	return pos;
}

Vector3 DrawJupiter(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	JupiterText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Jupiter.DrawObj();
	POP;

	return pos;
}

Vector3 DrawSaturn(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	SaturnText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Saturn.DrawObj();
	POP;

	return pos;
}

Vector3 DrawSaturnCircle(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	SatCircleText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	SatCircle.DrawObj();
	POP;

	return pos;
}

Vector3 DrawUran(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	UranText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Uran.DrawObj();
	POP;

	return pos;
}

Vector3 DrawNeptun(double t, double SMajorAxis, double SMinorAxis, double angl, double A)
{
	angl = (angl * (M_PI / 180));

	double x = (SUN.X() + SMajorAxis * cos(t));
	double y = (SUN.Y() + SMinorAxis * sin(t));
	double z = y * sin(angl);

	Vector3 pos;
	pos.setCoords(x, y, z);

	PUSH;
	NeptunText.bindTexture();
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(A, 0, 0, 1);
	Neptun.DrawObj();
	POP;

	return pos;
}

void BuildCircleTrajectory(double a, double b, double angle)
{
	angle = (angle * (M_PI / 180));

	double h = 0.01;
	double x;
	double y;


	glBegin(GL_LINE_STRIP);
	for (double t = 0; t <= 2 * M_PI; t += h)
	{
		x = (a * cos(t));
		y = (b * sin(t));
		double z = y * sin(angle);
		glVertex3d(x, y, z);
	}
	glEnd();

}

void Render(OpenGL *ogl)
{   
	GetSystemTime(&tm);
	
	static int prevSec;
	int Sec = tm.wMilliseconds;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);


	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);



	GLfloat amb[] = { 0.1, 0.1, 0.1, 1 };
	GLfloat dif[] = { 0.9, 0.9, 0.9, 1 };
	GLfloat spec[] = { 1, 1, 0, 1 };
	GLfloat sh = 3.0f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//Прогать тут  

	ChangeCamPosition();

	static double A = 0, A1 = 0;
	static double t[8] = { 0 };
	double h = 0.01;

	

	if (Sec != prevSec)
	{
		prevSec = Sec;
		if (!PlanetsRot);
		else 
		{
			A1 += 0.5;
			A += 1;
		}
		if (PlanetsMovement) ;
		else 
		{
			t[0] += 2 * h;
			t[1] += 1.5 * h;
			t[2] += 1.2 * h;
			t[3] += h;
			t[4] += 0.95 * h;
			t[5] += 0.9 * h;
			t[6] += 0.85 * h;
			t[7] += 0.8 * h;
		}
		if (setToStart) 
		{
			t[0] = 0;
			t[1] = 0;
			t[2] = 0;
			t[3] = 0;
			t[4] = 0;
			t[5] = 0;
			t[6] = 0;
			t[7] = 0;
		}
	}

	if (ShowTraj)
	{
		BuildCircleTrajectory(4, 4, 7.01 * 2);
		BuildCircleTrajectory(6, 6, 3.39 * 2);
		BuildCircleTrajectory(9, 9, 0);
		BuildCircleTrajectory(12, 12, 1.85 * 2);
		BuildCircleTrajectory(16, 16, 1.31 * 2);
		BuildCircleTrajectory(23, 23, 2.49 * 2);
		BuildCircleTrajectory(28, 28, 0.77 * 2);
		BuildCircleTrajectory(32, 32, 1.77 * 2);
	}

	if (A >= 360) A = 0;

	
	DrawSun(A1);
	MercuryPos = DrawMercury(t[0], 4, 4, 7.01 * 2, A);
	VenusPos = DrawVenus(t[1] + h, 6, 6, 3.39 * 2, A);
	EarthPos = DrawEarth(t[2], 9, 9, 0, A);
	DrawMoon(t[2], 9, 9, 0, A);
	MarsPos = DrawMars(t[3], 12, 12, 1.85 * 2, A);
	JupiterPos = DrawJupiter(t[4], 16, 16, 1.31 * 2, A);
	SaturnPos = DrawSaturn(t[5], 23, 23, 2.49 * 2, A);
	DrawSaturnCircle(t[5], 23, 23, 2.49 * 2, A);
	UranPos = DrawUran(t[6], 28, 28, 0.77 * 2, A);
	NeptunPos = DrawNeptun(t[7], 32, 32, 1.77 * 2, A);

	if (t[0] > 2 * M_PI) t[0] = 0;
	if (t[1] > 2 * M_PI) t[1] = 0;
	if (t[2] > 2 * M_PI) t[2] = 0;
	if (t[3] > 2 * M_PI) t[3] = 0;
	if (t[4] > 2 * M_PI) t[4] = 0;
	if (t[5] > 2 * M_PI) t[5] = 0;
	if (t[6] > 2 * M_PI) t[6] = 0;
	if (t[7] > 2 * M_PI) t[7] = 0;

	Shader::DontUseShaders();

	
	
}   //конец тела функции


bool gui_init = true;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	glActiveTexture(GL_TEXTURE0);
	rec.Draw();


		
	Shader::DontUseShaders(); 



	
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 200 - 10);
	//rec.setPosition(10, ogl->getHeight() - 200 - 10);
}

