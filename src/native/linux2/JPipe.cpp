/*
 * JPipe.cpp
 *
 *  Created on: Aug 25, 2014
 *      Author: jacob
 */


#include <string>
#include <iostream>

#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>

#include "jni.h"

#define READ   ( 1 << 0 )
#define WRITE  ( 1 << 1 )
#define CREATE ( 1 << 2 )

#define OPT_SET( mode, opt ) ( ( ( mode ) & ( opt ) ) == opt )
