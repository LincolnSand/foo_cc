#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif


#include <execinfo.h>
#include <signal.h>

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <cxxabi.h>

void my_terminate();

