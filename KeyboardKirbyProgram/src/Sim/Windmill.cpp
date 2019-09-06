#include "Windmill.h"

const double Windmill::default_angular_speed_ = 0.6;

Windmill::Windmill(const sf::SoundBuffer& sound_buffer)
	: points_()
	, pivot_set_(false)
  , rad_since_pivot_(0.0)
	, current_rad_(0.0)
	, rads_per_second_(default_angular_speed_)
	, pt_proportion_size_(0.005f)
	, started_(false)
	, click_sound_(sound_buffer)
	, paused_(false)
{
	pt_shape_.setFillColor(sf::Color::Transparent);
	pt_shape_.setOutlineColor(sf::Color::White);

	pt_pivot_shape_.setFillColor(sf::Color::Yellow);

	line_vertex_array_[0].color = sf::Color::Red;
	line_vertex_array_[1].color = sf::Color::Red;

}


void Windmill::Start()
{
	if (!pivot_set_)
	{
		current_pivot_ = *(points_.end() - 1);
		pivot_set_ = true;
	}

	started_ = true;

	UpdatePoints();
	for (auto point : points_)
	{
		point.on_clockwise_side_before = point.on_clockwise_side;
	}
	previous_pivot_ = current_pivot_;


}


void Windmill::TogglePause()
{
	paused_ = !paused_;
}


void Windmill::Restart()
{
	points_.clear();
	animations_.clear();
	started_ = false;
	pivot_set_ = false;
	paused_ = false;
	rads_per_second_ = default_angular_speed_;
	current_rad_ = 0;
}


void Windmill::UpdateLine(float dt, float length)
{
	current_rad_ += rads_per_second_ * dt;
	rad_since_pivot_ += rads_per_second_ * dt;

	if (current_rad_ >= 2 * M_PI)
		current_rad_ -= 2 * M_PI;

	line_vertex_array_[0].position.x = current_pivot_.position.x + length * (float)std::cos(current_rad_);
	line_vertex_array_[0].position.y = current_pivot_.position.y + length * (float)std::sin(current_rad_);

	line_vertex_array_[1].position.x = current_pivot_.position.x - length * (float)std::cos(current_rad_);
	line_vertex_array_[1].position.y = current_pivot_.position.y - length * (float)std::sin(current_rad_);

	

}


void Windmill::Update(float dt, float length)
{
	if (paused_ || !started_ || !pivot_set_)
		return;

	for (auto& anim : animations_)
		anim.UpdateAnim(dt);

	UpdateLine(dt, length);
	UpdatePoints();
	if (CheckPointSwitches())
	{
		click_sound_.play();
		animations_.push_back(SwitchAnimation(current_pivot_.position,
											  0.6f, // duration
											  0.25f, // thickness
											  1.1f, // initial radius 
											  5.0f // speed
											  ));
	}
	auto test_finished = std::remove_if(animations_.begin(), animations_.end(), [](const SwitchAnimation& anim)
	{
		return anim.isFinished();
	});
	animations_.erase(test_finished, animations_.end());
}


void Windmill::UpdatePointSize(sf::RenderWindow & window, sf::View & world_view)
{
	pt_shape_.setRadius(pt_proportion_size_ * world_view.getSize().y);
	pt_shape_.setOutlineThickness(pt_shape_.getRadius() * 0.3f);
	pt_shape_.setOrigin(pt_shape_.getRadius(), pt_shape_.getRadius());

	pt_pivot_shape_.setRadius(1.5f * pt_proportion_size_ * world_view.getSize().y);
	pt_pivot_shape_.setOrigin(pt_pivot_shape_.getRadius(), pt_pivot_shape_.getRadius());
}


