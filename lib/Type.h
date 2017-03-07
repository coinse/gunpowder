#ifndef __TYPE_H__
#define __TYPE_H__

#include <map>

typedef std::map<int, std::pair<int, bool>> ControlDependency;

typedef std::tuple<std::string, std::vector<std::string>> Decl;

#endif //__TYPE_H__
