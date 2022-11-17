#ifndef _CCUESPHERE_
#define _CCUESPHERE_
#include "CSphere.h"

class CCueSphere : public CSphere
{
private:
	BallType firstHitBallType;
public:
	CCueSphere(const char* ballImageFileName);
	BallType getFirstHitBallType() const noexcept; // ó�� �浹�� ���� ���� ��ȯ
	void setFirstHitBallType(BallType ballType) noexcept; // ó�� �浹�� ���� ���� ����
	void hitBy(CSphere& ball) noexcept; // �浹 ��, ���� ó�� �浹�� ���̶�� �� ���� ������ ����
};
#endif