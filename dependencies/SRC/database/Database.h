/***
 * Demonstrike Core
 */

#pragma once

//! Other libs we depend on.
#include "../SharedDependencyDefines.h"
#include "../consolelog/consolelog.h"
#include "../threading/Threading.h"
#include "../network/Network.h"

#include <mysql.h>

//! Our own includes.
#include "Field.h"
#include "DatabaseCallback.h"
#include "DirectDatabase.h"
#include "DatabaseEngine.h"
