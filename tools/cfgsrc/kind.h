#ifndef _KIND_H_
#define _KIND_H_

#include <stdint.h>
#include <unordered_map>
#include <string>



enum cost_type
{
	EN_CT_UNKNOWN = 0,
	EN_CT_MONEY = 10001,
	EN_CT_DIAMOND = 10101,
};
struct role_upgrade_cfg
{
	uint32_t Id;
	uint32_t Level;
	uint32_t CostType;
	int32_t CostValue;
	int32_t ScoreAdd;
};
struct ObjectLabel
{
	uint32_t L1;
	uint32_t L2;
	uint32_t L3;
	uint32_t L4;
	uint32_t L5;
};
struct Relation
{
	uint32_t item1;
	uint32_t item2;
	uint32_t item3;
};
struct objects_cfg
{
	uint32_t Id;
	uint32_t Type;
	std::string Name;
	std::string Desc;
	uint32_t Attr;
	std::unordered_map<uint32_t, uint32_t> LevelExp;
	ObjectLabel Labels;
	uint32_t gold;
	uint32_t exp;
	Relation relation;
	uint32_t usetime;
};

#endif /* _KIND_H_ */
