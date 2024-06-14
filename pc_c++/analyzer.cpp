#include <tuple>
#include "analyzer.h"
#include "configurator.h"

// When analyzing led colors from screenshot, its necessary to know resolution etc. -> store this information to worker
void analysisWorker::setCommonConfigs(instanceConfCommon& commonConf)
{
	this->res_x = commonConf.res_x;
	this->res_y = commonConf.res_y;
	if (commonConf.centerThird)
	{
		this->y = this->res_y / 3;
	}
	else {
		this->y = this->res_y;
	}
}
