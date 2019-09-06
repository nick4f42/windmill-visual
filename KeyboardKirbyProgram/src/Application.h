#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Sim/Windmill.h"

class Application
{
private:
	static const float kZoomSpeed;

private:
	sf::RenderWindow render_window_;
	sf::View world_view_;
	sf::View gui_view_;
	
	unsigned starting_height_;

	sf::Vector2f last_click_position_;
	bool mouse_dragging_;

	sf::SoundBuffer click_sound_buffer_;

	Windmill windmill_;

  sf::Texture msg_texture_;
  sf::RectangleShape msg_shape_;

  sf::RectangleShape hoverbox_shape_;

  bool msg_shown_;

	sf::Clock clock_;
	float dt_;

public:
	Application(sf::VideoMode video_mode, const char* title);

	void Run();

private:

	void UpdateViews();

	void PollEvents();
	void Update();
	void Render();

};
