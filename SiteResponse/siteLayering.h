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





#include <vector>
#include "soillayer.h"

#ifndef SITELAYERING_H
#define SITELAYERING_H

class SiteLayering {

public:
	SiteLayering();
	SiteLayering(const char*);
	~SiteLayering();

	double getTotThickness() { return sl_totalThickness; };
	double getEquivalentShearVel() { return sl_equivalentVS; };
	double getEquivalentCompVel() { return sl_equivalentVP; };
	int    getNumLayers() { return sl_numLayers; };

	void addNewLayer(SoilLayer newLayer);
	SoilLayer getLayer(int index);
	double getNaturalPeriod();
	int readFromFile(const char*);
	
private:
	std::vector<SoilLayer> sl_layers;
	double sl_natPeriod;
	double sl_totalThickness;
	double sl_equivalentVS;
	double sl_equivalentVP;
	int    sl_numLayers;
};

#endif
