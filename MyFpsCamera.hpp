#ifndef __MYFPSCAMERA_H__
#define __MYFPSCAMERA_H__

#include "Transform.h"

class MyFpsCamera
{
private:

  gk::Point _position;
  gk::Vector _front;
  gk::Vector _up;

  gk::Transform _worldToViewTransform;
  gk::Transform _projectionTransform;

public:

  MyFpsCamera()
  {
  }
  MyFpsCamera(const gk::Point& position,
	      const gk::Vector& front,
	      const gk::Vector& up,
	      const gk::Transform& projection)
    :_position(position),
     _front(front),
     _up(up),

     _projectionTransform(projection)
  {
    updateWorldToViewTransform();
  }

  const gk::Transform& worldToViewTransform() const
  {
    return _worldToViewTransform;
  }
  const gk::Transform& projectionTransform() const
  {
    return _projectionTransform;
  }

  void localTranslate(const gk::Vector& translation)
  {
    _position += _worldToViewTransform.inverse()(translation);

    updateWorldToViewTransform();
  }

  void yaw(float angle)
  {
    _front = gk::RotateY(angle)(_front);
    _up = gk::RotateY(angle)(_up);

    updateWorldToViewTransform();
  }
  void pitch(float angle)
  {
    gk::Vector right = gk::Cross(_front, _up) * -1;

    _front = gk::Rotate(angle, right)(_front);
    _up = gk::Rotate(angle, right)(_up);

    updateWorldToViewTransform();
  }

private:

  void updateWorldToViewTransform()
  {
    gk::Vector right = gk::Cross(_front, _up);

    _worldToViewTransform = gk::Transform(gk::Matrix4x4(
							right.x, _up.x, -_front.x, _position.x,
							right.y, _up.y, -_front.y, _position.y,
							right.z, _up.z, -_front.z, _position.z,
							0, 0, 0, 1)
					  ).inverse();
  }

};

#endif
