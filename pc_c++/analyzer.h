#pragma once

#include "configurator.h"

class analysisWorker {
private:
	int led_segments = 20;
	int res_x;
	int res_y;
	int y;

public:
	void setCommonConfigs(instanceConfCommon &commonConf);
	void doAnalysis();
};
