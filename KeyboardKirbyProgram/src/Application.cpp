#include "Application.h"


const float Application::kZoomSpeed = 0.1f;


Application::Application(sf::VideoMode video_mode, const char* title)
	: render_window_(video_mode, title)
	, world_view_()
	, starting_height_(video_mode.height)
	, mouse_dragging_(false)
	, windmill_(click_sound_buffer_)
  , msg_shape_({ 400, 300 })
  , hoverbox_shape_({ 30, 30 })
  , msg_shown_(false)
	, clock_()
	, dt_(0.f)
{
	UpdateViews();

	if (!click_sound_buffer_.loadFromFile("res/click.wav"))
		exit(-1);
  if (!msg_texture_.loadFromFile("res/message_text.png"))
    exit(-1);

  msg_shape_.setTexture(&msg_texture_);

  hoverbox_shape_.setFillColor(sf::Color(255, 255, 255, 32));
  hoverbox_shape_.setOutlineColor(sf::Color(150, 150, 150));
  hoverbox_shape_.setOutlineThickness(1.0f);

  msg_shape_.setOutlineColor(sf::Color(220, 220, 200));
  msg_shape_.setOutlineThickness(1.5f);

	render_window_.setFramerateLimit(120u);
}


void Application::Run()
{

	while (render_window_.isOpen())
	{
		dt_ = clock_.getElapsedTime().asSeconds();
		clock_.restart();

		PollEvents();

		Update();

		Render();
	}
}


inline void Application::UpdateViews()
{
  // Set sizes to be correct aspect ratio
	world_view_.setSize(
      (float)(starting_height_ * render_window_.getSize().x / render_window_.getSize().y),
			(float)(starting_height_));
	gui_view_.setSize(
      (float)(starting_height_ * render_window_.getSize().x / render_window_.getSize().y),
      (float)(starting_height_));
  // Sets gui center so it fits in top left corner of screen
  gui_view_.setCenter(gui_view_.getSize() / 2.0f);
}


void Application::PollEvents()
{
	sf::Event e;
	while (render_window_.pollEvent(e))
	{
		if (e.type == sf::Event::Closed)
		{
			render_window_.close();
		}
		else if (e.type == sf::Event::Resized)
		{
			UpdateViews();
		}
		else if (e.type == sf::Event::MouseWheelScrolled)
		{
			float zoom_amount = kZoomSpeed * e.mouseWheelScroll.delta;

      if (world_view_.getSize().y < 0.1 && zoom_amount > 0 ||
          world_view_.getSize().y > 10000 && zoom_amount < 0)
        continue;

      // Moves view so view zooms "into" mouse position
			sf::Vector2f view_center = world_view_.getCenter();
			sf::Vector2f mouse_position = render_window_.mapPixelToCoords(
        sf::Mouse::getPosition(render_window_), world_view_);

			world_view_.move(
          zoom_amount * (mouse_position.x - view_center.x),
				  zoom_amount * (mouse_position.y - view_center.y));

			world_view_.zoom(1.0f - zoom_amount);
		}
		else if (e.type == sf::Event::MouseButtonPressed)
		{
			if (e.mouseButton.button == sf::Mouse::Button::Left)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
				{
					windmill_.AddPoint(render_window_.mapPixelToCoords(sf::Vector2i(e.mouseButton.x, e.mouseButton.y), world_view_));
				}
				else
				{
					mouse_dragging_ = true;
					last_click_position_ = render_window_.mapPixelToCoords(sf::Mouse::getPosition(render_window_), world_view_);
				}
			}
			else if (e.mouseButton.button == sf::Mouse::Button::Right)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
				{
					windmill_.TryDelete(render_window_.mapPixelToCoords(sf::Mouse::getPosition(render_window_), world_view_));
				}
				else
				{
					windmill_.ChoosePivot(render_window_.mapPixelToCoords(sf::Mouse::getPosition(render_window_), world_view_));
				}
			}
		}
		else if (e.type == sf::Event::MouseButtonReleased)
		{
			if (e.mouseButton.button == sf::Mouse::Button::Left)
			{
				mouse_dragging_ = false;
			}
		}
		else if (e.type == sf::Event::MouseMoved)
		{
			if (mouse_dragging_)
			{
				world_view_.setCenter(world_view_.getCenter() - 
				(render_window_.mapPixelToCoords({ e.mouseMove.x, e.mouseMove.y}, world_view_) - last_click_position_));
			}
      else
      {
        auto mp = render_window_.mapPixelToCoords(
            sf::Mouse::getPosition(render_window_), 
            gui_view_);

        msg_shown_ = mp.x < 50 && mp.x >= 0 && mp.y < 50 && mp.y >= 0;
      }
		}
		else if (e.type == sf::Event::KeyPressed)
		{
			if (e.key.code == sf::Keyboard::Enter)
			{
				windmill_.Start();
			}
			else if (e.key.code == sf::Keyboard::P)
			{
				windmill_.TogglePause();
			}
			else if (e.key.code == sf::Keyboard::R)
			{
				windmill_.Restart();
			}
			else if (e.key.code == sf::Keyboard::V)
			{
				if (windmill_.isPivotSet())
					world_view_.setCenter(windmill_.getPivotPosition());
			}
			else if (e.key.code == sf::Keyboard::Left)
			{
				windmill_.MultiplyAngularSpeed(0.9);
			}
			else if (e.key.code == sf::Keyboard::Right)
			{
				windmill_.MultiplyAngularSpeed(1.1);
			}
		}
	}
}


inline void Application::Update()
{
	windmill_.Update(dt_, world_view_.getSize().x * 20.0f);
}


void Application::Render()
{
	render_window_.clear(sf::Color::Black);

  // World's View
	render_window_.setView(world_view_);
	windmill_.Draw(render_window_, world_view_);

  // Gui's View
  render_window_.setView(gui_view_);

  if (msg_shown_)
    render_window_.draw(msg_shape_);
  else
    render_window_.draw(hoverbox_shape_);

	render_window_.display();
}
