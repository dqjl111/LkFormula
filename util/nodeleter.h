/*************************************************
 Copyright (C), 1988-2016, Xiamen Kehua Hengsheng Co.,Ltd.
 Version:     1.0
 Date:        20160325
 Description: This class is use to assign a non pointer value to shared_ptr
 Author:      linzhipeng
 Modifer:
*************************************************/

#ifndef NODELETER
#define NODELETER
#include <memory>

class NoDeleter
{
public:
  void operator() (const void *) {}
};

/// Helper function to construct shared pointer with NoDeleter with cleaner syntax

template <typename T>
std::shared_ptr<T> noDeletePtr(T& r)
{
  return std::shared_ptr<T>(&r, NoDeleter());
}
#endif // NODELETER

