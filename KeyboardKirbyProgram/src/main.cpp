#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <SFML/Graphics.hpp>

#include "Application.h"

int main()
{

	Application app = { sf::VideoMode(1280, 720), "Windmill Visual" };

	app.Run();

	return 0;
}