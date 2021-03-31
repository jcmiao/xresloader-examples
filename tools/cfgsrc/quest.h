#ifndef _QUEST_H_
#define _QUEST_H_

#include <stdint.h>
#include <unordered_map>
#include <string>

#include "base.h"


enum ItemType2
{
	TYPE_UNKNOWN = 0,
	TYPE_D = 4,
	TYPE_E = 5,
	TYPE_F = 6,
};
struct quests_cfg
{
	enum ItemType3
	{
		TYPE_UNKNOWN = 0,
		TYPE_G = 7,
		TYPE_H = 8,
		TYPE_I = 9,
	};
	struct SubSubMsg
	{
		uint32_t p1;
		uint64_t p2;
		ItemType2 it2;
	};
	struct SubMsg
	{
		uint32_t Param1;
		uint32_t Param2;
		SubSubMsg Param3;
	};
	uint32_t Id;
	uint32_t Type;
	std::string Name;
	std::string Desc;
	SubMsg Param;
	ItemType2 it2;
	ItemType3 it3;
	base::ItemType it1;
};
struct TestBase
{
	uint32_t value;
	base::Base bbbbb;
};

#endif /* _QUEST_H_ */
