#ifndef CHECK_MATS_H_
#define CHECK_MATS_H_

#include "types.h"
#include "err.h"

#ifdef _WIN32
#define CHECK_MATS_API __declspec(dllexport)
#else
#define CHECK_MATS_API
#endif

#if defined(__cplusplus)
extern "C"
{
#endif
	CHECK_MATS_API int CheckMats(const Point& pt,  callback_fun fun, void* param, int rate);
#if defined(__cplusplus)
}
#endif 

#endif