/* ********************************************************************* **
**                 Site Response Analysis Tool                           **
**   -----------------------------------------------------------------   **
**                                                                       **
**   Developed by: Alborz Ghofrani (alborzgh@uw.edu)                     **
**                 University of Washington                              **
**                                                                       **
**   Date: October 2018                                                  **
**                                                                       **
** ********************************************************************* */





#ifndef FEMODEL_H
#define FEMODEL_H

#include "Domain.h"
#include "Matrix.h"

#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"

#include "DirectIntegrationAnalysis.h"

#define MAX_FREQUENCY 50.0
#define NODES_PER_WAVELENGTH 10

class SiteResponseModel {

public:
	SiteResponseModel();
	SiteResponseModel(SiteLayering, std::string, OutcropMotion*, OutcropMotion*);
	SiteResponseModel(SiteLayering, std::string, OutcropMotion*);
	~SiteResponseModel();

	int   runTestModel();
	int   runTotalStressModel();
	int   runEffectiveStressModel();
	void  setOutputDir(std::string outDir) { theOutputDir = outDir; };
	int subStepAnalyze(double dT, int subStep, int success, int remStep, DirectIntegrationAnalysis* theTransientAnalysis);

private:
	Domain *theDomain;
	SiteLayering    SRM_layering;
	OutcropMotion*  theMotionX;
	OutcropMotion*  theMotionZ;
	std::string     theOutputDir;
	std::string 	theModelType;
};


#endif