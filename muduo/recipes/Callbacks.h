//
// Created by cleon on 22-10-17.
//

#ifndef ANDROMEDA_CALLBACKS_H
#define ANDROMEDA_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "muduo/base/Timestamp.h"

namespace muduo {
namespace recipes {

using TimerCallback = boost::function<void()>;

}
}

#endif //ANDROMEDA_CALLBACKS_H