void Windmill::Draw(sf::RenderWindow & window, sf::View& world_view)
{
	if (started_)
		window.draw(line_vertex_array_, 2, sf::Lines);

	UpdatePointSize(window, world_view);

	for (auto& pt : points_)
	{
		if (pivot_set_ && pt == current_pivot_)
		{
			pt_pivot_shape_.setPosition(pt.position);
			window.draw(pt_pivot_shape_);
		}
		else
		{
			pt_shape_.setPosition(pt.position);
			window.draw(pt_shape_);
		}
		
	}
	
	if (started_)
		AnimateSwitches(window, pt_pivot_shape_.getRadius());
}


void Windmill::AddPoint(sf::Vector2f pos)
{
	points_.push_back({ pos });
	if (started_ && pivot_set_)
	{
		points_.back().on_clockwise_side = points_.back().on_clockwise_side_before = CheckPointSide(points_.back());
	}
}


bool Windmill::ChoosePivot(sf::Vector2f click_pos)
{
	for (auto& pt : points_)
	{
		if (std::sqrt(std::pow(pt.position.x - click_pos.x, 2.0f) + std::pow(pt.position.y - click_pos.y, 2))
			< pt_pivot_shape_.getRadius() * 1.5f)
		{
			current_pivot_ = pt;
			pivot_set_ = true;
			UpdatePoints();
			return true;
		}
	}

	return false;
}


void Windmill::TryDelete(sf::Vector2f click_pos)
{
	for (auto it = points_.begin(); it != points_.end(); it++)
	{
		if (std::sqrt(std::pow(it->position.x - click_pos.x, 2.0f) + std::pow(it->position.y - click_pos.y, 2))
			< pt_shape_.getRadius() * 1.5f)
		{
			if (pivot_set_ && *it == current_pivot_)
				pivot_set_ = started_ = false;

			points_.erase(it);
			return;
		}
	}
}


void Windmill::ChangeAngularSpeed(double m_speed)
{
	rads_per_second_ *= m_speed;

	if (rads_per_second_ < 0.001)
		rads_per_second_ = 0.001;
	else if (rads_per_second_ > 2)
		rads_per_second_ = 2;
}


bool Windmill::isPivotSet()
{
	return pivot_set_;
}


sf::Vector2f Windmill::getPivotPosition()
{
	return current_pivot_.position;
}


bool Windmill::CheckPointSide(Point& pt)
{
	float dy = (pt.position.y - current_pivot_.position.y);
	float dx = (pt.position.x - current_pivot_.position.x);

	double angle;

	if (dx < 0)
	{
		angle = std::atan(dy / dx) + M_PI;
	}
	else if (dx == 0)
	{
		angle = dy >= 0 ? M_PI_2 : 3 * M_PI_2;
	}
	else
	{
		angle = std::atan(dy / dx);

		if (angle < 0)
			angle += M_PI * 2;
	}

	bool clockwise_condition;

	if (angle < M_PI)
		clockwise_condition = current_rad_ > angle && current_rad_ < angle + M_PI;
	else
		clockwise_condition = current_rad_ > angle || current_rad_ < angle - M_PI;

	if (clockwise_condition)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void Windmill::UpdatePoints()
{
	for (auto& pt : points_)
	{
		if (pt == current_pivot_)
			continue;

		pt.on_clockwise_side_before = pt.on_clockwise_side;

		pt.on_clockwise_side = CheckPointSide(pt);
		
	}
	
}


bool Windmill::CheckPointSwitches()
{
	for (auto& pt : points_)
	{
		if (pt == current_pivot_)
			continue;

		if (pt.on_clockwise_side != pt.on_clockwise_side_before)
		{
			if (SwitchPivot(pt))
				return true;
		}
	}

	return false;
}


bool Windmill::SwitchPivot(Point& pt)
{
	if (pt == previous_pivot_ && rad_since_pivot_ < 0.3f)
		return false;

	previous_pivot_ = current_pivot_;
	current_pivot_ = pt;
	rad_since_pivot_ = 0;
	
	return true;
}


void Windmill::AnimateSwitches(sf::RenderWindow& window, float circle_radius)
{
	for (auto& anim : animations_)
	{
		anim.Draw(window, circle_radius);
	}
	
}