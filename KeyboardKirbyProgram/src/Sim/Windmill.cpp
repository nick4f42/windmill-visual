#include "Windmill.h"


const double Windmill::default_angular_speed_ = 0.45;

unsigned Point::index_count = 0u;

float Point::arrowhead_proportion = 0.025f;

double Point::arrow_angle = 0.4;

sf::RectangleShape Point::shaft({ 1.f, 1.f });

sf::VertexArray Point::arrowhead(sf::Triangles, 3u);


Point::Point(sf::Vector2f position)
  : position(position)
  , index(index_count++)
{
  shaft.setOrigin({ 0.0f, 0.5f }); // sets origin to center
}

unsigned Point::getIndexCount()
{
  return index_count++;
}


Windmill::Windmill(const sf::SoundBuffer& sound_buffer)
	: points_()
  , current_pivot_()
  , prev_pivot_index_((unsigned)(-1))
	, pivot_set_(false)
  , rad_since_pivot_(0.0)
	, current_rad_(0.0)
	, rads_per_second_(default_angular_speed_)
	, pt_proportion_size_(0.005f)
  , line_shape_({ 1.f, 1.f })
	, click_sound_(sound_buffer)
	, paused_(false)
	, started_(false)
{
	pt_shape_.setFillColor(sf::Color::Transparent);
	pt_shape_.setOutlineColor(sf::Color::White);

	pt_pivot_shape_.setFillColor(sf::Color::Yellow);

  line_shape_.setOrigin({ 0.5f, 0.5f }); // sets origin to center
  line_shape_.setFillColor(sf::Color(255, 40, 10));
}


void Windmill::Start()
{
  animations_.clear();

	if (!pivot_set_)
	{
		current_pivot_ = *(points_.end() - 1);
		pivot_set_ = true;
	}

	started_ = true;

	UpdatePoints();
	for (auto point : points_)
	{
		point.prev_on_clockwise = point.on_clockwise;
	}
	prev_pivot_index_ = current_pivot_.index;
}


void Windmill::TogglePause()
{
	paused_ = !paused_;
}


void Windmill::Restart()
{
	points_.clear();
  vectors_.clear();
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

  line_shape_.setPosition(current_pivot_.position);
  line_shape_.setRotation((float)(current_rad_ * 180.0f / M_PI));
}


void Windmill::Update(float dt, float length)
{
	if (!started_ || !pivot_set_)
		return;
  
  if (paused_)
  {
    UpdateLine(0.0f, length);
    return;
  }

  UpdateLine(dt, length);

	for (auto& anim : animations_)
		anim.UpdateAnim(dt);
	
	UpdatePoints();
	if (CheckPointSwitches())
	{
		click_sound_.play();

		animations_.push_back(SwitchAnimation(current_pivot_.position,
											  0.6f, // duration
											  0.25f, // thickness
											  1.1f, // initial radius 
											  6.0f // speed
											  ));
	}
	auto test_finished = std::remove_if(animations_.begin(), 
                                      animations_.end(), 
                                      [](const SwitchAnimation& anim)
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
	UpdatePointSize(window, world_view);

  // Draws the arrow to each next point (if next point is stored)
  // ---

  DrawVectors(window, world_view);

  if (started_)
  {
    // sets line very long and 2 pixels thick
    auto diff = world_view.getCenter() - current_pivot_.position;
    auto dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    line_shape_.setScale(2 * (dist + world_view.getSize().x + world_view.getSize().y),
      2.0f * world_view.getSize().y / (float)window.getSize().y);

    window.draw(line_shape_);
  }

  // Draw the point circles
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
	
  // Draw the "pop" animations
	if (started_)
		AnimateSwitches(window, pt_pivot_shape_.getRadius());
}


void Windmill::AddPoint(sf::Vector2f pos)
{
	points_.push_back(Point(pos));
	if (started_ && pivot_set_)
	{
		points_.back().on_clockwise = points_.back().prev_on_clockwise = CheckPointSide(points_.back());
	}
}


bool Windmill::ChoosePivot(sf::Vector2f click_pos)
{
	for (auto& pt : points_)
	{
		if (sqrt(pow(pt.position.x - click_pos.x, 2.0f) + pow(pt.position.y - click_pos.y, 2))
			  < pt_pivot_shape_.getRadius() * 1.5f)
		{
			current_pivot_ = pt;
			pivot_set_ = true;
			UpdatePoints();

      vectors_.clear();

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

      vectors_.clear();

			return;
		}
	}
}


void Windmill::MultiplyAngularSpeed(double m_speed)
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

	if (angle < M_PI)
		return current_rad_ > angle && current_rad_ < angle + M_PI;
	else
		return current_rad_ > angle || current_rad_ < angle - M_PI;
}


