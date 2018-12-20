/* ********************************************************************* **
**                 Site Response Analysis Tool                           **
**   -----------------------------------------------------------------   **
**                                                                       **
**   Developed by: Charles Wang (c_w@berkeley.edu)                       **
**                 University of California, Berkeley                    **
**      		   Alborz Ghofrani (alborzgh@uw.edu)                     **
**                 University of Washington                              **
**                     [TODO: add more]                                  **
**   Date:   October  2018   Alborz Ghofrani                             **
**   Update: December 2018   Charles Wang                                **
** ********************************************************************* */

#include <vector>
#include <iostream>
#include <sstream>

#include "EffectiveFEModel.h"

#include "Vector.h"
#include "Matrix.h"

#include "Node.h"
#include "Element.h"
#include "NDMaterial.h"
#include "SP_Constraint.h"
#include "MP_Constraint.h"
#include "LinearSeries.h"
#include "PathSeries.h"
#include "PathTimeSeries.h"
#include "LoadPattern.h"
#include "NodalLoad.h"
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

#include "LinearSOE.h"
#include "NodeIter.h"
#include "ElementIter.h"
#include "DataFileStream.h"
#include "Recorder.h"
#include "UniaxialMaterial.h"
#include "ElementStateParameter.h"

#include "SSPbrick.h"
#include "SSPquad.h"
#include "SSPquadUP.h"
#include "Brick.h"
#include "J2CyclicBoundingSurface.h"
#include "ElasticIsotropicMaterial.h"
#include "PM4Sand.h"
#include "ElasticMaterial.h"
#include "NewtonRaphson.h"
#include "LoadControl.h"
#include "Newmark.h"
#include "PenaltyConstraintHandler.h"
#include "TransformationConstraintHandler.h"
#include "BandGenLinLapackSolver.h"
#include "BandGenLinSOE.h"
#include "GroundMotion.h"
#include "ImposedMotionSP.h"
#include "TimeSeriesIntegrator.h"
#include "MultiSupportPattern.h"
#include "UniformExcitation.h"
#include "VariableTimeStepDirectIntegrationAnalysis.h"
#include "NodeRecorder.h"
#include "ElementRecorder.h"
#include "ViscousMaterial.h"
#include "ZeroLength.h"
#include "SingleDomParamIter.h"

#include "Information.h"
#include <vector> 
#include <map>

#include <fstream>
#include <string>
#include <iomanip>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <exception>
#include <cmath>




#define PRINTDEBUG true

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

SiteResponseModel::SiteResponseModel() : theModelType("2D"),
										 theMotionX(0),
										 theMotionZ(0),
										 theOutputDir(".")
{
}

SiteResponseModel::SiteResponseModel(SiteLayering layering, std::string modelType, OutcropMotion *motionX, OutcropMotion *motionY) : SRM_layering(layering),
																																	 theModelType(modelType),
																																	 theMotionX(motionX),
																																	 theMotionZ(motionY),
																																	 theOutputDir(".")
{
	if (theMotionX->isInitialized() || theMotionZ->isInitialized())
		theDomain = new Domain();
	else
	{
		opserr << "No motion is specified." << endln;
		exit(-1);
	}
}

SiteResponseModel::SiteResponseModel(SiteLayering layering, std::string modelType, OutcropMotion *motionX) : SRM_layering(layering),
																											 theModelType(modelType),
																											 theMotionX(motionX),
																											 theOutputDir(".")
{
	if (theMotionX->isInitialized())
		theDomain = new Domain();
	else
	{
		opserr << "No motion is specified." << endln;
		exit(-1);
	}
}

SiteResponseModel::~SiteResponseModel()
{
	if (theDomain != NULL)
		delete theDomain;
	theDomain = NULL;
}


