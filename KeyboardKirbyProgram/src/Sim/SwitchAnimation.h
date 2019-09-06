#pragma once

#include <SFML/Graphics.hpp>

class SwitchAnimation
{
private:
	sf::Vector2f position_;
	float duration_;
	float thickness_;
	float initial_radius_;
	float speed_;
	float current_time_;

	sf::CircleShape anim_shape_;

public:
	SwitchAnimation(sf::Vector2f position, float duration, float thickness, float initial_radius, float speed);

	void UpdateAnim(float dt);
	void Draw(sf::RenderWindow& window, float circle_radius);
	bool isFinished() const;
};

