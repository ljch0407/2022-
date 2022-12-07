#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

//좌표계 변환용 헤더파일
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include "filetobuf.h"
#include <iostream>
#include <vector>
#include <atlfile.h>
#include <random>

#define METEO_MAX 5

using namespace std;

GLvoid drawScene(GLvoid);
GLvoid ReShape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLuint make_shaderProgram();

void make_vertexShaders();
void make_fragmentShaders();

bool loadObj(const char* path, vector<glm::vec3>& out_verteices, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals);

void TimerFunction1(int value);
void TimerFunction2(int value);
void TimerFunction3(int value);
void TimerFunction4(int value);
void TimerFunction5(int value);
void TimerFunction6(int value);



void SpecialKey(int key, int x, int y);

GLuint width, height;
GLuint shaderID; //셰이더 프로그램 이름
GLuint vertexShader; // 버텍스 셰이더 객체
GLuint fragmentShader; // 프래그먼트 셰이더 객체
GLuint s_program;


GLfloat rColor, gColor, bColor;

void InitBuffer();
void InitShader();

random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dist(0, 18);


vector<glm::vec3> vertices_cube;
vector<glm::vec2> uvs_cube;
vector<glm::vec3> normals_cube;

vector<glm::vec3> vertices_player;
vector<glm::vec2> uvs_player;
vector<glm::vec3> normals_player;



bool cube = loadObj("cube.obj", vertices_cube, uvs_cube, normals_cube);
//bool player = loadObj("player.obj", vertices_player, uvs_player, normals_player);
GLuint VAO, VBO, VBO_NormalCube;
GLuint VAO_Player, VBO_Player, VBO_Player_normal;
GLuint colorBuffer, colorBuffercorn;


bool timer1 = false;
bool timer2 = false;
bool timer3 = false;
bool timer4 = false;
bool timer6 = true;

float PlayerRotateZ = 0.0f;
float PlayerRotateX = 0.0f;

float PlayerX = 0.0f;
float PlayerY = 0.0f;

float ParticleStart = 0.5f;
float ParticleZ = 0.5f;
float missileZ = 7;

float missile_xpos = 0;
float missile_ypos = 0;

float missileRotateX = 0;
float missileRotateZ = 0;

bool Is_missile_ready = true;
int MeteoCounter = 0;
struct Meteo
{
	float Xpos;
	float Ypos;
	float Zpos;

	bool hit = false;
};


Meteo Meteos[METEO_MAX];

void main(int argc, char* argv[])
{

	width = 800;
	height = 600;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Meteo Avoid");

	//Glew 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		cerr << "Unable to initialize GLEW" << endl;
		exit(EXIT_FAILURE);
	}
	else
		cout << "GLEW Initialized" << endl;

	glewInit();



	for (int i = 0; i < 5; ++i)
	{
		if (i % 2 == 0)
		{
			Meteos[i].Xpos = dist(rd) / 10;
			Meteos[i].Ypos = dist(rd) / 10;
		}
		else
		{
			Meteos[i].Xpos = -(dist(rd) / 10);
			Meteos[i].Ypos = -(dist(rd) / 10);
		}
		Meteos[i].Zpos = -25.f;
	}

	glutTimerFunc(100, TimerFunction6, 1); 
	glutTimerFunc(100, TimerFunction5, 1);

	InitShader();
	InitBuffer();
	glutKeyboardFunc(Keyboard);
	glutDisplayFunc(drawScene);
	glutReshapeFunc(ReShape);
	glutSpecialFunc(SpecialKey);

	glutMainLoop();
}


void make_vertexShaders()
{
	GLchar* vertexsource;

	vertexsource = filetobuf((char*)"VertexShaders.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexsource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errLog);
		cerr << "Error : vertex Shader 컴파일 실패 \n" << errLog << endl;
		//return false;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentsource;
	fragmentsource = filetobuf((char*)"FragShader.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errLog);
		cerr << "Error : fragment Shader 컴파일 실패 \n" << errLog << endl;
		//return false;
	}
}

GLuint make_shaderProgram()
{
	GLuint ShaderProgramID;
	ShaderProgramID = glCreateProgram();

	glAttachShader(ShaderProgramID, vertexShader);
	glAttachShader(ShaderProgramID, fragmentShader);

	glLinkProgram(ShaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errLog[512];
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errLog);
		cerr << "Error : shader Program 연결 실패 \n" << errLog << endl;
		return false;
	}

	glUseProgram(ShaderProgramID);
	return ShaderProgramID;
}

