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




#include <fstream>
#include <iostream>
#include "EffectiveFEModel.h"
#include "siteLayering.h"
#include "soillayer.h"
#include "outcropMotion.h"

#include "StandardStream.h"
#include "FileStream.h"
#include "OPS_Stream.h"


StandardStream sserr;
FileStream ferr("log");
OPS_Stream *opserrPtr = &ferr;
OPS_Stream *opsoutPtr = &sserr;

/*
SiteLayering setupDummyLayers()
{
	SiteLayering res;
	res.addNewLayer(SoilLayer("Clay1", 2.0,  98.78, 184.79, 2.05, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay2", 6.0, 100.76, 188.50, 1.97, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay3", 5.0,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay4", 2.4,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Clay5", 2.6,  98.53, 184.34, 2.06, 200.0, 1.0, 1.0));
	res.addNewLayer(SoilLayer("Rock", 0.0, 1000.0, 3000.0, 2.5, 0.0, 0.0, 0.0));

	return res;
}
*/

int main(int argc, char** argv)
{

	if (argc < 3)
	{
		opserr << ">>> SiteResponseTool: Not enough arguments. <<<" << endln;
		std::getchar();
		return -1;
	}

	std::string configureFile = argv[1];
	std::string anaDir = argv[2];
	std::string outDir = argv[3];
	SiteResponseModel *model;

	//./siteresponse ../test/siteLayering.loc -bbp ../test/9130326.nwhp.vel.bbp out thisLog
    // read the layering file
    std::string layersFN("/Users/simcenter/Codes/SimCenter/SiteResponseTool/test/siteLayering.loc");
    std::string bbpOName(".");
    SiteLayering siteLayers(layersFN.c_str());
    int t = siteLayers.getNumLayers();


    // read the motion
    OutcropMotion motionX;
    //OutcropMotion motionZ;

    int inputStyle = 2; // bbp=1 opensees=2

    if (inputStyle == 1)
    {
        std::string bbpFName("/Users/simcenter/Codes/SimCenter/SiteResponseTool/test/9130326.nwhp.vel.bbp");
        bbpOName = std::string("out");
        std::string bbpLName = std::string("thisLog");
        //ferr.setFile(bbpLName.c_str(), APPEND);
        // read bbp style motion
        motionX.setBBPMotion(bbpFName.c_str(), 1);
        //motionZ.setBBPMotion(bbpFName.c_str(), 2);

        model = new SiteResponseModel(siteLayers, "2D", &motionX);
        model->setOutputDir(bbpOName);
    }
    else {
        std::string bbpLName = "Log";
        //std::string motionXFN("/Users/simcenter/Codes/SimCenter/SiteResponseTool/test/RSN766_G02_000_VEL");
        std::string motionXFN(anaDir+"/Rock");//TODO: may not work on windows
        motionX.setMotion(motionXFN.c_str());
        bbpOName = "out";
        model = new SiteResponseModel("2D", &motionX);
        model->setOutputDir(bbpOName);
        model->setAnalysisDir(anaDir);
        model->setTclOutputDir(outDir);
        model->setConfigFile(configureFile);

        bool runAnalysis = true;
    	model->buildEffectiveStressModel2D(runAnalysis);



    }



}
