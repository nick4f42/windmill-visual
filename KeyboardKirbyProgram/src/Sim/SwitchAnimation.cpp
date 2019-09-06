#include "SwitchAnimation.h"


SwitchAnimation::SwitchAnimation(sf::Vector2f position, float duration, float thickness, float initial_radius, float speed)
	: position_(position)
	, duration_(duration)
	, thickness_(thickness)
	, initial_radius_(initial_radius)
	, speed_(speed)
	, current_time_(0.0f)
{
	anim_shape_.setFillColor(sf::Color::Transparent);
	anim_shape_.setPosition(position_);
}


void SwitchAnimation::UpdateAnim(float dt)
{
	current_time_ += dt;
}


void SwitchAnimation::Draw(sf::RenderWindow & window, float circle_radius)
{
	anim_shape_.setRadius(circle_radius * (initial_radius_ + current_time_ * speed_));
	anim_shape_.setOutlineColor(sf::Color::Yellow * sf::Color(255, 255, 255, (int)(255 * (1 - current_time_ / duration_))));
	anim_shape_.setOutlineThickness(anim_shape_.getRadius() * thickness_);
	anim_shape_.setOrigin(anim_shape_.getRadius(), anim_shape_.getRadius());

	window.draw(anim_shape_);
}


bool SwitchAnimation::isFinished() const
{
	return (current_time_ > duration_);
}
