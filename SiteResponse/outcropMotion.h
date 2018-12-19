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
#include "PathSeries.h"
#include "PathTimeSeries.h"
#include "GroundMotion.h"

#ifndef OUTCROPMOTION_H
#define OUTCROPMOTION_H

class OutcropMotion
{
public:
	OutcropMotion();
	OutcropMotion(const char* fName);
	~OutcropMotion();

	PathTimeSeries*  getDispSeries() { return theDispSeries; };
	PathTimeSeries*  getVelSeries() { return theVelSeries; };
	PathTimeSeries*  getAccSeries() { return theAccSeries; };
	GroundMotion*    getGroundMotion() { return theGroundMotion; };
	  
	bool                isInitialized() { return isThisInitialized; };
	std::vector<double> getDTvector() { return m_dt; };
	int                 getNumSteps() { return m_numSteps; };
	void                setMotion(const char* fName);
	void                setBBPMotion(const char* fName, int colNum);

private:
	PathTimeSeries* theAccSeries;
	PathTimeSeries* theVelSeries;
	PathTimeSeries* theDispSeries;
	GroundMotion*  theGroundMotion;

	bool isThisInitialized;
	int  m_numSteps;
	std::vector<double> m_dt;
};


#endif