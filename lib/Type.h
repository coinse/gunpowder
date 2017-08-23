/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include <map>

typedef std::map<int, std::pair<int, bool>> ControlDependency;

typedef std::tuple<std::string, std::vector<std::string>> Decl;

#endif //__TYPE_H__
