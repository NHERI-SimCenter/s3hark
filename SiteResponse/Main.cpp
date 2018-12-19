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

	// read the layering file
	std::string layersFN(argv[1]);
	std::string bbpOName(".");
	SiteLayering siteLayers(layersFN.c_str());

	// read the motion
	OutcropMotion motionX;
	OutcropMotion motionZ;

	if (strcmp(argv[2], "-bbp") == 0)
	{
		std::string bbpFName(argv[3]);
		bbpOName = std::string(argv[4]);
		std::string bbpLName = std::string(argv[5]);
		ferr.setFile(bbpLName.c_str(), APPEND);
		// read bbp style motion
		motionX.setBBPMotion(bbpFName.c_str(), 1);
		motionZ.setBBPMotion(bbpFName.c_str(), 2);
	}
	else {
		// read OpenSees style motion
		std::string bbpLName = std::string(argv[4]);
		std::string motionXFN(argv[2]);
		motionX.setMotion(motionXFN.c_str());

		bbpOName = std::string(argv[3]);
		

		/*
		if (argc > 3)
		{
			std::string motionZFN(argv[3]);
			motionZ.setMotion(motionXFN.c_str());
		}
		*/
	}

	/*
	ofstream *s = new ofstream;
  	s->open("model.tcl", std::ofstream::out);
  	ofstream &tclFile = *s;
	//int runEffectiveStressModel(ofstream &s)
	s->close();
	*/


	//SiteResponseModel model(siteLayers, "3D", &motionX, &motionZ);
	SiteResponseModel model(siteLayers, "2D", &motionX);
	model.setOutputDir(bbpOName);
	//model.runTotalStressModel();
	//model.runEffectiveStressModel();

	model.buildEffectiveStressModel2D();
	//model.runEffectiveStressModel2D();


	
	return 0;
}
