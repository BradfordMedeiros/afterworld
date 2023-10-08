#ifndef MOD_AFTERWORLD_COMPONENTS_ALERT
#define MOD_AFTERWORLD_COMPONENTS_ALERT

#include "../components/common.h"

extern Component alertComponent;

enum AlertMessageType { ALERT_DETAIL };
void pushAlertMessage(std::string message, AlertMessageType type);

#endif