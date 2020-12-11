#ifndef PCH_H
#define PCH_H

#include <thread>
#include <iostream>

#define GL_SILENCE_DEPRECATION

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>  //new
#include "materials.h" //new

/*
Plik 'glext.h' znajduje siê: https://www.khronos.org/registry/OpenGL/api/GL/glext.h
wymaga on pliku 'khrplatform.h' który znajduje siê tu: https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h
*/

#endif //PCH_H
