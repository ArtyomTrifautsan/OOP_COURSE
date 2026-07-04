#pragma once

#ifdef USE_CONCEPTS
#include "filter_iterator_concepts.hpp"
#else
#include "filter_iterator_sfinae.hpp"
#endif