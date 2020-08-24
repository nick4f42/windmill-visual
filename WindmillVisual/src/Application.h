#pragma once

#include <stdexcept>
#include <string>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Sim/Windmill.h"
#include "GUI.h"

class Application
{
private:

	static const float kZoomSpeed;

	sf::RenderWindow render_window_;
	sf::View world_view_;
	sf::View gui_view_;
	
	unsigned starting_height_;

	sf::Vector2f last_click_position_;
	bool mouse_dragging_;

	sf::SoundBuffer click_sound_buffer_;

	Windmill windmill_;

  GUI gui_;

  bool msg_shown_;

	sf::Clock clock_;
	float dt_;

  void UpdateViews();

  void PollEvents();
  inline void Update();
  void Render();

public:

	Application(sf::VideoMode video_mode, const char* title);

	void Run();

};