void Windmill::UpdatePoints()
{
	for (auto& pt : points_)
	{
		if (pt == current_pivot_)
			continue;

		pt.prev_on_clockwise = pt.on_clockwise;

		pt.on_clockwise = CheckPointSide(pt);
	}
}


bool Windmill::CheckPointSwitches()
{
	for (auto& pt : points_)
	{
		if (pt == current_pivot_)
			continue;

		if (pt.on_clockwise != pt.prev_on_clockwise)
		{
			if (SwitchPivot(pt))
				return true;
		}
	}

	return false;
}


bool Windmill::SwitchPivot(Point& pt)
{
	if (pt.index == prev_pivot_index_ && rad_since_pivot_ < 0.3f)
		return false;

  bool in_vectors_ = false;
  for (auto& v : vectors_)
  {
    if (v[0] == current_pivot_.position && v[1] == pt.position)
    {
      in_vectors_ = true;
      break;
    }
  }
  if (!in_vectors_)
    vectors_.push_back(std::array<sf::Vector2f, 2>({ current_pivot_.position, pt.position }));

  prev_pivot_index_ = current_pivot_.index;
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


void Windmill::DrawVectors(sf::RenderWindow& window, sf::View& world_view)
{
  for (auto it = vectors_.begin(); it != vectors_.end(); it++)
  {
    sf::Vector2f tail = (*it)[0], tip = (*it)[1];

    float length = sqrt(powf(tail.x - tip.x, 2) +
      powf(tail.y - tip.y, 2));

    double angle;
    if (tip.x == tail.x)
      angle = tip.y - tail.y > 0 ? M_PI_2 : 3 * M_PI_2;
    else
    {
      angle = atan((tip.y - tail.y) / (tip.x - tail.x));

      if (tip.x - tail.x < 0)
        angle += M_PI;
    }

    Point::shaft.setPosition(tail);

    Point::shaft.setRotation((float)(angle * 180.0 / M_PI));

    Point::shaft.setScale(length,
      2.0f * world_view.getSize().y / (float)window.getSize().y);

    Point::arrowhead[0].position = tip;

    Point::arrowhead[1].position = tip - Point::arrowhead_proportion * world_view.getSize().y *
      sf::Vector2f((float)cos(angle + Point::arrow_angle), (float)sin(angle + Point::arrow_angle));
    Point::arrowhead[2].position = tip - Point::arrowhead_proportion * world_view.getSize().y *
      sf::Vector2f((float)cos(angle - Point::arrow_angle), (float)sin(angle - Point::arrow_angle));

    auto color = getVectorColor((it - vectors_.begin()));

    for (int i = 0; i < 3; i++)
    {
      Point::arrowhead[i].position -= (length / 2.0f - 1.5f * Point::arrowhead_proportion * 
        world_view.getSize().y) * sf::Vector2f((float)cos(angle), (float)sin(angle));
      Point::arrowhead[i].color = color;
    }
    Point::shaft.setFillColor(color);

    window.draw(Point::arrowhead);

    window.draw(Point::shaft);
  }
}


sf::Color Windmill::getVectorColor(unsigned i)
{
  size_t s = vectors_.size();
  float t = s != 1 ? (float)i / (s-1) : 0;
  return sf::Color((int)(30 * t), (int)(90 * t), (int)(90 * (1.0f - t)));
}