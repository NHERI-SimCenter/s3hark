/* ********************************************************************* **
**                 Site Response Analysis Tool                           **
**   -----------------------------------------------------------------   **
**                                                                       **
**   Developed by: Charles Wang (c_w@berkeley.edu)                       **
**                 University of California, Berkeley                    **
**      		   Alborz Ghofrani (alborzgh@uw.edu)                     **
**                 University of Washington                              **
**                                                                       **
**   Date:   October  2018                                               **
**   Update: December 2018                                               **
** ********************************************************************* */



#ifndef EFFECTIVEFEMODEL_H
#define EFFECTIVEFEMODEL_H

#include <functional>
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
    SiteResponseModel(std::string, OutcropMotion*,std::function<void(double)>);
    SiteResponseModel(std::string , OutcropMotion *);
	~SiteResponseModel();

	int   buildEffectiveStressModel2D(bool doAnalysis);
    int   buildEffectiveStressModel3D(bool doAnalysis);
	int   runEffectiveStressModel2D();
    int   runEffectiveStressModel3D();
    void  setOutputDir(std::string outDir) { theOutputDir = outDir; }
    void setConfigFile(std::string configFile) { theConfigFile = configFile; }
    void  setTclOutputDir(std::string outDir) { theTclOutputDir = outDir; }
    void  setAnalysisDir(std::string anaDir) { theAnalysisDir = anaDir; }
    int subStepAnalyze(double dT, int subStep, DirectIntegrationAnalysis* theTransientAnalysis);


    std::function<void(double)> m_callbackFunction;
    //double pi = 3.1415926535897;
    const long double pi = 3.141592653589793238462643383279L;

    bool callback = false;
    void setCallback(bool cal) {callback = cal;}


private:
	Domain *theDomain;
	SiteLayering    SRM_layering;
	OutcropMotion*  theMotionX;
	OutcropMotion*  theMotionZ;
	std::string     theOutputDir;
	std::string 	theModelType;
    std::string 	theConfigFile;
    std::string     theTclOutputDir;
    std::string     theAnalysisDir;


};


#endif
