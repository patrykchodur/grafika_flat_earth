#include "pch.h"
#include <iostream>
#include <cmath>

typedef sf::Event sfe;
typedef sf::Keyboard sfk;

struct Spherical
{
	float distance, theta, fi;
	Spherical(float gdistance, float gtheta, float gfi) : distance(gdistance) , theta(gtheta), fi(gfi) { }
	float getX() { return distance * std::cos(theta)*std::cos(fi); }
	float getY() { return distance * std::sin(theta); }
	float getZ() { return distance * std::cos(theta)*std::sin(fi); }
};

struct Cartesian {
	float x, y, z;
	Cartesian(float x, float y, float z) : x(x), y(y), z(z) { }
	float getX() { return x; }
	float getY() { return y; }
	float getZ() { return z; }
	friend std::ostream& operator<<(std::ostream& stream, const Cartesian& cart) {
		stream << "Cartesian: x=" << cart.x << ", y=" << cart.y << ", z=" << cart.z;
		return stream;
	}
};

struct LookDirection {
	// alfa: left - right - -M_PI : M_PI
	// gamma: bottom - top - -M_PI/2 : M_PI/2
	float alfa, gamma;
	LookDirection(float alfa, float gamma) : alfa(alfa), gamma(gamma) { }
};

struct CartesianDirected : public Cartesian, public LookDirection {
	CartesianDirected(Cartesian car, LookDirection dir) : Cartesian(car), LookDirection(dir) { }
	CartesianDirected(const CartesianDirected&) = default;
	CartesianDirected(CartesianDirected&&) = default;
	CartesianDirected& moveForward(float dist) {
		x += std::cos(alfa) * std::cos(gamma) * dist;
		y += std::sin(alfa) * std::cos(gamma) * dist;
		z += std::sin(gamma) * dist;
		return *this;
	}
	CartesianDirected& moveRight(float dist) {
		x += std::sin(alfa) * dist;
		y -= std::cos(alfa) * dist;
		return *this;
	}
	CartesianDirected& moveLeft(float dist) {
		return moveRight(-dist);
	}
	CartesianDirected& moveUp(float dist) {
		z += dist;
		return *this;
	}
	CartesianDirected& turnRight(float angle) {
		alfa += angle;
		alfa = std::fmodf(alfa + M_PI , 2.0f * M_PI) - M_PI;
		return *this;
	}
	Cartesian& turnUp(float angle) {
		gamma += angle;
		gamma = std::fmin(gamma, M_PI/2.f);
		gamma = std::fmax(gamma, -M_PI/2.f);
		return *this;
	}
	Cartesian getCurrentPoint() const { return *this; }
	Cartesian getLookAtPoint() const {
		auto result = *this;
		result.moveForward(1.0);
		return result;
	}
	Cartesian getNorth() const {
		auto result = *this;
		result.moveUp(1.0);
		return result;
	}
	friend std::ostream& operator<<(std::ostream& stream, const CartesianDirected& cart) {
		stream << "CartesianDirected: x=" << cart.x << ", y=" << cart.y << ", z=" << cart.z << ", alfa=" << cart.alfa << ", gamma=" << cart.gamma;
		return stream;
	}
};


Spherical /* camera(3.0f, 0.2f, 1.2f), */ light_position(4.0f, 0.2f, 1.2f);
CartesianDirected camera({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f});
sf::Vector3f pos(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), rot(0.0f, 0.0f, 0.0f);
unsigned char projection_type = 'p';
float fov = 45.0f;
float timer = 0.0;

bool write_thread_is_up = false;
GLubyte *data;

