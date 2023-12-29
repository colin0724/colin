#ifndef MATS_H
#define MATS_H
#include <string>
#include<boost/function.hpp>

//typedef boost::function<void(std::string&, int)> Func;
typedef void (*Func)(std::string&, int, void*);

class Mats {
public:
	Mats(std::string& ip, Func fun, void* param);
	~Mats();

private:
	int m_mats;
	std::string m_ip;
	Func m_fun;
	void* m_param;

	void Init();
	static int ChangeMatsFun(int ret, void* param);
	int ChangeMats(int ret);
};

#endif // MATS_H