int SiteResponseModel::buildEffectiveStressModel2D()
{
	//Vector zeroVec(3);
	//zeroVec.Zero();

	// ------------------------------------------
	// 0. Define some limits
	// ------------------------------------------
	double minESizeH = 0.001;
    double minESizeV = 0.001;
    double colThickness = 1.0;// thickness of 2D ele
	double g = -9.81;



	// ------------------------------------------
	// 0. Load configurations form json file
	// ------------------------------------------
	//std::string configFile = "/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/SRT.json";
	std::string configFile = "/Users/simcenter/Codes/SimCenter/build-SiteResponseTool-Desktop_Qt_5_11_1_clang_64bit-Debug/SiteResponseTool.app/Contents/MacOS/SRT.json";
    std::ifstream i(configFile);
    if(!i)
        return false;// failed to open SRT.json TODO: print to log
    json SRT;
    i >> SRT;

	// set outputs for tcl 
	ofstream s ("/Users/simcenter/Codes/SimCenter/build-SiteResponseTool-Desktop_Qt_5_11_1_clang_64bit-Debug/SiteResponseTool.app/Contents/MacOS/model.tcl", std::ofstream::out);
	s << "# #########################################################" << "\n\n";
	s << "wipe \n\n";


	// basic settings
	int numLayers = 0;
	int numNodes = 0;
	int numElems = 0;
	double totalHeight = 0.0;
	double sElemX = 0.0;
    json basicSettings;
    double dampingCoeff,dashpotCoeff,groundWaterTable,rockDen,rockVs;
    std::string groundMotion;
    try
    {
        basicSettings = SRT["basicSettings"];
        dampingCoeff = basicSettings["dampingCoeff"];
        dashpotCoeff = basicSettings["dashpotCoeff"];
        groundMotion = basicSettings["groundMotion"];
        groundWaterTable = basicSettings["groundWaterTable"];
        rockDen = basicSettings["rockDen"];
        rockVs = basicSettings["rockVs"];
		sElemX = basicSettings["eSizeH"];
        if (sElemX<minESizeH)
        {
            std::string err = "eSizeH is tool small. change it in the json file.";throw err;
        }
    }
    catch (std::exception& e){std::cerr << "Standard exception: " << e.what() << std::endl;return false;}
    catch(std::string str){std::cerr << str << std::endl;return false;}


	std::vector<int> layerNumElems;
	std::vector<int> layerNumNodes;
	std::vector<double> layerElemSize;
	std::vector<int> dryNodes;


	s << "# ------------------------------------------ \n";
	s << "# 1. Build nodes and elements                \n";
	s << "# ------------------------------------------ \n \n";
	double yCoord = 0;
	Node *theNode;
	NDMaterial *theMat;

	Element *theEle;
	Parameter *theParameter;
	char **paramArgs = new char *[2];
	paramArgs[0] = new char[15];
	paramArgs[1] = new char[5];
	sprintf(paramArgs[0], "materialState");
	std::map<int, int> matNumDict;
	std::vector<int> soilMatTags;

	theNode = new Node(numNodes + 1, 3, 0.0, yCoord); theDomain->addNode(theNode);
	theNode = new Node(numNodes + 2, 3, sElemX, yCoord); theDomain->addNode(theNode);
	s << "model BasicBuilder -ndm 2 -ndf 3  \n\n";
	s << "node " << numNodes + 1 << " 0.0 " << yCoord << endln;
	s << "node " << numNodes + 2 << " " << sElemX << " " << yCoord << endln;
	numNodes += 2;			
	json soilProfile,soilLayers,mats;
	try
    {
		mats = SRT["materials"];
        soilProfile = SRT["soilProfile"];
        soilLayers = soilProfile["soilLayers"];
        std::sort(soilLayers.begin(),soilLayers.end(),
                  [](const json &a, const json &b) { return a["id"] > b["id"]; });
		//std::sort(mats.begin(),mats.end(),
        //         [](const json &a, const json &b) { return a["id"] > b["id"]; });
		for (auto l:soilLayers)
        {
			double thickness = l["thickness"];
			totalHeight += thickness;
		}
        for (auto l:soilLayers)
        {
            int lTag = l["id"];
			int matTag = l["material"];
            double eSizeV = l["eSize"];
            if (eSizeV<minESizeV) {std::string err = "eSize is tool small. change it in the json file.";throw err;}
            double thickness = l["thickness"];
            double vs = l["vs"];
            double Dr = l["Dr"];
			double vPerm = l["vPerm"];
			double hPerm = l["hPerm"];
			double uBulk = l["uBulk"];
            std::string color = l["color"];
			std::string lname = l["name"];

			double evoid = 0.0;
			double rho_d = 0.0;
			double rho_s = 0.0;
			double Gs = 2.67;

			soilMatTags.push_back(matTag);

			
			json mat = mats[matTag-1];
			std::cout << "mat id:" << mat["id"] << " mat type" << mat["type"] << std::endl;
			std::string matType = mat["type"];
			int trueMatId = mat["id"];
			if(!matType.compare("Elastic"))
			{
				double E = mat["E"];
				double density = mat["density"];
				double poisson = mat["poisson"];
				theMat = new ElasticIsotropicMaterial(matTag, E , poisson, density);
				s << "nDMaterial ElasticIsotropic " << matTag << " "<< E <<" " << " "<<poisson<<" "<<density<<endln;
				double emax = 0.8;
				double emin = 0.5;
				
				evoid  = emax - Dr * (emax - emin);
				rho_d = Gs / (1 + evoid);
				rho_s = rho_d *(1.0+evoid/Gs);

			}else if(!matType.compare("PM4Sand"))
			{
				double thisDr = mat["Dr"];
				double G0 = mat["G0"];
				double hpo = mat["hpo"];
				double thisDen = mat["Den"];

				double P_atm = mat["P_atm"];
				double h0 = mat["h0"];
				double emax = mat["emax"];
				double emin = mat["emin"];
				double nb = mat["nb"];
				double nd = mat["nd"];
				double Ado = mat["Ado"];
				double z_max = mat["z_max"];
				double cz = mat["cz"];
				double ce = mat["ce"];
				double phic = mat["phic"];
				double nu = mat["nu"];
				double cgd = mat["cgd"];
				double cdr = mat["cdr"];
				double ckaf = mat["ckaf"];
				double Q = mat["Q"];
				double R = mat["R"];
				double m = mat["m"];
				double Fsed_min = mat["Fsed_min"];
				double p_sedo = mat["p_sedo"];

				evoid  = emax - thisDr * (emax - emin);
				rho_d = Gs / (1 + evoid);
				rho_s = rho_d *(1.0+evoid/Gs);

				//theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
				theMat = new PM4Sand(matTag, thisDr,G0,hpo,thisDen,P_atm,h0,emax,emin,nb,nd,Ado,z_max,cz,ce,phic,nu,cgd,cdr,ckaf,Q,R,m,Fsed_min,p_sedo);
				s << "nDMaterial PM4Sand " << matTag<< " " << thisDr<< " " <<G0<< " " <<hpo<< " " <<thisDen<< " " <<P_atm<< " " <<h0<< " "<<emax<< " "<<emin<< " " <<
				nb<< " " <<nd<< " " <<Ado<< " " <<z_max<< " " <<cz<< " " <<ce<< " " <<phic<< " " <<nu<< " " <<cgd<< " " <<cdr<< " " <<ckaf<< " " <<
				Q<< " " <<R<< " " <<m<< " " <<Fsed_min<< " " <<p_sedo << endln;
			}
			OPS_addNDMaterial(theMat);
			if (PRINTDEBUG) opserr << "Material " << matType.c_str() << ", tag = " << matTag << endln;






            int numEleThisLayer = static_cast<int> (std::round(thickness / eSizeV));
            numEleThisLayer = std::max(1,numEleThisLayer);
            double t = thickness / numEleThisLayer;
			s << "# " << lname << ": thickness = "<< thickness << ", "<< numEleThisLayer<< " elements." << endln;
            for (int i=1; i<=numEleThisLayer;i++)
            {
                yCoord += t ;
				theNode = new Node(numNodes + 1, 3, 0.0, yCoord);
				theDomain->addNode(theNode);
				theNode = new Node(numNodes + 2, 3, sElemX, yCoord);
				theDomain->addNode(theNode);

				s << "node " << numNodes + 1 << " 0.0 " << yCoord << endln;
				s << "node " << numNodes + 2 << " " << sElemX << " " << yCoord << endln;

				theEle = new SSPquadUP(numElems + 1, numNodes - 1, numNodes, numNodes + 2, numNodes + 1,
									   *theMat, 1.0, uBulk, 1.0, 1.0, 1.0, evoid, 0.0, 0.0, g * 1.0); // -9.81 * theMat->getRho() TODO: theMat->getRho()
				
				s << "element SSPquadUP "<<numElems + 1<<" " 
					<<numNodes - 1 <<" "<<numNodes<<" "<< numNodes + 2<<" "<< numNodes + 1<<" "
					<< theMat->getTag() << " " << "1.0 "<<uBulk<<" 1.0 1.0 1.0 " <<evoid << " 0.0 0.0 "<< g * 1.0 << endln;
			

				theDomain->addElement(theEle);

				theParameter = new Parameter(numElems + 1, 0, 0, 0);
				sprintf(paramArgs[1], "%d", theMat->getTag());
				theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
				theDomain->addParameter(theParameter);

				matNumDict[numElems + 1] = theMat->getTag();



                if (yCoord >= (totalHeight - groundWaterTable))
				{ 	//record dry nodes above ground water table
					dryNodes.push_back(numNodes + 1);
					dryNodes.push_back(numNodes + 2);
				}
                numNodes += 2;
				numElems += 1;
            }
            std::cout << "layer tag: " << lTag << std::endl;
        }
    }
    catch (std::exception& e){std::cerr << "Standard exception: " << e.what() << std::endl;return false;}
    catch(std::string str){std::cerr << str << std::endl;return false;}
	s << "\n\n";


	s << "# ------------------------------------------ \n";
	s << "# 2. Apply boundary conditions.              \n";
	s << "# ------------------------------------------ \n \n";

	s << "# 2.1 Apply fixities at base              \n\n";
	SP_Constraint *theSP;
	int sizeTheSPtoRemove = 2 ; // for 3D it's 8;
	ID theSPtoRemove(sizeTheSPtoRemove); // these fixities should be removed later on if compliant base is used

	theSP = new SP_Constraint(1, 0, 0.0, true);
	theDomain->addSP_Constraint(theSP);
	theSPtoRemove(0) = theSP->getTag();
	theSP = new SP_Constraint(1, 1, 0.0, true);
	theDomain->addSP_Constraint(theSP);

	s << "fix 1 1 1 0" << endln;

	theSP = new SP_Constraint(2, 0, 0.0, true);
	theDomain->addSP_Constraint(theSP);
	theSPtoRemove(1) = theSP->getTag();
	theSP = new SP_Constraint(2, 1, 0.0, true);
	theDomain->addSP_Constraint(theSP);

	s << "fix 2 1 1 0" << endln << endln;



	s << "# 2.2 Apply periodic boundary conditions    \n\n";
	MP_Constraint *theMP;
	int crrdim = 2 ;//For 3D it's 3;
	Matrix Ccr(crrdim, crrdim);
	ID rcDOF(crrdim);
	// TODO: get clarified about the dimensions of Crr and rfDOF
	Ccr(0, 0) = 1.0;
	Ccr(1, 1) = 1.0; 
	rcDOF(0) = 0;
	rcDOF(1) = 1; 
	for (int nodeCount = 2; nodeCount < numNodes; nodeCount += 2)
	{
		theMP = new MP_Constraint(nodeCount + 1, nodeCount + 2, Ccr, rcDOF, rcDOF);
		theDomain->addMP_Constraint(theMP);
		s << "equalDOF " << nodeCount + 1 << " "<< nodeCount + 2 << " 1 2" << endln;
	}
	s << "\n\n";



	s << "# 2.3 Apply pore pressure boundaries for nodes above water table. \n\n";
	for (int i = 0; i < dryNodes.size(); i++)
	{
		theSP = new SP_Constraint(dryNodes[i], 2, 0.0, true);
		theDomain->addSP_Constraint(theSP);
		s << "fix " << dryNodes[i] << " 0 0 1" << endln;
	}
	s << "\n\n";



	s << "# ------------------------------------------ \n";
	s << "# 3. Gravity analysis.                       \n";
	s << "# ------------------------------------------ \n \n";

	// update material stage to consider elastic behavior
	// TODO: question: do it for any material type?
	ParameterIter &theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		theParameter->update(0.0);
	}
	s << endln;

	for (int i=0; i != soilMatTags.size(); i++)
		s << "updateMaterialStage -material "<< soilMatTags[i] <<" -stage 0" << endln << endln ; 



	// create the output streams
	OPS_Stream *theOutputStream;
	Recorder *theRecorder;

	// record last node's results
	ID nodesToRecord(1);
	nodesToRecord(0) = numNodes;

	int dimDofToRecord = 3;// For 3D it's 4
	ID dofToRecord(dimDofToRecord);
	dofToRecord(0) = 0;
	dofToRecord(1) = 1;
	dofToRecord(2) = 2;
	//dofToRecord(3) = 3;// 3D



	s << "# 3.1 elastic gravity analysis (transient) \n\n";

	double gamma = 5./6.;
	double beta = 5./9.;

	s << "constraints Transformation" << endln;
	s << "test NormDispIncr 1.0e-4 35 1" << endln;
	s << "algorithm   Newton" << endln;
	s << "numberer RCM" << endln;
	s << "system BandGeneral" << endln;
	s << "set gamma " << gamma << endln;
	s << "set beta " << beta << endln;
	s << "integrator  Newmark $gamma $beta" << endln;
	s << "analysis Transient" << endln << endln;
	
	s << "set startT  [clock seconds]" << endln;
	s << "analyze     10 1.0" << endln;
	s << "puts \"Finished with elastic gravity analysis...\"" << endln << endln;

	// create analysis objects - I use static analysis for gravity
	AnalysisModel *theModel = new AnalysisModel();
	CTestNormDispIncr *theTest = new CTestNormDispIncr(1.0e-4, 35, 1);                    // 2. test NormDispIncr 1.0e-7 30 1
	EquiSolnAlgo *theSolnAlgo = new NewtonRaphson(*theTest);                              // 3. algorithm   Newton (TODO: another option: KrylovNewton) 
	//StaticIntegrator *theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0); // *
	//ConstraintHandler *theHandler = new TransformationConstraintHandler(); // *
	TransientIntegrator* theIntegrator = new Newmark(5./6., 4./9.);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
	ConstraintHandler* theHandler = new PenaltyConstraintHandler(1.0e16, 1.0e16);          // 1. constraints Penalty 1.0e15 1.0e15
	RCM *theRCM = new RCM();
	DOF_Numberer *theNumberer = new DOF_Numberer(*theRCM);                                 // 4. numberer RCM (another option: Plain)
	BandGenLinSolver *theSolver = new BandGenLinLapackSolver();                            // 5. system BandGeneral (TODO: switch to SparseGeneral)
	LinearSOE *theSOE = new BandGenLinSOE(*theSolver);

	DirectIntegrationAnalysis* theAnalysis;												   // 7. analysis    Transient
	theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//StaticAnalysis *theAnalysis; // *
	//theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator); // *
	theAnalysis->setConvergenceTest(*theTest);

	int converged = theAnalysis->analyze(10,1.0); 
	if (!converged)
	{
		opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
	} else
	{
		opserr << "Didn't converge at time " << theDomain->getCurrentTime() << endln;
	}
	opserr << "Finished with elastic gravity analysis..." << endln << endln;



	s << "# 3.2 plastic gravity analysis (transient)" << endln << endln;

	// update material response to plastic
	theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		theParameter->update(1.0);
		// //updateMaterialStage -material $i -stage 1.0
		//s << "updateMaterialStage -material "<< theParameter->getTag() <<" -stage 1" << endln ; 
	}
	s << endln;

	for (int i=0; i != soilMatTags.size(); i++)
		s << "updateMaterialStage -material "<< soilMatTags[i] <<" -stage 1" << endln ; 

	// add parameters: FirstCall for plastic gravity analysis
	sprintf(paramArgs[0], "FirstCall");
	ElementIter &theElementIterFC = theDomain->getElements();
	int nParaPlus = 0;
	while ((theEle = theElementIterFC()) != 0)
	{
		int theEleTag = theEle->getTag();
		theParameter = new Parameter(numElems + nParaPlus + 1, 0, 0, 0);
		sprintf(paramArgs[1], "%d", matNumDict[theEleTag]);
		theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
		theDomain->addParameter(theParameter);
		nParaPlus += 1;

		//setParameter -value 0 -ele $elementTag FirstCall $matTag
		s << "setParameter -value 0 -ele "<<theEleTag<<" FirstCall "<< matNumDict[theEleTag] << endln;
	}

	// add parameters: poissonRatio for plastic gravity analysis
	sprintf(paramArgs[0], "poissonRatio");
	ElementIter &theElementIter = theDomain->getElements();
	while ((theEle = theElementIter()) != 0)
	{
		int theEleTag = theEle->getTag();
		theParameter = new Parameter(numElems + nParaPlus + 1, 0, 0, 0);
		sprintf(paramArgs[1], "%d", matNumDict[theEleTag]);
		theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
		theDomain->addParameter(theParameter);
		nParaPlus += 1;

		//setParameter -value 0 -ele $elementTag poissonRatio $matTag
		s << "setParameter -value 0.3 -ele "<< theEleTag <<" poissonRatio "<< matNumDict[theEleTag] << endln;
	}
	//TODO: the $i ?  in setParameter -value 0.3 -eleRange $layerBound($i) $layerBound([expr $i+1]) poissonRatio $i

	// update FirstCall and poissonRatio
	theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		int paraTag = theParameter->getTag();
		if (paraTag>numElems & paraTag<=(numElems+nParaPlus/2.))
		{// FirstCall
			theParameter->update(0.0);
		}else if (paraTag>(numElems+nParaPlus/2.)){// poissonRatio
			theParameter->update(0.3);
		}
	}
	s << endln;

	converged = theAnalysis->analyze(10,1.0); 
	s << "analyze     10 1.0" << endln;
	if (!converged)
	{
		opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
	} else
	{
		opserr << "Didn't converge at time " << theDomain->getCurrentTime() << endln;
	}
	opserr << "Finished with plastic gravity analysis..." endln;
	s << "puts \"Finished with plastic gravity analysis...\"" << endln << endln;
	


	s << "# 3.3 Update element permeability for post gravity analysis"<< endln << endln;

	// add parameters: hPerm for dynamic analysis
	sprintf(paramArgs[0], "hPerm");
	ElementIter &theElementIterhPerm = theDomain->getElements();
	while ((theEle = theElementIterhPerm()) != 0)
	{
		int theEleTag = theEle->getTag();
		theParameter = new Parameter(numElems + nParaPlus + 1, 0, 0, 0);
		sprintf(paramArgs[1], "%d", matNumDict[theEleTag]);
		theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
		theDomain->addParameter(theParameter);
		nParaPlus += 1;
	}

	// add parameters: vPerm for dynamic analysis
	sprintf(paramArgs[0], "vPerm");
	ElementIter &theElementItervPerm = theDomain->getElements();
	while ((theEle = theElementItervPerm()) != 0)
	{
		int theEleTag = theEle->getTag();
		theParameter = new Parameter(numElems + nParaPlus + 1, 0, 0, 0);
		sprintf(paramArgs[1], "%d", matNumDict[theEleTag]);
		theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
		theDomain->addParameter(theParameter);
		nParaPlus += 1;
	}

	// update hPerm and vPerm 
	theParamIter = theDomain->getParameters();
	while ((theParameter = theParamIter()) != 0)
	{
		int paraTag = theParameter->getTag();
		if (paraTag>(numElems+nParaPlus/2.) & paraTag<=(numElems+3.*nParaPlus/4.))
		{// hperm
			theParameter->update(1.0e-7/9.81/*TODO*/);

		}else if (paraTag>(numElems+3.*nParaPlus/4.)){// vPerm
			theParameter->update(1.0e-7/9.81/*TODO*/);
		}
	}

	theElementIter = theDomain->getElements();
	while ((theEle = theElementIter()) != 0)
	{
		int theEleTag = theEle->getTag();
		//setParameter -value 1 -ele $elementTag hPerm $matTag
		s << "setParameter -value "<<1.0e-7/9.81/*TODO*/<<" -ele "<< theEleTag<<" hPerm "<<endln;
		s << "setParameter -value "<<1.0e-7/9.81/*TODO*/<<" -ele "<< theEleTag<<" vPerm "<<endln;
	}
	s << endln << endln << endln;






	s << "# ------------------------------------------------------------\n";
	s << "# 4. Add the compliant base                                   \n";
	s << "# ------------------------------------------------------------\n\n";

	s << "# 4.1 Set basic properties of the base. \n\n";
	int dashMatTag = soilMatTags.size() + 1; 
	double colArea = sElemX * colThickness; 
	double vis_C = dashpotCoeff * colArea;
	double cFactor = colArea * dashpotCoeff;
	int numberTheViscousMats = 1; // for 3D it's 2
	UniaxialMaterial *theViscousMats[numberTheViscousMats];
	theViscousMats[0] = new ViscousMaterial(dashMatTag, vis_C, 1.0);
	OPS_addUniaxialMaterial(theViscousMats[0]);
	
	s << "set colThickness "<< colThickness << endln;
	s << "set sElemX " << sElemX << endln;
	s << "set colArea [expr $sElemX*$colThickness]" << endln; // [expr $sElemX*$thick(1)]
	s << "set rockVs "<< rockVs << endln;
	s << "set rockDen " << rockDen << endln;
	s << "set dashpotCoeff  [expr $rockVs*$rockDen]" << endln; // [expr $rockVs*$rockDen]
	s << "uniaxialMaterial Viscous " << dashMatTag <<" "<<"[expr $dashpotCoeff*$colArea] 1"<<endln;
	s << "set cFactor [expr $colArea*$dashpotCoeff]" << endln;

	/*
	if (theModelType.compare("2D")) // 3D
	{
		theViscousMats[1] = new ViscousMaterial(numLayers + 20, vis_C, 1.0);
		OPS_addUniaxialMaterial(theViscousMats[1]);
		// TODO: s << 
	}
	*/


	s << "# 4.2 Create dashpot nodes and apply proper fixities. \n\n";

	theNode = new Node(numNodes + 1, 2, 0.0, 0.0);
	theDomain->addNode(theNode); // TODO ?
	theNode = new Node(numNodes + 2, 2, 0.0, 0.0);
	theDomain->addNode(theNode); // TODO ?

	s << "model BasicBuilder -ndm 2 -ndf 2" << endln << endln; 
	s << "node " << numNodes + 1 << " 0.0 0.0" << endln;
	s << "node " << numNodes + 2 << " 0.0 0.0" << endln;

	theSP = new SP_Constraint(numNodes + 1, 0, 0.0, true);
	theDomain->addSP_Constraint(theSP);
	theSP = new SP_Constraint(numNodes + 1, 1, 0.0, true);
	theDomain->addSP_Constraint(theSP);
	s << "fix " << numNodes + 1 << " 1 1" << endln;

	theSP = new SP_Constraint(numNodes + 2, 1, 0.0, true);
	theDomain->addSP_Constraint(theSP);
	s << "fix " << numNodes + 2 << " 0 1" << endln;
	s << endln;



	s << "# 4.3 Apply equalDOF to the node connected to the column. \n\n";

	int numConn = 1; // for 3D it's 2 
	Matrix Ccrconn(numConn, numConn);
	ID rcDOFconn(numConn);
	Ccrconn(0, 0) = 1.0;
	rcDOFconn(0) = 0;
	theMP = new MP_Constraint(1, numNodes + 2, Ccrconn, rcDOFconn, rcDOFconn);
	theDomain->addMP_Constraint(theMP); //TODO
	s << "equalDOF " << 1 << " "<< numNodes + 2 << " 1" << endln;
	



	s << "# 4.4 Remove fixities created for gravity. \n\n";

	for (int i_remove = 0; i_remove < sizeTheSPtoRemove; i_remove++)
	{
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(i_remove));
		delete theSP;
	}
	// TODO:
	s << "remove sp 1 1" << endln;
	s << "remove sp 2 1" << endln;


	s << "# 4.5 Apply equalDOF for the first 4 nodes (3D) or 2 nodes (2D). \n\n";

	int numMP1 = 1;// for 3D it's 2
	Matrix constrainInXZ(numMP1, numMP1);
	ID constDOF(numMP1);
	if (!theModelType.compare("2D")) //2D
	{
		constrainInXZ(0, 0) = 1.0;
		constDOF(0) = 0;
		theMP = new MP_Constraint(1, 2, constrainInXZ, constDOF, constDOF);
		theDomain->addMP_Constraint(theMP);
		s << "equalDOF " << 1 << " "<< 2 << " 1 " << endln;
	}



	s << "# 4.6 Create the dashpot element. \n\n";

	Vector x(3);
	Vector y(3);
	x(0) = 1.0;
	x(1) = 0.0;
	x(2) = 0.0;
	y(0) = 0.0;
	y(1) = 1.0;
	y(2) = 0.0;
	int numberDirections = 1;// for 3D it's 2
	ID directions(numberDirections);
	directions(0) = 0; 
	//directions(1) = 2; // 3D
	//element zeroLength [expr $nElemT+1]  $dashF $dashS -mat [expr $numLayers+1]  -dir 1
	theEle = new ZeroLength(numElems + 1, 2, numNodes + 1, numNodes + 2, x, y, 1, theViscousMats, directions); //TODO ?
	theDomain->addElement(theEle);
	s << "element zeroLength "<<numElems + 1 <<" "<< numNodes + 1 <<" "<< numNodes + 2<<" -mat "<<dashMatTag<<"  -dir 1" << endln;
	s << "\n\n\n";
	

	s << "setTime 0.0" << endln;
	s << "wipeAnalysis" << endln;
	s << "remove recorders" << endln << endln << endln;



	//setTime 0.0
	//wipeAnalysis
	//remove recorders
	theDomain->setCommittedTime(0.0);
	delete theAnalysis;
	theDomain->removeRecorders();










	s << "# ------------------------------------------------------------\n";
	s << "# 5. Dynamic analysis                                         \n";
	s << "# ------------------------------------------------------------\n\n";

	s << "model BasicBuilder -ndm 2 -ndf 3" << endln; // TODO: it seems this is not necessary.


	s << "# ------------------------------------------------------------\n";
	s << "# 5.1 Apply the rock motion                                    \n";
	s << "# ------------------------------------------------------------\n\n";
	int numSteps = 0;
	std::vector<double> dt;


	double dT = 0.001; // This is the time step in solution
	double motionDT =  0.005; // This is the time step in the motion record. TODO: use a funciton to get it
	int nSteps = 1998;//theMotionX->getNumSteps() ; //1998; // number of motions in the record. TODO: use a funciton to get it
	int remStep = nSteps * motionDT / dT;
	s << "set dT " << dT << endln;
	s << "set motionDT " << motionDT << endln;
	s << "set mSeries \"Path -dt $motionDT -filePath /Users/simcenter/Codes/SimCenter/SiteResponseTool/test/RSN766_G02_000_VEL.txt -factor $cFactor\""<<endln;
	// using a stress input with the dashpot
	if (theMotionX->isInitialized())
	{
		LoadPattern *theLP = new LoadPattern(1, vis_C);
		theLP->setTimeSeries(theMotionX->getVelSeries());

		NodalLoad *theLoad;
		int numLoads = 3; // for 3D it's 4
		Vector load(numLoads);
		load(0) = 1.0;
		load(1) = 0.0;
		load(2) = 0.0;
		//load(3) = 0.0;

		//theLoad = new NodalLoad(1, numNodes + 2, load, false); theLP->addNodalLoad(theLoad);
		theLoad = new NodalLoad(1, 1, load, false);
		theLP->addNodalLoad(theLoad);
		theDomain->addLoadPattern(theLP);

		s << "pattern Plain 10 $mSeries {"<<endln;
		s << "    load 1  1.0 0.0 0.0" << endln;
		s << "}" << endln << endln;

		// update the number of steps as well as the dt vector
		int temp = theMotionX->getNumSteps();
		if (temp > numSteps)
		{
			numSteps = temp;
			dt = theMotionX->getDTvector();
		}
	}



	s << "# ------------------------------------------------------------\n";
	s << "# 5.2 Define the analysis                                     \n";
	s << "# ------------------------------------------------------------\n\n";
	// I have to change to a transient analysis
	// remove the static analysis and create new transient objects
	delete theIntegrator;
	//delete theAnalysis;

	//theTest->setTolerance(1.0e-5);

	s << "constraints Transformation" << endln; 
	s << "test NormDispIncr 1.0e-4 35 0" << endln; // TODO
	s << "algorithm   Newton" << endln;
	s << "numberer    RCM" << endln;
	s << "system BandGeneral" << endln;





	// create analysis objects - I use static analysis for gravity
	theModel = new AnalysisModel();
	theTest = new CTestNormDispIncr(1.0e-4, 35, 1);                    // 2. test NormDispIncr 1.0e-7 30 1
	theSolnAlgo = new NewtonRaphson(*theTest);                              // 3. algorithm   Newton (TODO: another option: KrylovNewton) 
	//StaticIntegrator *theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0); // *
	//ConstraintHandler *theHandler = new TransformationConstraintHandler(); // *
	//TransientIntegrator* theIntegrator = new Newmark(5./6., 4./9.);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
	theHandler = new PenaltyConstraintHandler(1.0e16, 1.0e16);          // 1. constraints Penalty 1.0e15 1.0e15
	theRCM = new RCM();
	theNumberer = new DOF_Numberer(*theRCM);                                 // 4. numberer RCM (another option: Plain)
	theSolver = new BandGenLinLapackSolver();                            // 5. system BandGeneral (TODO: switch to SparseGeneral)
	theSOE = new BandGenLinSOE(*theSolver);


	//VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//StaticAnalysis *theAnalysis; // *
	//theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator); // *
	






	double gamma_dynm = 0.5;
	double beta_dynm = 0.25;
	TransientIntegrator* theTransientIntegrator = new Newmark(gamma_dynm, beta_dynm);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
	//theTransientIntegrator->setConvergenceTest(*theTest);

	// setup Rayleigh damping   TODO: calcualtion of these paras
	// apply 2% at the natural frequency and 5*natural frequency
	double natFreq = SRM_layering.getNaturalPeriod();
	double pi = 4.0 * atan(1.0);

	/*
	double dampRatio = 0.02;
	double a0 = dampRatio * (10.0 * pi * natFreq) / 3.0;
	double a1 = dampRatio / (6.0 * pi * natFreq);
	*/

	// method in N10_T3 
	double fmin = 5.01;
	double Omegamin  = fmin * 2.0 * pi;
	double ximin = 0.025;
	double a0 = ximin * Omegamin; //# factor to mass matrix
	double a1 = ximin / Omegamin; //# factor to stiffness matrix

	if (PRINTDEBUG)
	{
		opserr << "f1 = " << natFreq << "    f2 = " << 5.0 * natFreq << endln;
		opserr << "a0 = " << a0 << "    a1 = " << a1 << endln;
	}
	theDomain->setRayleighDampingFactors(a0, a1, 0.0, 0.0);

	DirectIntegrationAnalysis* theTransientAnalysis;
	theTransientAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis *theTransientAnalysis;
	//theTransientAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

	// reset time in the domain
	theDomain->setCurrentTime(0.0);

	s << "set gamma_dynm " << gamma_dynm << endln;
	s << "set beta_dynm " << beta_dynm << endln;
	s << "integrator  Newmark $gamma_dynm $beta_dynm" << endln;
	s << "set a0 " << a0 << endln;
	s << "set a1 " << a1 << endln;
	s << "rayleigh    $a0 $a1 0.0 0.0" << endln;
	//s << "analysis Transient" << endln << endln;
	s << "analysis Transient" << endln << endln;
	
	// count quad elements
	ElementIter &theElementIterh = theDomain->getElements();
	std::vector<int> quadElem;
	while ((theEle = theElementIterh()) != 0)
	{
		int theEleTag = theEle->getTag();
		if (theEle->getNumDOF() == 12) // quad ele
			quadElem.push_back(theEleTag);
	}
	int numQuadEles = quadElem.size();


	s << "# ------------------------------------------------------------\n";
	s << "# 5.3 Define outputs and recorders                            \n";
	s << "# ------------------------------------------------------------\n\n";


	// Record the response at the surface
	std::string outFile = theOutputDir + PATH_SEPARATOR + "surface.acc";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.vel";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "surface.disp";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	s<< "eval \"recorder Node -file out_tcl/surface.disp -time -dT $motionDT -node "<<numNodes<<" -dof 1 2 3  disp\""<<endln;// 1 2
	s<< "eval \"recorder Node -file out_tcl/surface.acc -time -dT $motionDT -node "<<numNodes<<" -dof 1 2 3  accel\""<<endln;// 1 2
	s<< "eval \"recorder Node -file out_tcl/surface.vel -time -dT $motionDT -node "<<numNodes<<" -dof 1 2 3 vel\""<<endln;// 3


	
	// Record the response of base node
	nodesToRecord.resize(1);
	nodesToRecord(0) = 1;
	
	dofToRecord.resize(1);
	dofToRecord(0) = 0; // only record the x dof

	outFile = theOutputDir + PATH_SEPARATOR + "base.acc";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "base.vel";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "base.disp";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	s<< "eval \"recorder Node -file out_tcl/base.disp -time -dT $motionDT -node 1 -dof 1 2 3  disp\""<<endln;// 1 2
	s<< "eval \"recorder Node -file out_tcl/base.acc -time -dT $motionDT -node 1 -dof 1 2 3  accel\""<<endln;// 1 2
	s<< "eval \"recorder Node -file out_tcl/base.vel -time -dT $motionDT -node 1 -dof 1 2 3 vel\""<<endln;// 3

	// Record pwp at node 17
	dofToRecord.resize(1);
	dofToRecord(0) = 2; // only record the pore pressure dof
	ID pwpNodesToRecord(1);
	pwpNodesToRecord(0) = 17;
	outFile = theOutputDir + PATH_SEPARATOR + "pwpLiq.out";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &pwpNodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	s<< "eval \"recorder Node -file out_tcl/pwpLiq.out -time -dT $motionDT -node 17 -dof 3 vel\""<<endln;


	// Record the response of all nodes
	nodesToRecord.resize(numNodes);
	for (int i=0;i<numNodes;i++)
		nodesToRecord(i) = i;
	dofToRecord.resize(2);
	dofToRecord(0) = 0;
	dofToRecord(1) = 1;

	outFile = theOutputDir + PATH_SEPARATOR + "displacement.out";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "velocity.out";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	outFile = theOutputDir + PATH_SEPARATOR + "acceleration.out";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	dofToRecord.resize(1);
	dofToRecord(0) = 2;
	outFile = theOutputDir + PATH_SEPARATOR + "porePressure.out";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

	s<< "eval \"recorder Node -file out_tcl/displacement.out -time -dT $motionDT -nodeRange 1 "<<numNodes<<" -dof 1 2  disp\""<<endln;
	s<< "eval \"recorder Node -file out_tcl/velocity.out -time -dT $motionDT -nodeRange 1 "<<numNodes<<" -dof 1 2  vel\""<<endln;
	s<< "eval \"recorder Node -file out_tcl/acceleration.out -time -dT $motionDT -nodeRange 1 "<<numNodes<<" -dof 1 2  accel\""<<endln;
	s<< "eval \"recorder Node -file out_tcl/porePressure.out -time -dT $motionDT -nodeRange 1 "<<numNodes<<" -dof 3 vel\""<<endln;

	
	// Record element results
	OPS_Stream* theOutputStream2;
	ID elemsToRecord(quadElem.size());
	for (int i=0;i<quadElem.size();i+=1)
		elemsToRecord(i) = quadElem[i];
	const char* eleArgs = "stress";
	outFile = theOutputDir + PATH_SEPARATOR + "stress.out";
	theOutputStream2 = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new ElementRecorder(&elemsToRecord, &eleArgs, 1, true, *theDomain, *theOutputStream2, motionDT, NULL);
	theDomain->addRecorder(*theRecorder);

	const char* eleArgsStrain = "strain";
	outFile = theOutputDir + PATH_SEPARATOR + "strain.out";
	theOutputStream2 = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new ElementRecorder(&elemsToRecord, &eleArgsStrain, 1, true, *theDomain, *theOutputStream2, motionDT, NULL);
	theDomain->addRecorder(*theRecorder);

	s<< "recorder Element -file out_tcl/stress.out -time -dT $motionDT  -eleRange 1 "<<numQuadEles<<"  stress 3"<<endln;
	s<< "recorder Element -file out_tcl/strain.out -time -dT $motionDT  -eleRange 1 "<<numQuadEles<<"  strain"<<endln;
	s<< endln << endln;


	



	s << "# ------------------------------------------------------------\n";
	s << "# 5.4 Perform dynamic analysis                                \n";
	s << "# ------------------------------------------------------------\n\n";

	s << "set nSteps " << nSteps << endln;
	s << "set remStep " << remStep << endln;
	s << "set success 0" << endln << endln;

	s << "proc subStepAnalyze {dT subStep} {" << endln;
	s << "	if {$subStep > 10} {" << endln;
	s << "		return -10" << endln;
	s << "	}" << endln;
	s << "	for {set i 1} {$i < 3} {incr i} {" << endln;
	s << "		puts \"Try dT = $dT\"" << endln;
	s << "		set success [analyze 1 $dT]" << endln;
	s << "		if {$success != 0} {" << endln;
	s << "			set success [subStepAnalyze [expr $dT/2.0] [expr $subStep+1]]" << endln;
	s << "			if {$success == -10} {" << endln;
	s << "				puts \"Did not converge.\"" << endln;
	s << "				return $success" << endln;
	s << "			}" << endln;
	s << "		} else {" << endln;
	s << "			if {$i==1} {" << endln;
	s << "				puts \"Substep $subStep : Left side converged with dT = $dT\"" << endln;
	s << "			} else {" << endln;
	s << "				puts \"Substep $subStep : Right side converged with dT = $dT\"" << endln;
	s << "			}" << endln;
	s << "		}" << endln;
	s << "	}" << endln;
	s << "	return $success" << endln;
	s << "}" << endln << endln << endln;


	/*
	// solution 1: direct steps 
	s << "set thisStep 0"<<endln;
	s << "set success 0"<<endln;
	s << "while {$thisStep < 1998} {"<<endln;
	s << "    set thisStep [expr $thisStep+1]"<<endln;
	s << "    set success [analyze 1 $dT]"<<endln;
	s << "    if {$success == 0} {;# success"<<endln;
	s << "        puts \"Analysis Finished at step: $thisStep\""<<endln;
	s << "    } else {"<<endln;
	s << "        puts \"Analysis Failed at step: $thisStep ----------------------------------------------!!!\""<<endln;
	s << "    }"<<endln;
	s << "}"<<endln<<endln; 
	s << "wipe"<<endln;
	s << "puts \"Site response analysis is finished.\n\""<< endln;
	s << "exit"<<endln<< endln <<endln;
	*/
	

	
	s << "puts \"Start analysis\"" << endln;
	s << "set startT [clock seconds]" << endln;
	s << "while {$success != -10} {" << endln;
	s << "	set subStep 0" << endln;
	s << "	set success [analyze $remStep  $dT]" << endln;
	s << "	if {$success == 0} {" << endln;
	s << "		puts \"Analysis Finished\"" << endln;
	s << "		break" << endln;
	s << "	} else {" << endln;
	s << "		set curTime  [getTime]" << endln;
	s << "		puts \"Analysis failed at $curTime . Try substepping.\"" << endln;
	s << "		set success  [subStepAnalyze [expr $dT/2.0] [incr subStep]]" << endln;
	s << "		set curStep  [expr int($curTime/$dT + 1)]" << endln;
	s << "		set remStep  [expr int($nSteps-$curStep)]" << endln;
	s << "		puts \"Current step: $curStep , Remaining steps: $remStep\"" << endln;
	s << "	}" << endln;
	s << "}" << endln << endln;
	s << "set endT [clock seconds]" << endln << endln;
	s << "puts \"loading analysis execution time: [expr $endT-$startT] seconds.\"" << endln << endln;
	s << "puts \"Finished with dynamic analysis...\"" << endln << endln;

	s << endln;
	s << "print -file out_tcl/Domain.out" << endln << endln;
	
	s << "wipe" << endln;
	s << "puts \"Site response analysis is finished.\n\""<< endln;
	s << "exit" << endln << endln;

	s.close();
	
	


	/*
	OPS_Stream* theOutputStreamAll;
	theOutputStreamAll = new DataFileStream("Domain.out", OVERWRITE, 2, 0, false, 6, false);
	theDomain->Print(*theOutputStreamAll);
	opserr << theOutputStreamAll;
	delete theOutputStreamAll;
	*/



	double totalTime = dT * nSteps;
	int success = 0;

	opserr << "Analysis started:" << endln;
	std::stringstream progressBar;
	for (int analysisCount = 0; analysisCount < remStep; ++analysisCount)
	{
		//int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		double stepDT = dt[analysisCount];
		//int converged = theTransientAnalysis->analyze(1, stepDT, stepDT / 2.0, stepDT * 2.0, 1); // *
		//int converged = theTransientAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
		int converged = theTransientAnalysis->analyze(1, dT);
		if (!converged)
		{
			opserr << "Converged at time " << theDomain->getCurrentTime() << endln;

			if (analysisCount % (int)(remStep / 20) == 0)
			{
				progressBar << "\r[";
				for (int ii = 0; ii < ((int)(20 * analysisCount / remStep)-1); ii++)
					progressBar << "-";
				progressBar << " 🚌  ";
				for (int ii = (int)(20 * analysisCount / remStep)+1; ii < 20; ii++)
					progressBar << ".";

				progressBar << "]  " << (int)(100 * analysisCount / remStep) << "%";
				opsout << progressBar.str().c_str();
				opsout.flush();
			}
		}
		else
		{
			opserr << "Site response analysis did not converge." << endln;
			exit(-1);
		}
	}
	opserr << "Site response analysis done..." << endln;
	progressBar << "\r[";
	for (int ii = 0; ii < 20; ii++)
		progressBar << "-";

	progressBar << "]  🚌   100%\n";
	opsout << progressBar.str().c_str();
	opsout.flush();
	opsout << endln;


	return 0;
}




int SiteResponseModel::subStepAnalyze(double dT, int subStep, int success, int remStep, DirectIntegrationAnalysis* theTransientAnalysis)
{
	if (subStep > 10)
		return -10;
	for (int i; i < 3; i++)
	{
		opserr << "Try dT = " << dT << endln;
		success = theTransientAnalysis->analyze(remStep, dT);// 0 means success
		//success = subStepAnalyze(int(dT/2), subStep +1, success);
	}
	
	return 0;

}

int SiteResponseModel::runEffectiveStressModel2D()
{

}
