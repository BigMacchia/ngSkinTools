#pragma once
#include <vector>

#include "maya.h"
#include "InflListerBase.h"

using namespace std;

/**
 * ngListInfluences command computation part
 */
class InflListerByWeights: public InflListerBase
{
public:
	virtual void execute();
};
