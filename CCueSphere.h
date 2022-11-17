#ifndef _CCUESPHERE_
#define _CCUESPHERE_
#include "CSphere.h"

class CCueSphere : public CSphere
{
private:
	BallType firstHitBallType;
public:
	CCueSphere(const char* ballImageFileName);
	BallType getFirstHitBallType() const noexcept; // 처음 충돌한 공의 종류 반환
	void setFirstHitBallType(BallType ballType) noexcept; // 처음 충돌한 공의 종류 설정
	void hitBy(CSphere& ball) noexcept; // 충돌 후, 만약 처음 충돌한 공이라면 그 공의 종류를 저장
};
#endif