void InitBuffer()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices_cube.size() * sizeof(glm::vec3), &vertices_cube[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &VBO_NormalCube);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_NormalCube);
	glBufferData(GL_ARRAY_BUFFER, normals_cube.size() * sizeof(glm::vec3), &normals_cube[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_NormalCube);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//----------------------------------------------------------------------------------------------------------------------------------

	//glGenVertexArrays(1, &VAO_Player);
	//glBindVertexArray(VAO_Player);

	//glGenBuffers(1, &VBO_Player);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO_Player);
	//glBufferData(GL_ARRAY_BUFFER, vertices_player.size() * sizeof(glm::vec3), &vertices_player[0], GL_STATIC_DRAW);

	//glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO_Player);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//glGenBuffers(1, &VBO_Player_normal);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO_Player_normal);
	//glBufferData(GL_ARRAY_BUFFER, normals_player.size() * sizeof(glm::vec3), &normals_player[0], GL_STATIC_DRAW);

	//glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO_Player_normal);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//---------------------------------------------------------------------------------------------------------------------------------

}

void InitShader()
{
	make_vertexShaders();
	make_fragmentShaders();

	s_program = glCreateProgram();

	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(s_program);
}

GLvoid ReShape(int w, int h)
{
	glViewport(0, 0, w, h);
}


GLvoid drawScene()
{
	glClearColor(rColor, gColor, bColor, 1.0f);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glUseProgram(s_program);

	unsigned int modelLocation_XY = glGetUniformLocation(s_program, "modelMatrix");
	unsigned int viewLocation = glGetUniformLocation(s_program, "viewMatrix");
	unsigned int ProjLoc = glGetUniformLocation(s_program, "projectionMatrix");

	// 카메라 세팅
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	viewMatrix = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));


	//투영변환 세팅
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, &projection[0][0]);

	//라이트 세팅
	unsigned int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
	glUniform3f(lightPosLocation, 0, 1.0, 0);
	unsigned int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	glUniform3f(lightColorLocation, 1, 1, 1);
	unsigned int viewPosLocation = glGetUniformLocation(s_program, "viewPos");
	glUniform3f(viewPosLocation, 0, 1, -6);

	//Player Settings
	{
		unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");
		glUniform3f(objColorLocation, 1.0f, 0.0, 0.0);

		glm::mat4 Player = glm::mat4(1.0f);
		

		Player = glm::translate(Player, glm::vec3(PlayerX, PlayerY, 0.0f));

		Player = glm::rotate(Player, glm::radians(PlayerRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
		Player = glm::rotate(Player, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Player = glm::rotate(Player, glm::radians(PlayerRotateZ), glm::vec3(0.0f, 0.0f, 1.0f));

		Player = glm::scale(Player, glm::vec3(0.5f, 0.5f, 0.5f));

		unsigned int  modelLocation = glGetUniformLocation(s_program, "modelMatrix");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Player));
		glBindVertexArray(VAO);
		//glBindVertexArray(VAO_Player);
		glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
	}

	//Particles
	{
		unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");
		glUniform3f(objColorLocation, 1.0f, 1.0f, 0.0f);

		glm::mat4 Particle = glm::mat4(1.0f);


		Particle = glm::translate(Particle, glm::vec3(PlayerX, PlayerY, ParticleZ));

		Particle = glm::rotate(Particle, glm::radians(PlayerRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
		Particle = glm::rotate(Particle, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Particle = glm::rotate(Particle, glm::radians(PlayerRotateZ), glm::vec3(0.0f, 0.0f, 1.0f));

		Particle = glm::scale(Particle, glm::vec3(0.05f, 0.05f, 0.05f));

		unsigned int  modelLocation = glGetUniformLocation(s_program, "modelMatrix");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Particle));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
	}

	//projectile
	{
		unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");
		glUniform3f(objColorLocation, 0.0f, 0.0f, 1.0f);

		glm::mat4 missile = glm::mat4(1.0f);


		missile = glm::translate(missile, glm::vec3(missile_xpos, missile_ypos, missileZ));

		missile = glm::rotate(missile, glm::radians(missileRotateX), glm::vec3(1.0f, 0.0f, 0.0f));
		missile = glm::rotate(missile, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		missile = glm::rotate(missile, glm::radians(missileRotateZ), glm::vec3(0.0f, 0.0f, 1.0f));

		missile = glm::scale(missile, glm::vec3(0.25f, 0.25f, 0.25f));

		unsigned int  modelLocation = glGetUniformLocation(s_program, "modelMatrix");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(missile));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
	}


	if (MeteoCounter < METEO_MAX)
	{
		{
			unsigned int objColorLocation = glGetUniformLocation(s_program, "objectColor");
			glUniform3f(objColorLocation, 1.0f, 1.0f, 1.0f);

			glm::mat4 Meteo = glm::mat4(1.0f);


			Meteo = glm::translate(Meteo, glm::vec3(Meteos[MeteoCounter].Xpos, Meteos[MeteoCounter].Ypos, Meteos[MeteoCounter].Zpos));

			Meteo = glm::rotate(Meteo, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			Meteo = glm::rotate(Meteo, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			Meteo = glm::rotate(Meteo, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			Meteo = glm::scale(Meteo, glm::vec3(0.5f, 0.5f, 0.5f));

			unsigned int  modelLocation = glGetUniformLocation(s_program, "modelMatrix");
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Meteo));
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
		}
	}
	

	timer3 = true;
	glutTimerFunc(10, TimerFunction3, 1);

	glutSwapBuffers();
}

bool loadObj(const char* path, vector<glm::vec3>& out_verteices, vector<glm::vec2>& out_uvs, vector<glm::vec3>& out_normals)
{
	vector<unsigned int> vertexIndices, uvindices, normalIndices;
	vector<glm::vec3> temp_vertices;
	vector<glm::vec2> temp_uvs;
	vector<glm::vec3> temp_normals;

	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file \n");
		return false;
	}

	while (1)
	{
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;


		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			if (matches != 9)
			{
				printf("File can't be read by our simple parser : Try exporting with other options\n");
				return false;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);

			uvindices.push_back(uvIndex[0]);
			uvindices.push_back(uvIndex[1]);
			uvindices.push_back(uvIndex[2]);

			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		out_verteices.push_back(vertex);
	}

	for (unsigned int i = 0; i < normalIndices.size(); i++)
	{
		unsigned normalIndex = normalIndices[i];
		glm::vec3 normal = temp_normals[normalIndex - 1];
		out_normals.push_back(normal);
	}


}


GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		timer1 = true;
		PlayerRotateZ = 30.f;
		PlayerX -= 0.1f;
		glutTimerFunc(100, TimerFunction1, 1);
		break;
	case 'd':
		timer1 = true;
		PlayerX += 0.1f;
		PlayerRotateZ = -30.f;
		glutTimerFunc(100, TimerFunction1, 1);
		break;

	case 'w':
		timer2 = true;
		PlayerY += 0.1f;
		PlayerRotateX = 30.f;
		glutTimerFunc(100, TimerFunction2, 1);
		break;
	case 's':
		timer2 = true;
		PlayerY -= 0.1f;
		PlayerRotateX = -30.f;
		glutTimerFunc(100, TimerFunction2, 1);
		break;

	case 'k':
		missile_xpos = PlayerX;
		missile_ypos = PlayerY;
		Is_missile_ready = false;
		missileZ = 0;
		glutTimerFunc(100, TimerFunction4, 1);
		break;


	case 'q':
		exit(1);
		break;
	}
	glutPostRedisplay();
}

