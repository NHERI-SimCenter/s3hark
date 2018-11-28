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





#ifndef SOILLAYER_H
#define SOILLAYER_H

#include <string>

class SoilLayer {
public:
	SoilLayer();
	~SoilLayer();

	SoilLayer(std::string name, double thickness, double vs, double vp, double rho, double Su, double hG, double m);


	std::string getName() { return sl_name;  };
	double      getThickness() { return sl_thickness; };
	double      getShearVelocity() { return sl_vs; };
	double      getCompVelocity() { return sl_vp; };
	double      getRho() { return sl_rho; };
	double      getSu() { return sl_su; };
	double      getMat_h() { return sl_hG; };
	double      getMat_m() { return sl_m; };

	double      getMatShearModulus();
	double      getMatBulkModulus();
	double      getNaturalPeriod();

private:
	std::string sl_name;
	double sl_thickness;
	double sl_vs;
	double sl_vp;
	double sl_rho;
	double sl_su;
	double sl_hG;
	double sl_m;
};

#endif