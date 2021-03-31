#ifndef _BASE_H_
#define _BASE_H_

#include <stdint.h>
#include <unordered_map>
#include <string>

namespace base
{
	enum ItemType
	{
		TYPE_UNKNOWN = 0,
		TYPE_A = 1,
		TYPE_B = 2,
		TYPE_C = 3,
	};
	struct Base
	{
		uint32_t id;
		std::string name;
	};
}

#endif /* _BASE_H_ */
