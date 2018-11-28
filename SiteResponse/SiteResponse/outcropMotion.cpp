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





#include "outcropMotion.h"
#include <string>
#include <fstream>
#include <sstream>

#include "Vector.h"

bool fileExists(const char* fileName)
{
	// open the file to see if it exists
	std::ifstream file(fileName);
	if (file)
		return true;

	return false;
}

int readDT(const char* fileName, int& numSteps, std::vector<double>& dt)
{
	int res = -1;
	numSteps = 0;
	std::ifstream file(fileName);
	if (file)
	{
		std::string line;
		res = 1;
		double t_n, t_n1;

		// find the first non-empty line
		while (getline(file, line))
		{
			// skip comment lines
			if ((line == "") || (line[0] == '%') || (line[0] == '#'))
				continue;
			std::istringstream lines(line);
			lines >> t_n;
			break;
		}

		// count number of lines and calculate the dt for each step
		while (getline(file, line))
		{
			// skip comment lines
			if ((line == "") || (line[0] == '%') || (line[0] == '#'))
				continue;
			std::istringstream lines(line);
			lines >> t_n1;
			dt.push_back(t_n1 - t_n);
			t_n = t_n1;
			++numSteps;
		}
	}
	return res;
}

OutcropMotion::OutcropMotion() :
	theGroundMotion(NULL),
	theAccSeries(),
	theVelSeries(),
	theDispSeries(),
	isThisInitialized(false),
	m_numSteps(0)
{

}

OutcropMotion::OutcropMotion(const char* fName):
	theGroundMotion(NULL),
	theAccSeries(),
	theVelSeries(),
	theDispSeries(),
	isThisInitialized(false),
	m_numSteps(0)
{
	this->setMotion(fName);
}

OutcropMotion::~OutcropMotion() 
{

}

void
OutcropMotion::setMotion(const char* fName)
{
	isThisInitialized = true;

	// assuming time, displacement, velocity and acceleration are located in different files
	std::string motionName(fName);
	std::string timeFName = motionName + ".time";
	std::string accFName = motionName + ".acc";
	std::string velFName = motionName + ".vel";
	std::string dispFName = motionName + ".disp";

	// check to see if the time file exists
	if (readDT(timeFName.c_str(), m_numSteps, m_dt) > 0)
	{
		// assuming acceleration is in g's
		if (fileExists(accFName.c_str()))
			theAccSeries = new PathTimeSeries(1, accFName.c_str(), timeFName.c_str(), 9.81, true);

		// assuming velocity is in m/s
		if (fileExists(velFName.c_str()))
			theVelSeries = new PathTimeSeries(2, velFName.c_str(), timeFName.c_str(), 1.0, true);

		// assuming displcement is in m
		if (fileExists(dispFName.c_str()))
			theDispSeries = new PathTimeSeries(3, dispFName.c_str(), timeFName.c_str(), 1.0, true);

		// create a ground motion. It's useful for UniformExcitatpon or MultipleSupport 
		if ((theAccSeries != NULL) || (theVelSeries != NULL) || (theDispSeries != NULL))
			theGroundMotion = new GroundMotion(theDispSeries, theVelSeries, theAccSeries, NULL);
		else
		{
			// only time file exists. This is a problem
			isThisInitialized = false;
			opserr << "None of the files " << accFName.c_str() << " or " << velFName.c_str() << " or " << dispFName.c_str() << " exist." << endln;
		}
	}
	else {
		// the time file does not exist. This is a problem
		isThisInitialized = false;
		opserr << "The file " << timeFName.c_str() << " containing the array of time does not exist." << endln;
	}
}
void                
OutcropMotion::setBBPMotion(const char* fName, int colNum)
{
	Vector Path(100000);
	Vector Time(100000);
	m_numSteps = 0;
	std::ifstream file(fName);
	if (file)
	{
		std::string line;
		double t_n, t_n1;
		double dummy;

		// find the first non-empty line
		while (getline(file, line))
		{
			// skip comment lines
			if ((line == "") || (line[0] == '%') || (line[0] == '#'))
				continue;
			std::istringstream lines(line);
			lines >> t_n;
			for (int ii = 0; ii < colNum - 1; ii++)
				lines >> dummy;
			lines >> Path(0);
			break;
		}

		// count number of lines and calculate the dt for each step
		while (getline(file, line))
		{
			// skip comment lines
			if ((line == "") || (line[0] == '%') || (line[0] == '#'))
				continue;
			std::istringstream lines(line);
			lines >> t_n1;
			m_dt.push_back(t_n1 - t_n);
			t_n = t_n1;
			++m_numSteps;
			for (int ii = 0; ii < colNum - 1; ii++)
				lines >> dummy;
			lines >> dummy;
			Path(m_numSteps) = dummy / 100.0;
			Time(m_numSteps) = t_n1;
		}
		Path.resize(m_numSteps+1);
		Time.resize(m_numSteps+1);
		theVelSeries = new PathTimeSeries(2, Path, Time, 1.0, false);
		isThisInitialized = true;
		
		// create a ground motion. It's useful for UniformExcitatpon or MultipleSupport 
		if ((theAccSeries != NULL) || (theVelSeries != NULL) || (theDispSeries != NULL))
			theGroundMotion = new GroundMotion(theDispSeries, theVelSeries, theAccSeries, NULL);
		else
		{
			isThisInitialized = false;
			opserr << "File " << fName << " seems to be empty." << endln;
		}
	}
	else {
		// the time file does not exist. This is a problem
		isThisInitialized = false;
		opserr << "File " << fName << " does not exist." << endln;
	}
	return;
}
