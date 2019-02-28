#ifndef SITERESPONSE_H
#define SITERESPONSE_H


#include "EffectiveFEModel.h"
#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"


class SiteResponse {

public:
    SiteResponse(std::string configureFile,std::string anaDir,std::string outDir);
	~SiteResponse();

    void run();
    void buildTcl();

	
private:
    SiteResponseModel *model;
    // read the motion
    OutcropMotion motionX;
    OutcropMotion motionZ;
    std::string m_configureFile;
    std::string m_analysisDir;
    std::string m_outputDir;



};

#endif
