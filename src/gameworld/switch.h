#ifndef MOD_AFTERWORLD_SWITCH
#define MOD_AFTERWORLD_SWITCH

#include <unordered_map>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"

struct ManagedSwitch {
	bool switchOn;
	std::optional<std::string> onSignal;
	std::optional<std::string> offSignal;
};

struct Switches {
	std::unordered_map<objid, ManagedSwitch> switches;
};

void addSwitch(Switches& switches, objid id, std::optional<std::string> onSignal, std::optional<std::string> offSignal);
void removeSwitch(Switches& switches, objid id);
void handleSwitch(Switches& switches, std::string switchValue);

void onAddConditionId(objid id, std::string& value);

#endif 