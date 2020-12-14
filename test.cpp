#include "pch.h"
#include <iostream>
#include <cmath>
#include "coordinates.hpp"
#include "objloader.hpp"

typedef sf::Event sfe;
typedef sf::Keyboard sfk;

void checkGLError()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::cerr << err;
    }
}


Spherical light_position(4.0f, 0.2f, 1.2f);
CartesianDirected camera({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f});
sf::Vector3f pos(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), rot(0.0f, 0.0f, 0.0f);
unsigned char projection_type = 'p';
float fov = 45.0f;
float timer = 0.0;

sf::Texture TEXid;
sf::Shader shader[1];

constexpr size_t water_polygons = 500;

sf::Vector3f flag[water_polygons][water_polygons]; //new
sf::Vector2f flag_texcoord[water_polygons][water_polygons]; //new

std::vector<sf::Vector3f> flat_earth_vertices;
std::vector<sf::Vector2f> flat_earth_uvs;
std::vector<sf::Vector3f> flat_earth_normals;

GLuint flat_earth_vertexbuffer;
GLuint flat_earth_uvbuffer;

constexpr auto move_speed = 0.01f;
constexpr auto look_speed = 2.0f;

void initOpenGL(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	GLfloat light_ambient_global[4] = { 0.5,0.5,0.5,1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient_global);

	TEXid.loadFromFile("water.jpg");

	shader[0].loadFromFile("shader_3.vert", "shader_3.frag");

	// ----------------------- begin new -----------------------------------
	for (int x = 0; x < water_polygons; x++)
		for (int y = 0; y < water_polygons; y++)
		{
			flag[x][y].x = 2*x / static_cast<float>(water_polygons) - 1.0f; // (x - 10) * 0.1f;
			flag[x][y].y = 2*y / static_cast<float>(water_polygons) - 1.0f; // (y - 10) * 0.1f; // -1:1
			flag[x][y].z = 0.0f;

			flag_texcoord[x][y].x = (flag[x][y].x + 1.0f) / 2.0f;
			flag_texcoord[x][y].y = (flag[x][y].y + 1.0f) / 2.0f;
		}
	// ----------------------- end new -----------------------------------
}

void glTexCoordsf(sf::Vector2f v)
{
	glTexCoord2f(v.x, v.y);
}


void reshapeScreen(sf::Vector2u size) 
{
	glViewport(0, 0, (GLsizei)size.x, (GLsizei)size.y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (GLdouble)size.x / (GLdouble)size.y, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void glVertexsf(sf::Vector3f v)
{
	glVertex3f(v.x, v.y, v.z);
}

void drawScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	auto currentPoint = camera.getCurrentPoint();
	auto lookAtPoint = camera.getLookAtPoint();
	auto northPoint = camera.getNorth();
	gluLookAt(currentPoint.getX(), currentPoint.getY(), currentPoint.getZ(), 
			lookAtPoint.getX(), lookAtPoint.getY(), lookAtPoint.getZ(), 
			northPoint.getX(), northPoint.getY(), northPoint.getZ()
			);

	GLfloat light0_position[4] = { light_position.getX(), light_position.getY(), light_position.getZ(), 0.0f }; 
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position); 

	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(0, 0, 0); glVertex3f(1.0, 0, 0);
	glColor3f(0.0, 1.0, 0.0); glVertex3f(0, 0, 0); glVertex3f(0, 1.0, 0);
	glColor3f(0.0, 0.0, 1.0); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1.0);
	glEnd();

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, 0xAAAA);
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(0, 0, 0); glVertex3f(-1.0, 0, 0);
	glColor3f(0.0, 1.0, 0.0); glVertex3f(0, 0, 0); glVertex3f(0, -1.0, 0);
	glColor3f(0.0, 0.0, 1.0); glVertex3f(0, 0, 0); glVertex3f(0, 0, -1.0);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	glTranslatef(pos.x, pos.y, pos.z);   
	glRotatef(rot.x, 1, 0, 0);           
	glRotatef(rot.y, 0, 1, 0);           
	glRotatef(rot.z, 0, 0, 1);           
	glScalef(scale.x, scale.y, scale.z); 

	glEnable(GL_LIGHTING);


	//------------------- begin new -------------------------------------------

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	sf::Texture::bind(&TEXid);

	sf::Shader::bind(&shader[0]);

	glBegin(GL_QUADS);
	for (int x = 0; x < water_polygons - 1; x++)
		for (int y = 0; y < water_polygons - 1; y++)
		{
			glTexCoordsf(flag_texcoord[x][y]);         glVertexsf(flag[x][y]);
			glTexCoordsf(flag_texcoord[x + 1][y]);     glVertexsf(flag[x + 1][y]);
			glTexCoordsf(flag_texcoord[x + 1][y + 1]); glVertexsf(flag[x + 1][y + 1]);
			glTexCoordsf(flag_texcoord[x][y + 1]);     glVertexsf(flag[x][y + 1]);
		}
	glEnd();

	sf::Shader::bind(NULL);
	glDisable(GL_TEXTURE_2D);

	//----------- end new ---------------------------
	

	glDisable(GL_COLOR_MATERIAL);
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, PolishedGoldAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, PolishedGoldDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, PolishedGoldSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, PolishedGoldShininess);
	// glColor3f(1.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 1, 0, 0);
	glTranslatef(0, -0.1, 0);
	glBegin(GL_TRIANGLES);
	for (auto&& iter : flat_earth_vertices)
		glVertexsf(iter);
	glEnd();
	glPopMatrix();
	glEnable(GL_COLOR_MATERIAL);

	/*
	// printing flat earth model
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, flat_earth_vertexbuffer);
	glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, flat_earth_uvbuffer);
	glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, flat_earth_vertices.size() );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	*/

}

