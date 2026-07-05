#pragma once

#ifdef USE_CONCEPTS
#include "serialize_concepts.hpp"
#else
#include "serialize_sfinae.hpp"
#endif