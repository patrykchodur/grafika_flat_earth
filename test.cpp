#include "pch.h"
#include <iostream>
#include <cmath>
#include "coordinates.hpp"
#include "objloader.hpp"

typedef sf::Event sfe;
typedef sf::Keyboard sfk;


Cartesian sun_position(0.6f, 0.0f, 0.6f);
CartesianDirected camera({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f});
float timer = 0.0;

sf::Texture TEXid;
sf::Shader shader[1];

sf::Texture earth_texture;
sf::Texture skybox_texture;

constexpr size_t water_polygons = 500;
constexpr float skybox_width = 32.f;
constexpr float sun_dist = 0.6f;

constexpr auto move_speed = 0.01f;
constexpr auto look_speed = 2.0f;

constexpr float fov = 45.0f;

sf::Vector3f flag[water_polygons][water_polygons];
sf::Vector2f flag_texcoord[water_polygons][water_polygons];

std::vector<sf::Vector3f> flat_earth_vertices;
std::vector<sf::Vector2f> flat_earth_uvs;
std::vector<sf::Vector3f> flat_earth_normals;

GLuint flat_earth_vertexbuffer;
GLuint flat_earth_uvbuffer;


void initOpenGL(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	GLfloat light_ambient_global[4] = { 0.01,0.01,0.01,1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient_global);

	TEXid.loadFromFile("water.jpg");
	earth_texture.loadFromFile("flat_earth_texture.png");
	skybox_texture.loadFromFile("skybox.jpg");

	shader[0].loadFromFile("shader_3.vert", "shader_3.frag");

	for (int x = 0; x < water_polygons; x++)
		for (int y = 0; y < water_polygons; y++)
		{
			flag[x][y].x = 2*x / static_cast<float>(water_polygons) - 1.0f; // (x - 10) * 0.1f;
			flag[x][y].y = 2*y / static_cast<float>(water_polygons) - 1.0f; // (y - 10) * 0.1f; // -1:1
			flag[x][y].z = 0.0f;

			flag_texcoord[x][y].x = (flag[x][y].x + 1.0f) / 2.0f;
			flag_texcoord[x][y].y = (flag[x][y].y + 1.0f) / 2.0f;
		}
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

void glNormalsf(sf::Vector3f v) {
	glNormal3f(v.x, v.y, v.z);
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

	// LIGHT SOURCE
	GLfloat light0_position[4] = { sun_position.getX(), sun_position.getY(), sun_position.getZ(), 0.0f }; 
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position); 



	// DRAWING SKYBOX
	sf::Vector2f texture_corner;
	sf::Vector3f skybox_corner = {-skybox_width/2.f, -skybox_width/2.f + 0.01f, -skybox_width/2.f};

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	sf::Texture::bind(&skybox_texture);

	glPushMatrix();
	glTranslatef(camera.getX(), camera.getY(), camera.getZ());

	auto draw_box = [&] (sf::Vector2f texture_corner, sf::Vector3f skybox_corner) {
		glBegin(GL_QUADS);
			glTexCoord2f(texture_corner.x, texture_corner.y);
			glVertex3f(skybox_corner.x, skybox_corner.y, skybox_corner.z);
			glTexCoord2f(texture_corner.x, texture_corner.y + 1.f/3.f);
			glVertex3f(skybox_corner.x, skybox_corner.y, skybox_corner.z + skybox_width);
			glTexCoord2f(texture_corner.x + 1.f/4.f, texture_corner.y + 1.f/3.f);
			glVertex3f(skybox_corner.x + skybox_width, skybox_corner.y, skybox_corner.z + skybox_width);
			glTexCoord2f(texture_corner.x + 1.f/4.f, texture_corner.y);
			glVertex3f(skybox_corner.x + skybox_width, skybox_corner.y, skybox_corner.z);
		glEnd();
	};

	glColor3f(1.f, 1.f, 1.f);
	texture_corner = {0.f, 1.f/3.f};
	draw_box(texture_corner, skybox_corner);

	glRotatef(90.f, 0, 0, 1);
	texture_corner = {1.f/4.f, 1.f/3.f};
	draw_box(texture_corner, skybox_corner);

	glRotatef(90.f, 0, 0, 1);
	texture_corner = {2.f/4.f, 1.f/3.f};
	draw_box(texture_corner, skybox_corner);

	glRotatef(90.f, 0, 0, 1);
	texture_corner = {3.f/4.f, 1.f/3.f};
	draw_box(texture_corner, skybox_corner);

	glRotatef(90.f, 1, 0, 0);
	texture_corner = {2.f/4.f, 0.f/3.f};
	draw_box(texture_corner, skybox_corner);

	glRotatef(180.f, 1, 0, 0);
	texture_corner = {2.f/4.f, 2.f/3.f};
	draw_box(texture_corner, skybox_corner);


	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	


	// DRAWING WATER

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	sf::Texture::bind(&TEXid);

	sf::Shader::bind(&shader[0]);

	glBegin(GL_QUADS);
	for (int x = 0; x < water_polygons - 1; x++)
		for (int y = 0; y < water_polygons - 1; y++)
		{
			float tmp_x = static_cast<float>(x) / water_polygons * 2.0f - 1.0f;
			float tmp_y = static_cast<float>(y) / water_polygons * 2.0f - 1.0f;

			if (tmp_x * tmp_x + tmp_y * tmp_y > 0.95f)
				continue;

			glTexCoordsf(flag_texcoord[x][y]);         glVertexsf(flag[x][y]);
			glTexCoordsf(flag_texcoord[x + 1][y]);     glVertexsf(flag[x + 1][y]);
			glTexCoordsf(flag_texcoord[x + 1][y + 1]); glVertexsf(flag[x + 1][y + 1]);
			glTexCoordsf(flag_texcoord[x][y + 1]);     glVertexsf(flag[x][y + 1]);
		}
	glEnd();

	sf::Shader::bind(NULL);
	glDisable(GL_TEXTURE_2D);


	glEnable(GL_TEXTURE_2D);
	sf::Texture::bind(&earth_texture);
	glPushMatrix();
		glRotatef(90.0f, 1, 0, 0);
		glTranslatef(0, -0.1, 0);
		glBegin(GL_TRIANGLES);
		for(size_t iter = 0; iter < flat_earth_vertices.size(); ++iter) {

			glNormalsf(flat_earth_normals[iter] + sf::Vector3f(flat_earth_vertices[iter].x, flat_earth_vertices[iter].y, flat_earth_vertices[iter].z));
			glTexCoord2f(0.5f*flat_earth_vertices[iter].x+ 0.5f, 0.5f*flat_earth_vertices[iter].z+0.5f);

			glVertexsf(flat_earth_vertices[iter]);
		}
		glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);


	// DRAWING SUN
	GLUquadricObj* qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluQuadricNormals(qobj, GLU_SMOOTH);

	glDisable(GL_LIGHTING);
	glPushMatrix();
		glColor3f(1.0f, 1.0f, 0.8f);
		glTranslatef(sun_position.getX(), sun_position.getY(), sun_position.getZ());
		glRotatef(0, 0, 0, 0);
		gluSphere(qobj, 0.07f, 50, 50);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	gluDeleteQuadric(qobj);

	// DRAWING MOON
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL);
	gluQuadricNormals(qobj, GLU_SMOOTH);

	glDisable(GL_COLOR_MATERIAL);
	glPushMatrix();
		glMaterialfv(GL_FRONT, GL_AMBIENT, MoonAmbient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, MoonDiffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, MoonSpecular);
		glMaterialf(GL_FRONT, GL_SHININESS, MoonShininess);
		glColor3f(0, 0, 1.0f);
		glTranslatef(-sun_position.getX(), -sun_position.getY(), sun_position.getZ());
		glRotatef(0, 0, 0, 0);
		gluSphere(qobj, 0.07f, 50, 50);
	glPopMatrix();
	glEnable(GL_COLOR_MATERIAL);

	gluDeleteQuadric(qobj);
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
		sun_position.x = sun_dist * std::cos(timer);
		sun_position.y = sun_dist * std::sin(timer);
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
