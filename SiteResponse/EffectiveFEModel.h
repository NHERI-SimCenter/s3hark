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

#include "AnalysisModel.h"
#include "CTestNormDispIncr.h"
#include "StaticAnalysis.h"
#include "DirectIntegrationAnalysis.h"
#include "EquiSolnAlgo.h"
#include "StaticIntegrator.h"
#include "TransientIntegrator.h"
#include "ConstraintHandler.h"
#include "RCM.h"
#include "DOF_Numberer.h"
#include "BandGenLinSolver.h"
#include <PlainHandler.h>
//#include <SparseGenColLinSolver.h>
/*
#include <SuperLU.h>
#include <SparseGenColLinSOE.h>
#include <KrylovNewton.h>
#include <KrylovAccelerator.h>
#include <AcceleratedNewton.h>
*/



#include "LinearSOE.h"
#include "NodeIter.h"
#include "ElementIter.h"
#include "DataFileStream.h"
#include "Recorder.h"
#include "UniaxialMaterial.h"
#include "ElementStateParameter.h"

#define MAX_FREQUENCY 50.0
#define NODES_PER_WAVELENGTH 10

class SiteResponseModel {

public:
	SiteResponseModel();
	SiteResponseModel(SiteLayering, std::string, OutcropMotion*, OutcropMotion*);
	SiteResponseModel(SiteLayering, std::string, OutcropMotion*);
    SiteResponseModel(std::string, OutcropMotion*,std::function<bool(double)>);
    SiteResponseModel(std::string, OutcropMotion*, OutcropMotion *, std::function<bool(double)>);
    SiteResponseModel(std::string , OutcropMotion *);
    SiteResponseModel(std::string , OutcropMotion *, OutcropMotion *);
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


    std::function<bool(double)> m_callbackFunction;
    //double pi = 3.1415926535897;
    const long double pi = 3.141592653589793238462643383279L;

    bool callback = false;
    void setCallback(bool cal) {callback = cal;}
    void setForward(bool f) {forward = f;}
    int trueRun();


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
    bool forward = true;
    bool m_doAnalysis = false;

    // 2D solver
    std::vector<double> dt;
    AnalysisModel *theModel;
    CTestNormDispIncr *theTest;
    EquiSolnAlgo *theSolnAlgo;
    TransientIntegrator* theIntegrator;// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
    ConstraintHandler* theHandler;          // 1. constraints Penalty 1.0e15 1.0e15
    RCM *theRCM;
    DOF_Numberer *theNumberer;                                 // 4. numberer RCM (another option: Plain)
    BandGenLinSolver *theSolver;                            // 5. system BandGeneral (TODO: switch to SparseGeneral)
    LinearSOE *theSOE;
    DirectIntegrationAnalysis* theAnalysis;         // static    2D
    DirectIntegrationAnalysis* theTransientAnalysis;// dynamic   2D
    TransientIntegrator* theTransientIntegrator;
    double m_dT;
    int m_nSteps;
    int m_remStep;


};


#endif
