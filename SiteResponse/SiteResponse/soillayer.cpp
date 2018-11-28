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





#include "soillayer.h"

SoilLayer::SoilLayer() :
	sl_name("DefaultLayer"),
	sl_thickness(0.0),
	sl_vs(0.0), sl_vp(0.0),
	sl_rho(0.0), sl_hG(1.0), sl_m(1.0)
{

}


SoilLayer::SoilLayer(std::string name, double thickness, double vs, double vp, double rho, double Su, double hG, double m) :
	sl_name(name),
	sl_thickness(thickness),
	sl_vs(vs), sl_vp(vp), sl_su(Su),
	sl_rho(rho), sl_hG(hG), sl_m(m)
{

}

SoilLayer::~SoilLayer()
{

}

double
SoilLayer::getMatShearModulus()
{
	return sl_rho * sl_vs * sl_vs;
}

double
SoilLayer::getMatBulkModulus()
{
	return sl_rho * (sl_vp * sl_vp - 4.0/3.0 * sl_vs * sl_vs);
}

double
SoilLayer::getNaturalPeriod()
{
	return 4.0 * sl_thickness / sl_vs;
}
