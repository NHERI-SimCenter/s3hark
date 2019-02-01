#ifndef SITERESPONSE_H
#define SITERESPONSE_H


#include "EffectiveFEModel.h"
#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"


class SiteResponse {

public:
	SiteResponse();
	~SiteResponse();

    void run();



	
private:
    SiteResponseModel *model;



};

#endif