sf::Texture TEXid;
sf::Shader shader[1];
sf::Vector3f flag[20][20]; //new
sf::Vector2f flag_texcoord[20][20]; //new

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
	for (int x = 0; x < 20; x++)
		for (int y = 0; y < 20; y++)
		{
			flag[x][y].x = (x - 10) * 0.1f;
			flag[x][y].y = 0.0f;
			flag[x][y].z = (y - 10) * 0.1f;

			flag_texcoord[x][y].x = (flag[x][y].x + 1.0f) / 2.0f;
			flag_texcoord[x][y].y = (flag[x][y].z + 1.0f) / 2.0f;
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
	if (projection_type == 'p') gluPerspective(fov, (GLdouble)size.x / (GLdouble)size.y, 0.1, 100.0);
	else glOrtho(-1.245*((GLdouble)size.x / (GLdouble)size.y), 1.245*((GLdouble)size.x / (GLdouble)size.y), -1.245, 1.245, -3.0, 12.0); 
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
	std::cout << "\n\n\n";
	std::cout << "\033[3A";
	std::cout << "currentPoint: " << currentPoint << '\n';
	std::cout << "lookAtPoint: " << lookAtPoint << '\n';
	std::cout << "northPoint: " << northPoint << '\n';
	std::cout << "\033[3A";
	// Spherical north_of_camera(camera.distance, camera.theta + 0.01f, camera.fi); 
	gluLookAt(currentPoint.getX(), currentPoint.getY(), currentPoint.getZ(), 
			lookAtPoint.getX(), lookAtPoint.getY(), lookAtPoint.getZ(), 
			// northPoint.getX(), northPoint.getY(), northPoint.getZ()
			0, 0, 1e15
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
	for (int x = 0; x < 19; x++)
		for (int y = 0; y < 19; y++)
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
}

void write_data_to_disk(unsigned int sw,unsigned int sh)
{
	sf::Image image;
	image.create(sw, sh, data);
	image.flipVertically();
	image.saveToFile("screen.png");

	delete[]data;
	write_thread_is_up = false;
}

int main(int argc, char* argv[])
{
	bool running = true;
	// sf::ContextSettings context(24, 0, 0, 4, 5);
	sf::RenderWindow window(sf::VideoMode(800, 600), "Open GL Lab1 17"); // , 7U, context);
	int shift_key_state = 1;
	sf::Clock clock;
	sf::Vector2i mouse_last_position(0, 0);

	window.setVerticalSyncEnabled(true);
	reshapeScreen(window.getSize());
	initOpenGL();

	while (running)
	{
		sfe event;
		sf::Time elapsed = clock.restart(); 
		timer += elapsed.asSeconds(); 
		shader[0].setUniform("time", timer);
		while (window.pollEvent(event))
		{
			if (event.type == sfe::Closed || (event.type == sfe::KeyPressed && event.key.code == sfk::Escape) ) running = false;
			if (event.type == sfe::Resized) reshapeScreen(window.getSize());
			if (event.type == sf::Event::KeyPressed) 
			{
				if (event.key.code == sf::Keyboard::Num0) { projection_type = 'o'; reshapeScreen(window.getSize()); } 
				if (event.key.code == sf::Keyboard::Num9) { projection_type = 'p'; reshapeScreen(window.getSize()); } 
			}
			if (event.type == sfe::MouseButtonPressed &&  event.mouseButton.button == sf::Mouse::Left)
			{
				mouse_last_position = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
			}
			if (event.type == sfe::MouseMoved && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				camera.turnRight(4.0f / window.getSize().x *(event.mouseMove.x - mouse_last_position.x));
				camera.turnUp(4.0f / window.getSize().y*(event.mouseMove.y - mouse_last_position.y));
				mouse_last_position = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
			}

			if (event.key.code == sfk::F1 && !write_thread_is_up)
			{
				write_thread_is_up = true;
				sf::Vector2u size = window.getSize();
				data = new GLubyte[size.x * size.y * 4];

				glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
				std::thread (write_data_to_disk, size.x, size.y).detach();
			}

		}
		if (sfk::isKeyPressed(sfk::Left)) camera.moveLeft(0.01f);
		if (sfk::isKeyPressed(sfk::Right)) camera.moveRight(0.01f);
		if (sfk::isKeyPressed(sfk::Up)) camera.moveUp(0.01f);
		if (sfk::isKeyPressed(sfk::Down)) camera.moveUp(-0.01f);

		if (sfk::isKeyPressed(sfk::W)) camera.moveForward(0.01f), std::cout << camera << '\n';
		if (sfk::isKeyPressed(sfk::S)) camera.moveForward(-0.01f);
		if (sfk::isKeyPressed(sfk::A)) camera.moveLeft(0.01f);
		if (sfk::isKeyPressed(sfk::D)) camera.moveRight(0.01f);
		/*
		if (sfk::isKeyPressed(sfk::I)) light_position.fi -= 0.01f; 
		if (sfk::isKeyPressed(sfk::O)) light_position.fi += 0.01f; 
		if (sfk::isKeyPressed(sfk::K)) light_position.theta += 0.01f; 
		if (sfk::isKeyPressed(sfk::L)) light_position.theta -= 0.01f; 

		if (sfk::isKeyPressed(sfk::LShift)) shift_key_state = -1;        
		if (sfk::isKeyPressed(sfk::Q)) pos.x += 0.01f*shift_key_state;
		if (sfk::isKeyPressed(sfk::A)) pos.y += 0.01f*shift_key_state;
		if (sfk::isKeyPressed(sfk::Z)) pos.z += 0.01f*shift_key_state;
		if (sfk::isKeyPressed(sfk::W)) scale.x += 0.01f*shift_key_state;
		if (sfk::isKeyPressed(sfk::S)) scale.y += 0.01f*shift_key_state;
		if (sfk::isKeyPressed(sfk::X)) scale.z += 0.01f*shift_key_state;
		if (sfk::isKeyPressed(sfk::E)) rot.x += 0.5f*shift_key_state;
		if (sfk::isKeyPressed(sfk::D)) rot.y += 0.5f*shift_key_state;
		if (sfk::isKeyPressed(sfk::C)) rot.z += 0.5f*shift_key_state;
		*/
		shift_key_state = 1;

		if (sfk::isKeyPressed(sfk::LBracket)) { fov -= 1.0f; reshapeScreen(window.getSize()); }
		if (sfk::isKeyPressed(sfk::RBracket)) { fov += 1.0f; reshapeScreen(window.getSize()); }

		drawScene();
		window.display();
	}
	return 0;
}
