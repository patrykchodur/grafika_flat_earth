#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>

#include "pch.h"

bool loadOBJ(
	const char * path, 
	std::vector<sf::Vector3f> & out_vertices, 
	std::vector<sf::Vector2f> & out_uvs, 
	std::vector<sf::Vector3f> & out_normals
);



bool loadAssImp(
	const char * path, 
	std::vector<unsigned short> & indices,
	std::vector<sf::Vector3f> & vertices,
	std::vector<sf::Vector2f> & uvs,
	std::vector<sf::Vector3f> & normals
);

#endif
