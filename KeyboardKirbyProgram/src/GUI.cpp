#include "GUI.h"


GUI::GUI(const char* text, unsigned text_size)
  : font_()
  , text_(text, font_, text_size)
  , background_({ 0.0f, 0.0f })
  , hoverbox_shape_(sf::Vector2f(35.0f, 35.0f))
{
  sf::Vector2f padding = sf::Vector2f(20.0f, 20.0f);

  LoadFont("res/monofonto.ttf");

  text_.setFillColor(sf::Color(220, 220, 220));
  text_.setPosition(padding);


  sf::Vector2f textSize(text_.getGlobalBounds().width,
                        text_.getGlobalBounds().height);

  background_.setSize(2.f*padding + textSize);

  hoverbox_shape_.setFillColor(sf::Color(255, 255, 255, 20));
  hoverbox_shape_.setOutlineColor(sf::Color(150, 150, 150));
  hoverbox_shape_.setOutlineThickness(1.0f);

  background_.setFillColor(sf::Color(255, 255, 255, 20));
  background_.setOutlineColor(sf::Color(150, 150, 150));
  background_.setOutlineThickness(1.0f);
}


void GUI::LoadFont(const char* filepath)
{
  if (!font_.loadFromFile(filepath))
  {
    std::string msg("Can not load file: ");
    msg += filepath;
    throw std::runtime_error(msg);
  }
}


void GUI::Draw(sf::RenderWindow& window, sf::View& gui_view, bool shown)
{
  if (shown)
  {
    window.draw(background_);
    window.draw(text_);
  }
  else
  {
    window.draw(hoverbox_shape_);
  }
}