void TimerFunction1(int value)
{

	
	if (PlayerRotateZ > 0)
		PlayerRotateZ--;

	if (PlayerRotateZ < 0)
		PlayerRotateZ++;

	if (PlayerX < -1.8)
		PlayerX = -1.8f;

	if (PlayerX > 1.8)
		PlayerX = 1.8f;

	glutPostRedisplay();

	if (PlayerRotateZ == 0)
		timer1 = false;

	

	if(timer1)
		glutTimerFunc(100, TimerFunction1, 1);
}


void TimerFunction2(int value)
{

	if (PlayerRotateX > 0)
		PlayerRotateX--;

	if (PlayerRotateX < 0)
		PlayerRotateX++;

	if (PlayerY < -1.8)
		PlayerY = -1.8f;

	if (PlayerY > 1.8)
		PlayerY = 1.8f;


	glutPostRedisplay();

	if (PlayerRotateX == 0)
		timer2 = false;

	if (timer2)
		glutTimerFunc(100, TimerFunction2, 1);
}

void TimerFunction3(int value)
{
	ParticleZ += 0.001f;
	
	if (ParticleZ > 5.9)
		ParticleZ = 0.5f;

	glutPostRedisplay();

	if (timer3)
		glutTimerFunc(100, TimerFunction3, 1);
}


void TimerFunction4(int value)
{

	missileZ -= 2.f;
	
	missileRotateX--;
	missileRotateZ++;

	if (missileZ < -40.f)
	{
		Is_missile_ready = true;
		missileZ = 7;
	}


	if (!Is_missile_ready)
		glutTimerFunc(100, TimerFunction4, 1);
}



void TimerFunction5(int value)
{

	Meteos[MeteoCounter].Zpos++;

	if (Meteos[MeteoCounter].Zpos > 5.f)
	{
		MeteoCounter++;
	}
	
	if(MeteoCounter< METEO_MAX)
		glutTimerFunc(100, TimerFunction5, 1);
}



void TimerFunction6(int value)
{


	if (MeteoCounter >= METEO_MAX)
	{

		rColor += 0.1f;
		gColor += 0.1f;
		bColor += 0.1f;
	}

	glutPostRedisplay();
	if (timer6)
		glutTimerFunc(100, TimerFunction6, 1);
}




void SpecialKey(int key, int x, int y)
{

	glutPostRedisplay();
}