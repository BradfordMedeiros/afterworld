#include "./alert.h"

extern CustomApiBindings* gameapi;


const float bufferExpirationTimeMs = 5000;
const int maxBufferSize = 1;

struct AlertMessage {
	std::string message;
	std::optional<double> time;
	AlertMessageType type;
};
struct Alerts {
	std::deque<AlertMessage> messageBuffer;
};

std::string amountToDraw2(std::string& text, double createTime, float rate){
	auto currIndex = static_cast<int>((gameapi -> timeSeconds(true) - createTime) * rate);
	return text.substr(0, currIndex);
}

const int letterSize = 8;
const float letterSizeNdi = letterSize / 1000.f;
const float margin = letterSizeNdi * 3;
const float marginLeft = margin;
const float marginBottom = margin;

void renderAlerts2(Alerts& alerts, int yoffset, std::deque<AlertMessage>& buffer){
	for (int i = 0; i < buffer.size(); i++){
		AlertMessage& message = buffer.at(i);
		if (message.type != ALERT_DETAIL){
			continue;
		}
		if (!message.time.has_value()){
			message.time = gameapi -> timeSeconds(true);
		}

		auto textToDraw = amountToDraw2(message.message, message.time.value(), 100);
		gameapi -> drawText(
			textToDraw, 
			(-1 + marginLeft),
			(-1 + (letterSizeNdi * 0.5) + marginBottom), 
			letterSize, 
			false, 
			std::nullopt, 
			std::nullopt,
			true, 
			std::nullopt, 
			std::nullopt,
			std::nullopt,
			std::nullopt
		);
		break;
	}
}

bool isNotExpiredMessage2(AlertMessage& message){
	if (!message.time.has_value()){
		return true;
	}
	auto currTime = gameapi -> timeSeconds(true);
	auto createTime = message.time.value();
	auto diff = (currTime - createTime) * 1000;
	return diff < bufferExpirationTimeMs;
}

void filterExpiredMessages2(Alerts& alerts){
	std::deque<AlertMessage> newMessageBuffer;
	for (auto &message : alerts.messageBuffer){
		if (isNotExpiredMessage2(message)){
			newMessageBuffer.push_back(message);
		}
	}
	alerts.messageBuffer = newMessageBuffer;
}

Alerts alerts {
	.messageBuffer = {},
};

void pushAlertMessage(std::string message, AlertMessageType type){
   alerts.messageBuffer.push_back(AlertMessage {
   	.message = message,
   	.time = std::nullopt,
   	.type = type,
   });
   if (alerts.messageBuffer.size() > maxBufferSize){
   	alerts.messageBuffer.pop_front();
   }
}

void onAlertFrame(){
	renderAlerts2(alerts, 400, alerts.messageBuffer);
  filterExpiredMessages2(alerts); // probably shouldn't be done every frame
}

Component alertComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	onAlertFrame();
    return BoundingBox2D {
    	.x = 0,
    	.y = 0,
    	.width = 0.f,
    	.height = 0.f,
    };
  },
};
