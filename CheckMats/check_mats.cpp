#include "check_mats.h"
#include <boost/thread.hpp>

void* g_param = 0;
callback_fun g_fun = 0;

void ChangeMats()
{
	int mats = 0;
	while (g_fun) {
		g_fun(mats++ % 5, g_param);
		boost::this_thread::sleep(boost::posix_time::millisec(500));
	}
}

int CheckMats(const Point& pt, callback_fun fun, void* param, int rate)
{
	if (!fun) {
		return -1;
	}

	g_fun = fun;
	g_param = param;

	boost::thread thread(&ChangeMats);
    return CHECK_MATS_RESULT_SUCCESS;
}
