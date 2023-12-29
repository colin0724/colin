#include "mats.h"
#include "check_mats.h"
#include <memory>
#include <iostream>

Mats::Mats(std::string& ip, Func fun, void* param) :
	m_mats(-1),
	m_ip(ip),
	m_fun(fun),
	m_param(param)
{
	Point pt = { 0, 0 };
	CheckMats(pt, ChangeMatsFun, this, 1);
}

Mats::~Mats() 
{
}

int Mats::ChangeMatsFun(int ret, void* param)
{
	if (!param) {
		return -1;
	}

	Mats* pthis = (Mats*)param;
	return pthis->ChangeMats(ret);
	return 0;
}

int Mats::ChangeMats(int ret)
{
	if (m_mats == ret) {
		return 0;
	}

	m_mats = ret;
	m_fun(m_ip, m_mats, m_param);
	return 0;
}