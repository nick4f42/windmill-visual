#pragma once

#include <string>
#include <stdexcept>

#include <SFML/Graphics.hpp>

class GUI
{
public:

  GUI(const char* text, unsigned text_size);

  void LoadFont(const char* filepath);

  void Draw(sf::RenderWindow& window, sf::View& gui_view, bool shown);

private:

  sf::Font font_;

  sf::Text text_;

  sf::RectangleShape background_;
  sf::RectangleShape hoverbox_shape_;

};