void load_flat_earth() {
		// Read our .obj file
	bool res = loadOBJ("flat_earth.obj", flat_earth_vertices, flat_earth_uvs, flat_earth_normals);
	if (!res)
		std::cout << "Error loading earth\n";

	// Load it into a VBO
	
	std::vector<float> tmp_buffer;
	tmp_buffer.reserve(flat_earth_vertices.size());

	for (auto&& iter : flat_earth_vertices) {
		tmp_buffer.emplace_back(iter.x);
		tmp_buffer.emplace_back(iter.y);
		tmp_buffer.emplace_back(iter.z);
	}

	glGenBuffers(1, &flat_earth_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, flat_earth_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, flat_earth_vertices.size() * 3 * sizeof(float), &tmp_buffer[0], GL_STATIC_DRAW);

	tmp_buffer.clear();

	for (auto&& iter : flat_earth_uvs) {
		tmp_buffer.emplace_back(iter.x);
		tmp_buffer.emplace_back(iter.y);
	}

	glGenBuffers(1, &flat_earth_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, flat_earth_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, flat_earth_uvs.size() * 2 * sizeof(float), &tmp_buffer[0], GL_STATIC_DRAW);
}

int main(int argc, char* argv[])
{
	bool running = true;
	sf::ContextSettings context(24);// , 0, 0, 4, 1, sf::ContextSettings::Core);
	sf::RenderWindow window(sf::VideoMode(800, 600), "Open GL Lab1 17", 7U, context);
	sf::Clock clock;
	sf::Vector2i mouse_last_position(0, 0);

	window.setVerticalSyncEnabled(true);
	reshapeScreen(window.getSize());
	initOpenGL();
	
	load_flat_earth();

	while (running)
	{
		sfe event;
		sf::Time elapsed = clock.restart(); 
		timer += elapsed.asSeconds(); 
		shader[0].setUniform("time", 4 * timer);
		while (window.pollEvent(event))
		{
			if (event.type == sfe::Closed || (event.type == sfe::KeyPressed && event.key.code == sfk::Escape))
				running = false;
			if (event.type == sfe::Resized)
				reshapeScreen(window.getSize());
			if (event.type == sfe::MouseButtonPressed &&  event.mouseButton.button == sf::Mouse::Left)
				mouse_last_position = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
			if (event.type == sfe::MouseMoved && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				camera.turnRight(look_speed / window.getSize().x *(event.mouseMove.x - mouse_last_position.x));
				camera.turnUp(look_speed / window.getSize().y*(event.mouseMove.y - mouse_last_position.y));
				mouse_last_position = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
			}


		}
		if (sfk::isKeyPressed(sfk::Left)) camera.moveLeft(move_speed);
		if (sfk::isKeyPressed(sfk::Right)) camera.moveRight(move_speed);
		if (sfk::isKeyPressed(sfk::Up)) camera.moveUp(move_speed);
		if (sfk::isKeyPressed(sfk::Down)) camera.moveDown(move_speed);

		if (sfk::isKeyPressed(sfk::W)) camera.moveForward(move_speed);
		if (sfk::isKeyPressed(sfk::S)) camera.moveBackward(move_speed);
		if (sfk::isKeyPressed(sfk::A)) camera.moveLeft(move_speed);
		if (sfk::isKeyPressed(sfk::D)) camera.moveRight(move_speed);

		drawScene();
		window.display();
	}
	return 0;
}
