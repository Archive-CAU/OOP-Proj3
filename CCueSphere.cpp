#include "CCueSphere.h"

CCueSphere::CCueSphere(const char* ballImageFileName) : CSphere(ballImageFileName)
{
	this->firstHitBallType = BallType::NONE;
}

BallType CCueSphere::getFirstHitBallType() const noexcept
{
	return this->firstHitBallType;
}

void CCueSphere::setFirstHitBallType(BallType ballType) noexcept
{
	this->firstHitBallType = ballType;
}

void CCueSphere::hitBy(CSphere& ball) noexcept
{
	if (this->hasIntersected(ball))
	{
		if (this->firstHitBallType == BallType::NONE)
		{
			this->firstHitBallType = ball.getBallType();
		}

		__super::hitBy(ball);
	}
}