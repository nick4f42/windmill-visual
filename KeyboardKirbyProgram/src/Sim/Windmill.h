#pragma once

#define _USE_MATH_DEFINES

#include <vector>
#include <array>
#include <math.h>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "SwitchAnimation.h"

struct Point
{
	sf::Vector2f position;
	bool on_clockwise_side = false;
	bool on_clockwise_side_before = false;

	bool operator==(Point& other)
	{
		return position == other.position;
	}
};

class Windmill
{
private:

	static const double default_angular_speed_;

	std::vector<Point> points_;
	Point current_pivot_;
	bool pivot_set_;
	Point previous_pivot_;
	double rad_since_pivot_;

	double current_rad_;
	double rads_per_second_;

	float pt_proportion_size_;

	sf::CircleShape pt_shape_;
	sf::CircleShape pt_pivot_shape_;

  sf::RectangleShape line_shape_;

	sf::Sound click_sound_;

	std::vector<SwitchAnimation> animations_;

	bool paused_;
	bool started_;


  void UpdateLine(float dt, float length);

  bool CheckPointSide(Point& pt);

  void UpdatePoints();

  void UpdatePointSize(sf::RenderWindow& window, sf::View& world_view);

  bool CheckPointSwitches();

  bool SwitchPivot(Point& pt);

  void AnimateSwitches(sf::RenderWindow& window, float circle_radius);

public:

	Windmill(const sf::SoundBuffer & sound_buffer);

	void Start();

	void TogglePause();

	void Restart();

	void Update(float dt, float length);

	void Draw(sf::RenderWindow & window, sf::View & world_view);

	void AddPoint(sf::Vector2f pos);

	bool ChoosePivot(sf::Vector2f click_pos);

	void TryDelete(sf::Vector2f click_pos);

	void MultiplyAngularSpeed(double m_speed);

	bool isPivotSet();

	sf::Vector2f getPivotPosition();

};

