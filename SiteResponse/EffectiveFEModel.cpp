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

#include <functional>
#include <vector>
#include <iostream>
#include <iomanip>
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


#include "SSPbrick.h"
#include "SSPquad.h"
#include "SSPquadUP.h"
#include "SSPbrickUP.h"
#include "Brick.h"
#include "J2CyclicBoundingSurface.h"
#include "ElasticIsotropicMaterial.h"
#include "PM4Sand.h"
#include "PM4Silt.h"
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
#include "PressureDependMultiYield02.h"
#include "PressureDependMultiYield.h"
#include "PressureIndependMultiYield.h"
#include "ManzariDafalias.h"

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

SiteResponseModel::SiteResponseModel(std::string modelType, OutcropMotion *motionX, std::function<bool(double)> callbackFunction) : theModelType(modelType),
																											 theMotionX(motionX),
                                                                                                             theOutputDir(".")
{
	if (theMotionX->isInitialized())
    {
        theDomain = new Domain();
        m_callbackFunction = callbackFunction;
    }
	else
	{
        opserr << "No motion is specified. Program exited." << endln;
		exit(-1);
	}
}

SiteResponseModel::SiteResponseModel(std::string modelType, OutcropMotion *motionX, OutcropMotion *motionZ, std::function<bool(double)> callbackFunction) : theModelType(modelType),
                                                                                                             theMotionX(motionX),
                                                                                                             theMotionZ(motionZ),
                                                                                                             theOutputDir(".")
{
    if (theMotionX->isInitialized() && theMotionX->isInitialized())
    {
        theDomain = new Domain();
        m_callbackFunction = callbackFunction;
    }
    else
    {
        opserr << "No motion is specified. Program exited." << endln;
        exit(-1);
    }
}

SiteResponseModel::SiteResponseModel(std::string modelType, OutcropMotion *motionX) : theModelType(modelType),
                                                                                                             theMotionX(motionX),
                                                                                                             theOutputDir(".")
{
    if (theMotionX->isInitialized())
    {
        theDomain = new Domain();
    }
    else
    {
        opserr << "No motion is specified. Program exited." << endln;
        exit(-1);
    }
}

SiteResponseModel::SiteResponseModel(std::string modelType, OutcropMotion *motionX, OutcropMotion *motionZ) : theModelType(modelType),
                                                                                                             theMotionX(motionX),
                                                                                                             theMotionZ(motionZ),
                                                                                                             theOutputDir(".")
{
    if (theMotionX->isInitialized() && theMotionX->isInitialized())
    {
        theDomain = new Domain();
    }
    else
    {
        opserr << "No motion is specified. Program exited." << endln;
        exit(-1);
    }
}

SiteResponseModel::~SiteResponseModel()
{
	if (theDomain != NULL)
		delete theDomain;
	theDomain = NULL;
}


int SiteResponseModel::buildEffectiveStressModel2D(bool doAnalysis)
{
    m_doAnalysis = doAnalysis;

	Vector zeroVec(3);
	zeroVec.Zero();

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
    //std::string configFile = "SRT.json";
    std::ifstream i(theConfigFile);
    if(!i)
        return false;// failed to open SRT.json TODO: print to log
    json SRT;
    i >> SRT;

	// set outputs for tcl 
	//ofstream s ("/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/model.tcl", std::ofstream::out);
    /*
	ofstream s ("model.tcl", std::ofstream::out);
	ofstream ns ("out_tcl/nodesInfo.dat", std::ofstream::out);
	ofstream es ("out_tcl/elementInfo.dat", std::ofstream::out);
    */
    ofstream s (theAnalysisDir + "/model.tcl", std::ofstream::out);//TODO: may not work on windows
    s.precision(16);
    ofstream ns (theTclOutputDir+"/nodesInfo.dat", std::ofstream::out);
    ofstream es (theTclOutputDir+"/elementInfo.dat", std::ofstream::out);
	//ofstream s ("/Users/simcenter/Codes/SimCenter/build-SiteResponseTool-Desktop_Qt_5_11_1_clang_64bit-Debug/SiteResponseTool.app/Contents/MacOS/model.tcl", std::ofstream::out);
	s << "# #########################################################" << "\n\n";
	s << "wipe \n\n";


	// basic settings
	int numLayers = 0;
	int numNodes = 0;
	int numElems = 0;
	double totalHeight = 0.0;
	double sElemX = 0.0;
    double slopex1 = 0.0;
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
        slopex1 = basicSettings["slopex1"];
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
	std::map<int, int> matNumDict;
	std::vector<int> soilMatTags;
    std::vector<double> vPermVec;
    std::vector<double> hPermVec;

    std::map<int, std::string> eleTypeDict;

	theNode = new Node(numNodes + 1, 3, 0.0, yCoord); theDomain->addNode(theNode);
	theNode = new Node(numNodes + 2, 3, sElemX, yCoord); theDomain->addNode(theNode);
	s << "model BasicBuilder -ndm 2 -ndf 3  \n\n";
	s << "node " << numNodes + 1 << " 0.0 " << yCoord << endln;
	s << "node " << numNodes + 2 << " " << sElemX << " " << yCoord << endln;
	ns << numNodes + 1 << " 0.0 " << yCoord << endln;
	ns << numNodes + 2 << " " << sElemX << " " << yCoord << endln;
    numNodes += 2;

    s << std::scientific << std::setprecision(14);

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
            if (eSizeV<minESizeV) {
                //std::string err = "eSize is tool small. change it in the json file.";throw err;
                eSizeV = minESizeV;
            }
            double thickness = l["thickness"];
            double vs = l["vs"];
            double Dr = l["Dr"];
			double vPerm = l["vPerm"];
			double hPerm = l["hPerm"];
			double uBulk = l["uBulk"];
            double evoid = l["void"];
            std::string color = l["color"];
			std::string lname = l["name"];
			if (!lname.compare("Rock"))
			{
                //rockVs = vs;
                //rockDen = l["density"];
				continue;
			}

            //double evoid = 0.0;
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
				
                //evoid  = emax - Dr * (emax - emin);
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

                //evoid  = emax - thisDr * (emax - emin);
				rho_d = Gs / (1 + evoid);
				rho_s = rho_d *(1.0+evoid/Gs);
				//theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
				theMat = new PM4Sand(matTag, thisDr,G0,hpo,thisDen,P_atm,h0,emax,emin,nb,nd,Ado,z_max,cz,ce,phic,nu,cgd,cdr,ckaf,Q,R,m,Fsed_min,p_sedo);
				s << "nDMaterial PM4Sand " << matTag<< " " << thisDr<< " " <<G0<< " " <<hpo<< " " <<thisDen<< " " <<P_atm<< " " <<h0<< " "<<emax<< " "<<emin<< " " <<
				nb<< " " <<nd<< " " <<Ado<< " " <<z_max<< " " <<cz<< " " <<ce<< " " <<phic<< " " <<nu<< " " <<cgd<< " " <<cdr<< " " <<ckaf<< " " <<
				Q<< " " <<R<< " " <<m<< " " <<Fsed_min<< " " <<p_sedo << endln;

                /*
                theMat = new PM4Sand(matTag, thisDr,G0,hpo,thisDen);
                s << "nDMaterial PM4Sand " << matTag<< " " << thisDr<< " " <<G0<< " " <<hpo<< " " <<thisDen << endln;
                */
            }else if(!matType.compare("PM4Silt"))
            {
                double thisDr = mat["Dr"];
                double S_u = mat["S_u"];
                double Su_Rat = mat["Su_Rat"];
                double G_o = mat["G_o"];
                double h_po = mat["h_po"];
                double thisDen = mat["Den"];

                double Su_factor = mat["Su_factor"];
                double P_atm = mat["P_atm"];
                double nu = mat["nu"];
                double nG = mat["nG"];
                double h0 = mat["h0"];
                double eInit = mat["eInit"];
                double lambda = mat["lambda"];
                double phicv = mat["phicv"];
                double nb_wet = mat["nb_wet"];
                double nb_dry = mat["nb_dry"];
                double nd = mat["nd"];
                double Ado = mat["Ado"];
                double ru_max = mat["ru_max"];
                double z_max = mat["z_max"];
                double cz = mat["cz"];
                double ce = mat["ce"];
                double cgd = mat["cgd"];
                double ckaf = mat["ckaf"];
                double m_m = mat["m_m"];
                double CG_consol = mat["CG_consol"];

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                theMat = new PM4Silt(matTag, S_u, Su_Rat, G_o, h_po, thisDen, Su_factor, P_atm,nu, nG, h0, eInit, lambda, phicv, nb_wet, nb_dry, nd, Ado, ru_max, z_max,cz, ce, cgd, ckaf, m_m, CG_consol);
                s << "nDMaterial PM4Silt " << matTag<< " " << S_u<< " " <<Su_Rat<< " " <<G_o<< " " <<h_po<< " " <<thisDen<< " "
                  <<Su_factor<< " " <<P_atm<< " " <<nu<< " " <<nG<< " " <<h0<< " " <<eInit<< " " <<lambda<< " " <<phicv<< " "
                 <<nb_wet<< " " <<nb_dry<< " " <<nd<< " " <<Ado<< " " <<ru_max<< " " <<z_max<< " " <<cz<< " " <<ce<< " " <<cgd
                << " " <<ckaf<< " " <<m_m<< " " <<CG_consol << endln;
            }else if(!matType.compare("PIMY"))
            {
                double thisDr = mat["Dr"];
                int nd = 2;//mat["nd"];
                double rho = mat["rho"];
                double refShearModul = mat["refShearModul"];
                double refBulkModul = mat["refBulkModul"];
                double cohesi = mat["cohesi"];
                double peakShearStra = mat["peakShearStra"];

                double frictionAng = mat["frictionAng"];
                double refPress = mat["refPress"];
                double pressDependCoe = mat["pressDependCoe"];
                double noYieldSurf = mat["noYieldSurf"];

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                //TODO: PM4Silt->PIMY
                //TODO: deal with noYieldSurf
                theMat = new PressureIndependMultiYield(matTag,nd,rho,refShearModul,refBulkModul,cohesi,peakShearStra,
                                                        frictionAng, refPress,  pressDependCoe);
                s << "nDMaterial PressureIndependMultiYield "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "<<refBulkModul<<" "<<cohesi<<" "<<peakShearStra<<" "<<
                        frictionAng<<" "<< refPress<<" "<<pressDependCoe<<endln;
            }else if(!matType.compare("PDMY"))
            {

                double thisDr = mat["Dr"];
                int nd = 2;//mat["nd"];
                double rho = mat["rho"];
                double refShearModul = mat["refShearModul"];
                double refBulkModul = mat["refBulkModul"];
                double frictionAng = mat["frictionAng"];
                double peakShearStra = mat["peakShearStra"];

                double refPress = mat["refPress"];
                double pressDependCoe = mat["pressDependCoe"];
                double PTAng = mat["PTAng"];
                double contrac = mat["contrac"];
                double dilat1 = mat["dilat1"];
                double dilat2 = mat["dilat2"];
                double liquefac1 = mat["liquefac1"];
                double liquefac2 = mat["liquefac2"];
                double liquefac3 = mat["liquefac3"];
                double e = mat["e"];
                double cs1 = mat["cs1"];
                double cs2 = mat["cs2"];
                double cs3 = mat["cs3"];
                double pa = mat["pa"];
                double c = mat["c"];
                double noYieldSurf = mat["noYieldSurf"];

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                //TODO: PM4Silt->PDMY
                //TODO: deal with noYieldSurf
                double hv = 0.;
                double pv = 1.;

                theMat = new PressureDependMultiYield(matTag,nd,rho,refShearModul,refBulkModul,frictionAng,peakShearStra,
                                                      refPress,pressDependCoe,PTAng,contrac,dilat1,dilat2,liquefac1,liquefac2,liquefac3,20,0,
                                                      e, cs1,cs2,cs3,pa,c);
                s << "nDMaterial PressureDependMultiYield "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "<<refBulkModul<<" "<<frictionAng<<" "<<peakShearStra<<" "<<
                        refPress<<" "<<pressDependCoe<<" "<<PTAng<<" "<<contrac<<" "<<dilat1<<" "<<dilat2<<" "<<liquefac1<<" "<<liquefac2<<" "<<liquefac3 << " " << "20"
                          <<" "<<e<<" "<<cs1<<" "<<cs2<<" "<<cs3<<" "<<pa<<" "<<c <<endln;
            }else if(!matType.compare("PDMY02"))
            {

                double thisDr = mat["Dr"];
                double nd = 2;// mat["nd"];
                double rho = mat["rho"];
                double refShearModul = mat["refShearModul"];
                double refBulkModul = mat["refBulkModul"];
                double frictionAng = mat["frictionAng"];
                double peakShearStra = mat["peakShearStra"];

                double refPress = mat["refPress"];
                double pressDependCoe = mat["pressDependCoe"];
                double PTAng = mat["PTAng"];
                double contrac1 = mat["contrac1"];
                double contrac3 = mat["contrac3"];
                double dilat1 = mat["dilat1"];
                double dilat3 = mat["dilat3"];
                double contrac2 = mat["contrac2"];
                double dilat2 = mat["dilat2"];
                double liquefac1 = mat["liquefac1"];
                double liquefac2 = mat["liquefac2"];
                double e = mat["e"];
                double cs1 = mat["cs1"];
                double cs2 = mat["cs2"];
                double cs3 = mat["cs3"];
                double pa = mat["pa"];
                double c = mat["c"];
                double noYieldSurf = mat["noYieldSurf"];

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                //TODO: PM4Silt->PDMY02
                //TODO: deal with noYieldSurf
                theMat = new PressureDependMultiYield02(matTag,nd,rho,refShearModul,refBulkModul,frictionAng,
                       peakShearStra, refPress,  pressDependCoe,PTAng,contrac1,contrac3,  dilat1,dilat3,20,0,
                              contrac2, dilat2,liquefac1,liquefac2,e,cs1,cs2,cs3,pa);
                s << "nDMaterial PressureDependMultiYield02 "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "<<refBulkModul<<" "<<frictionAng<<" "<<peakShearStra<<" "<<
                        refPress<<" "<<pressDependCoe<<" "<<PTAng<<" "<<contrac1<<" "<<contrac3<<" "<<dilat1<<" "<<dilat3<<" 20 "<<contrac2<<" "<<dilat2<<" "<<liquefac1<<" "<<liquefac2
                          <<" "<<e<<" "<<cs1<<" "<<cs2<<" "<<cs3<<" "<<pa<<" "<<endln;

            }
            else if(!matType.compare("ManzariDafalias"))
                        {
                            double Dr = mat["Dr"];
                            double G0 = mat["G0"];
                            double nu = mat["nu"];
                            double e_init = mat["e_init"];
                            double Mc = mat["Mc"];
                            double c = mat["c"];

                            double lambda_c = mat["lambda_c"];
                            double e0 = mat["e0"];
                            double ksi = mat["ksi"];
                            double P_atm = mat["P_atm"];
                            double m = mat["m"];
                            double h0 = mat["h0"];
                            double ch = mat["ch"];
                            double nb = mat["nb"];
                            double A0 = mat["A0"];
                            double nd = mat["nd"];
                            double z_max = mat["z_max"];
                            double cz = mat["cz"];
                            double Den = mat["Den"];

                            //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                            //TODO: PM4Silt->ManzariDafalias
                            theMat = new ManzariDafalias(matTag, G0, nu, e_init, Mc, c, lambda_c, e0, ksi, P_atm, m, h0, ch, nb, A0, nd, z_max, cz, Den);
                            s << "nDMaterial ManzariDafalias " << matTag<< " " << G0<< " " <<nu<< " " <<e_init<< " " <<Mc<< " " <<c<< " " <<lambda_c<< " "
                              <<e0<< " " <<ksi<< " " <<P_atm<< " " <<m<< " " <<h0<< " " <<ch<< " " <<nb<< " " <<A0<< " " <<nd<< " " <<z_max<< " " <<cz<< " " <<Den << endln;
                        }
            else if(!matType.compare("J2Bounding"))
                        {
                            double Dr = mat["Dr"];
                            double G = mat["G"];
                            double K = mat["K"];
                            double su = mat["su"];
                            double rho = mat["rho"];
                            double h = mat["h"];
                            double m = mat["m"];
                            double k_in = mat["k_in"];
                            double beta = mat["beta"];


                            // new J2
                            //TODO: k_in -> chi
                            double h0 = 0.0;
                            theMat = new J2CyclicBoundingSurface(matTag, G, K, su, rho, h, m,h0, k_in, beta);
                            s << "nDMaterial J2CyclicBoundingSurface " << matTag<< " " << G<< " " <<K<< " "
                              <<su<< " " <<rho<< " " <<h<< " " <<m<< " " << h0 << " " <<k_in<< " " <<beta << endln;


                            /*
                             * double h0 = 0.0;
                            theMat = new J2CyclicBoundingSurface(matTag, G, K, su, rho, h, m, k_in, beta);
                            s << "nDMaterial J2CyclicBoundingSurface " << matTag<< " " << G<< " " <<K<< " "
                              <<su<< " " <<rho<< " " <<h<< " " <<m << " " << h0 << " " <<k_in<< " " <<beta << endln;
                            */
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
				ns << numNodes + 1 << " 0.0 " << yCoord << endln;
				ns << numNodes + 2 << " " << sElemX << " " << yCoord << endln;

                double alpha = 1.0e-8;


				s << "element SSPquadUP "<<numElems + 1<<" " 
					<<numNodes - 1 <<" "<<numNodes<<" "<< numNodes + 2<<" "<< numNodes + 1<<" "
                    << theMat->getTag() << " " << "1.0 "<<uBulk<<" 1.0 1.0 1.0 " <<evoid << " "<< alpha<< " ";
                double b1 = 0.0, b2 = 0.0;
                slopex1 = slopex1 > 90 ? (180.-slopex1) : slopex1;
                if (slopex1 <= 90)
                {
                    b1 = -1.0 * g * sin(slopex1*pi/180.);
                    b2 = g * cos(slopex1*pi/180.);
                    s<< std::to_string(b1) <<" "<< std::to_string(b2)  << endln;
                }
                else
                {
                    b1 = 1.0 * g * sin((180-slopex1)*pi/180.);
                    b2 = g * cos((180-slopex1)*pi/180.);
                    s<< std::to_string(b1) <<" "<< std::to_string(b2)  << endln;
                }
				es << numElems + 1<<" " <<numNodes - 1 <<" "<<numNodes<<" "<< numNodes + 2<<" "<< numNodes + 1<<" "
					<< theMat->getTag() << endln;

                theEle = new SSPquadUP(numElems + 1, numNodes - 1, numNodes, numNodes + 2, numNodes + 1,
                                       *theMat, 1.0, uBulk, 1.0, 1.0, 1.0, evoid, alpha, b1, b2); // -9.81 * theMat->getRho() TODO: theMat->getRho()
                hPermVec.push_back(hPerm);
                vPermVec.push_back(vPerm);

				theDomain->addElement(theEle);

				matNumDict[numElems + 1] = theMat->getTag();
                eleTypeDict[numElems + 1] = matType;



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
        if (0.0 >= (totalHeight - groundWaterTable))
        { 	//record dry nodes above ground water table
            dryNodes.push_back(1);
            dryNodes.push_back(2);
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
    int sizeTheSPtoRemove = 2 ; // for 3D2D it's 8, for 3D1D it's 4;
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
    int crrdim = 2 ;//For 3D it's 3;  TODO
	Matrix Ccr(crrdim, crrdim);
	ID rcDOF(crrdim);
	// TODO: get clarified about the dimensions of Crr and rfDOF
    // Confirmed for 2D:
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

    double gamma = 0.8333;// 5./6.;
    double beta = 0.4444;//4./9.;

	s << "constraints Transformation" << endln;
	s << "test NormDispIncr 1.0e-4 35 1" << endln;
	s << "algorithm   Newton" << endln;
	s << "numberer RCM" << endln;
    s << "system SparseGeneral" << endln;//BandGeneral
	s << "set gamma " << gamma << endln;
	s << "set beta " << beta << endln;
	s << "integrator  Newmark $gamma $beta" << endln;
	s << "analysis Transient" << endln << endln;
	
	s << "set startT  [clock seconds]" << endln;
    s << "analyze     10 1.0" << endln;
	s << "puts \"Finished with elastic gravity analysis...\"" << endln << endln;

	// create analysis objects - I use static analysis for gravity
    //AnalysisModel *
    theModel = new AnalysisModel();
    //CTestNormDispIncr *
    theTest = new CTestNormDispIncr(1.0e-4, 35, 1);
    //EquiSolnAlgo *
    theSolnAlgo = new NewtonRaphson(*theTest);
    // 2. test NormDispIncr 1.0e-7 30 1
    //EquiSolnAlgo *theSolnAlgo = new NewtonRaphson(*theTest);                              // 3. algorithm   Newton (TODO: another option: KrylovNewton)
    //StaticIntegrator *theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0); // *
    //TransientIntegrator*
    theIntegrator = new Newmark(gamma, beta);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
    //ConstraintHandler*
    theHandler = new PenaltyConstraintHandler(1.0e14, 1.0e14);          // 1. constraints Penalty 1.0e15 1.0e15
    //ConstraintHandler * theHandler = new TransformationConstraintHandler(); // *
    //theHandler = new TransformationConstraintHandler(); // *
    //RCM *
    theRCM = new RCM();
    //DOF_Numberer *
    theNumberer = new DOF_Numberer(*theRCM);                                 // 4. numberer RCM (another option: Plain)
    //BandGenLinSolver *
    theSolver = new BandGenLinLapackSolver();                            // 5. system BandGeneral (TODO: switch to SparseGeneral)
    //LinearSOE *
    theSOE = new BandGenLinSOE(*theSolver);

    //DirectIntegrationAnalysis* theAnalysis;												   // 7. analysis    Transient
    theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

	//VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
	//theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

    //StaticAnalysis *theAnalysis; // *
    //theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator); // *

    theAnalysis->setConvergenceTest(*theTest);

    //doAnalysis = true;

	int converged;
	if(doAnalysis)
	{


    // transient
    converged = theAnalysis->analyze(10,1.0);
	if (!converged)
	{
		opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
	} else
	{
		opserr << "Didn't converge at time " << theDomain->getCurrentTime() << endln;
	}
	opserr << "Finished with elastic gravity analysis..." << endln << endln;


        /*
    // static
    for (int analysisCount = 0; analysisCount < 2; ++analysisCount) {
            //int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
            int converged = theAnalysis->analyze(1);
            if (!converged) {
                opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
            }
        }
        */



	}






	s << "# 3.2 plastic gravity analysis (transient)" << endln << endln;

    ElementIter &theElementIter1 = theDomain->getElements();
    while ((theEle = theElementIter1()) != 0)
    {
        int theEleTag = theEle->getTag();
        if(!eleTypeDict[theEleTag].compare("PM4Sand")
         || !eleTypeDict[theEleTag].compare("PM4Silt")
         || !eleTypeDict[theEleTag].compare("ManzariDafalias")
         || !eleTypeDict[theEleTag].compare("Elastic"))
        {
            Information stateInfo(1.0);
            theEle->updateParameter(5,stateInfo);
        } else if(!eleTypeDict[theEleTag].compare("PDMY")
                  || !eleTypeDict[theEleTag].compare("PDMY02")
                  || !eleTypeDict[theEleTag].compare("PIMY")
                  || !eleTypeDict[theEleTag].compare("J2Bounding"))
        {
            Information stateInfo(1);
            theEle->updateParameter(1,stateInfo);
        }
    }
	s << endln;

	for (int i=0; i != soilMatTags.size(); i++)
		s << "updateMaterialStage -material "<< soilMatTags[i] <<" -stage 1" << endln ; 

	// add parameters: FirstCall for plastic gravity analysis
	ElementIter &theElementIterFC = theDomain->getElements();
	int nParaPlus = 0;
	while ((theEle = theElementIterFC()) != 0)
	{

		int theEleTag = theEle->getTag();
        if(!eleTypeDict[theEleTag].compare("PM4Sand")
         || !eleTypeDict[theEleTag].compare("PM4Silt")
         || !eleTypeDict[theEleTag].compare("Elastic"))
        {
            Information myInfox(0);
            theEle->updateParameter(8,myInfox);
            s << "setParameter -value 0 -ele "<<theEleTag<<" FirstCall "<< matNumDict[theEleTag] << endln;
        }
	}

	// add parameters: poissonRatio for plastic gravity analysis
	ElementIter &theElementIter = theDomain->getElements();
	while ((theEle = theElementIter()) != 0)
	{
		int theEleTag = theEle->getTag();

        if(!eleTypeDict[theEleTag].compare("ManzariDafalias")
                || !eleTypeDict[theEleTag].compare("PM4Sand")
                || !eleTypeDict[theEleTag].compare("PM4Silt")
                || !eleTypeDict[theEleTag].compare("Elastic"))
        {
            Information myInfox(0.3);
            theEle->updateParameter(7,myInfox);

            //setParameter -value 0 -ele $elementTag poissonRatio $matTag
            s << "setParameter -value 0.3 -ele "<< theEleTag <<" poissonRatio "<< matNumDict[theEleTag] << endln;
        }
    }
    s << endln;



    if(doAnalysis)
    {
        converged = theAnalysis->analyze(10,1.0);

        if (!converged)
        {
            opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
        } else
        {
            opserr << "Didn't converge at time " << theDomain->getCurrentTime() << endln;
        }
        opserr << "Finished with plastic gravity analysis..." endln;
    }
    s << "analyze     10 1.0" << endln;
    s << "puts \"Finished with plastic gravity analysis...\"" << endln << endln;
	


	s << "# 3.3 Update element permeability for post gravity analysis"<< endln << endln;

	theElementIter = theDomain->getElements();
    char out[64];
	while ((theEle = theElementIter()) != 0)
	{
		int theEleTag = theEle->getTag();
		//setParameter -value 1 -ele $elementTag hPerm $matTag
        double thishPerm = -hPermVec[theEleTag-1]/g;
        double thisvPerm = -vPermVec[theEleTag-1]/g;

        // precision
        sprintf(out, "%.*g", 6, thishPerm);thishPerm = strtod(out, 0);
        sprintf(out, "%.*g", 6, thisvPerm);thisvPerm = strtod(out, 0);

        Information myInfox(thishPerm);
        theEle->updateParameter(3,myInfox);
        Information myInfoy(thisvPerm);
        theEle->updateParameter(4,myInfoy);

        s << "setParameter -value "<< std::setprecision(6) << thishPerm<<" -ele "<< theEleTag<<" hPerm "<<endln;
        s << "setParameter -value "<< std::setprecision(6) << thisvPerm<<" -ele "<< theEleTag<<" vPerm "<<endln;

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

    const int numberTheViscousMats = 1; // for 3D it's 2
    UniaxialMaterial* theViscousMats[numberTheViscousMats];

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


    s << "\n\n# 4.2 Create dashpot nodes and apply proper fixities. \n\n";

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
	



    s << "\n\n# 4.4 Remove fixities created for gravity. \n\n";

	for (int i_remove = 0; i_remove < sizeTheSPtoRemove; i_remove++)
	{
		theSP = theDomain->removeSP_Constraint(theSPtoRemove(i_remove));
		delete theSP;
	}
	// TODO:
	s << "remove sp 1 1" << endln;
	s << "remove sp 2 1" << endln;


    s << "\n\n# 4.5 Apply equalDOF for the first 4 nodes (3D) or 2 nodes (2D). \n\n";

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



    s << "\n\n# 4.6 Create the dashpot element. \n\n";

    Vector x(3);
    Vector y(3);
    x(0) = 1.0;
    x(1) = 0.0;  //|-----------> what???
    x(2) = 0.0;  //|-----------> I change the dimension of x and y from 3 to 1, and the results are still the same?
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
	

    //theDomain->Print(opserr);

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


    s << "\n\n# ------------------------------------------------------------\n";
	s << "# 5.1 Apply the rock motion                                    \n";
	s << "# ------------------------------------------------------------\n\n";
	int numSteps = 0;
    //std::vector<double> dt;


    double dT = 0.0005; // This is the time step in solution
    double motionDT = theMotionX->getDt();//  0.005; // This is the time step in the motion record. TODO: use a funciton to get it
    int nStepsMotion = theMotionX->getNumSteps();//1998;//theMotionX->getNumSteps() ; //1998; // number of motions in the record. TODO: use a funciton to get it
    int nSteps = int((nStepsMotion-1) * motionDT / dT);
    int remStep = nSteps;
	s << "set dT " << dT << endln;
	s << "set motionDT " << motionDT << endln;
    //s << "set mSeries \"Path -dt $motionDT -filePath /Users/simcenter/Codes/SimCenter/SiteResponseTool/test/RSN766_G02_000_VEL.txt -factor $cFactor\""<<endln;
    s << "set mSeries \"Path -dt $motionDT -filePath Rock-x.vel -factor $cFactor\""<<endln;

    // using a stress input with the dashpot
	if (theMotionX->isInitialized())
	{
        LoadPattern *theLP = new LoadPattern(10, vis_C);
        theLP->setTimeSeries(theMotionX->getVelSeries());

		NodalLoad *theLoad;
        int numLoads = 3; // for 3D it's 4
		Vector load(numLoads);
		load(0) = 1.0;
        load(1) = 0.0;
        load(2) = 0.0;
		//load(3) = 0.0;

		//theLoad = new NodalLoad(1, numNodes + 2, load, false); theLP->addNodalLoad(theLoad);
        theLoad = new NodalLoad(99999999, 1, load, false);
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
    //delete theIntegrator;
	//delete theAnalysis;

	//theTest->setTolerance(1.0e-5);

	s << "constraints Transformation" << endln; 
	s << "test NormDispIncr 1.0e-4 35 0" << endln; // TODO
	s << "algorithm   Newton" << endln;
	s << "numberer    RCM" << endln;
    s << "system SparseGeneral" << endln;//BandGeneral





	// create analysis objects - I use static analysis for gravity
	theModel = new AnalysisModel();
    theTest = new CTestNormDispIncr(1.0e-4, 35, 0);                    // 2. test NormDispIncr 1.0e-7 30 1
	theSolnAlgo = new NewtonRaphson(*theTest);                              // 3. algorithm   Newton (TODO: another option: KrylovNewton) 
	//StaticIntegrator *theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0); // *
    //ConstraintHandler *theHandler = new TransformationConstraintHandler(); // *
    // *
	//TransientIntegrator* theIntegrator = new Newmark(5./6., 4./9.);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
    //theIntegrator = new Newmark(0.5, 0.25);// * Newmark(0.5, 0.25)
    theHandler = new TransformationConstraintHandler();
    //theHandler = new PenaltyConstraintHandler(1.0e16, 1.0e16);          // 1. constraints Penalty 1.0e15 1.0e15
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
    //TransientIntegrator* theTransientIntegrator
    theTransientIntegrator = new Newmark(gamma_dynm, beta_dynm);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
	//theTransientIntegrator->setConvergenceTest(*theTest);

	// setup Rayleigh damping   TODO: calcualtion of these paras
	// apply 2% at the natural frequency and 5*natural frequency
    //double natFreq = SRM_layering.getNaturalPeriod();
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

    a0 = 0.787;
    a1 = 0.0007942;

	if (PRINTDEBUG)
	{
        //opserr << "f1 = " << natFreq << "    f2 = " << 5.0 * natFreq << endln;
		opserr << "a0 = " << a0 << "    a1 = " << a1 << endln;
	}
    theDomain->setRayleighDampingFactors(a0, a1, 0.0, 0.0);

    //DirectIntegrationAnalysis* theTransientAnalysis;
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


    double recDT = 0.001;

    std::string outFile;
    if(doAnalysis)
    {
        // Record the response at the surface
        outFile = theOutputDir + PATH_SEPARATOR + "surface.acc";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        outFile = theOutputDir + PATH_SEPARATOR + "surface.vel";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        outFile = theOutputDir + PATH_SEPARATOR + "surface.disp";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);
    }

    s << "file mkdir out_tcl" << endln;
    s << "set recDT " << recDT << endln;
    s<< "eval \"recorder Node -file out_tcl/surface.disp -time -dT $recDT -node "<<numNodes<<" -dof 1 2 3  disp\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/surface.acc -time -dT $recDT -node "<<numNodes<<" -dof 1 2 3  accel\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/surface.vel -time -dT $recDT -node "<<numNodes<<" -dof 1 2 3 vel\""<<endln;// 3


    if(doAnalysis)
    {
        // Record the response of base node
        nodesToRecord.resize(1);
        nodesToRecord(0) = 1;

        dofToRecord.resize(1);
        dofToRecord(0) = 0; dofToRecord(1) = 1; dofToRecord(2) = 2;

        outFile = theOutputDir + PATH_SEPARATOR + "base.acc";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        outFile = theOutputDir + PATH_SEPARATOR + "base.vel";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        outFile = theOutputDir + PATH_SEPARATOR + "base.disp";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);
    }



    s<< "eval \"recorder Node -file out_tcl/base.disp -time -dT $recDT -node 1 -dof 1 2 3  disp\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/base.acc -time -dT $recDT -node 1 -dof 1 2 3  accel\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/base.vel -time -dT $recDT -node 1 -dof 1 2 3 vel\""<<endln;// 3


    /*
	// Record pwp at node 17
	dofToRecord.resize(1);
	dofToRecord(0) = 2; // only record the pore pressure dof
	ID pwpNodesToRecord(1);
	pwpNodesToRecord(0) = 17;
	outFile = theOutputDir + PATH_SEPARATOR + "pwpLiq.out";
	theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
	theRecorder = new NodeRecorder(dofToRecord, &pwpNodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
	theDomain->addRecorder(*theRecorder);

    s<< "eval \"recorder Node -file out_tcl/pwpLiq.out -time -dT $recDT -node 17 -dof 3 vel\""<<endln;
    */


    if(doAnalysis)
    {
        // Record the response of all nodes
        nodesToRecord.resize(numNodes);
        for (int i=0;i<numNodes;i++)
            nodesToRecord(i) = i+1;
        dofToRecord.resize(2);
        dofToRecord(0) = 0;
        dofToRecord(1) = 1;

        outFile = theOutputDir + PATH_SEPARATOR + "displacement.out";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "disp", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        outFile = theOutputDir + PATH_SEPARATOR + "velocity.out";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        outFile = theOutputDir + PATH_SEPARATOR + "acceleration.out";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "accel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);

        dofToRecord.resize(1);
        dofToRecord(0) = 2;
        outFile = theOutputDir + PATH_SEPARATOR + "porePressure.out";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, recDT, true, NULL);
        theDomain->addRecorder(*theRecorder);
    }

    s<< "eval \"recorder Node -file out_tcl/displacement.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 1 2  disp\""<<endln;
    s<< "eval \"recorder Node -file out_tcl/velocity.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 1 2  vel\""<<endln;
    s<< "eval \"recorder Node -file out_tcl/acceleration.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 1 2  accel\""<<endln;
    s<< "eval \"recorder Node -file out_tcl/porePressure.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 3 vel\""<<endln;

    if(doAnalysis)
    {
        // Record element results
        OPS_Stream* theOutputStream2;
        ID elemsToRecord(quadElem.size());
        for (int i=0;i<quadElem.size();i+=1)
            elemsToRecord(i) = quadElem[i];
        //const char* eleArgs = "stress";
        const char *eleArgs[2];
        eleArgs[0] = "stress";
        eleArgs[1] = "3";

        outFile = theOutputDir + PATH_SEPARATOR + "stress.out";
        theOutputStream2 = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new ElementRecorder(&elemsToRecord, eleArgs, 2, true, *theDomain, *theOutputStream2, recDT, NULL);
        theDomain->addRecorder(*theRecorder);

        const char* eleArgsStrain = "strain";
        outFile = theOutputDir + PATH_SEPARATOR + "strain.out";
        theOutputStream2 = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new ElementRecorder(&elemsToRecord, &eleArgsStrain, 1, true, *theDomain, *theOutputStream2, recDT, NULL);
        theDomain->addRecorder(*theRecorder);
    }

    s<< "recorder Element -file out_tcl/stress.out -time -dT $recDT  -eleRange 1 "<<numQuadEles<<"  stress 3"<<endln;
    s<< "recorder Element -file out_tcl/strain.out -time -dT $recDT  -eleRange 1 "<<numQuadEles<<"  strain"<<endln;
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
	

    /*
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
    */


    s << "puts \"Start analysis\"" << endln;
    s << "set startT [clock seconds]" << endln;
    //s << "if {1} {" << endln;
    s << "set finalTime [expr $remStep * $dT]" << endln;
    s << "set success 0" << endln;
    s << "set currentTime 0." << endln;
    s << "set timeMarker 0." << endln;
    s << "while {$success == 0 && $currentTime < $finalTime} {" << endln;
    s << "	set subStep 0" << endln;
    s << "	set success [analyze 1  $dT]" << endln;
    s << "	if {$success != 0} {" << endln;
    s << "	set curTime  [getTime]" << endln;
    s << "	puts \"Analysis failed at $curTime . Try substepping.\"" << endln;
    s << "	set success  [subStepAnalyze [expr $dT/2.0] [incr subStep]]" << endln;
    s << "	set curStep  [expr int($curTime/$dT + 1)]" << endln;
    s << "	set remStep  [expr int($nSteps-$curStep)]" << endln;
    s << "	puts \"Current step: $curStep , Remaining steps: $remStep\"" << endln;
    s << "    } else {" << endln;
    s << "          set progress [expr $currentTime/$finalTime * 100.]" << endln;
    s << "          if { $progress > $timeMarker} {" << endln;
    s << "              set timeMarker [expr $timeMarker+2]" << endln;
    s << "              puts \"$progress%\"" << endln;
    s << "              }" << endln;
    s << "              set currentTime [getTime]" << endln;
    s << "	}" << endln;
    s << "}" << endln << endln;
    //s << "}" << endln << endln;


    /*
    s << "if {0} {" << endln;
    s << "while {$success != -10} {" << endln;
    s << "    set subStep 0" << endln;
    s << "    set success [analyze $remStep  $dT]" << endln;
    s << "    if {$success == 0} {" << endln;
    s << "        puts \"Analysis Finished\"" << endln;
    s << "        break" << endln;
    s << "    } else {" << endln;
    s << "        set curTime  [getTime]" << endln;
    s << "        puts \"Analysis failed at $curTime . Try substepping.\"" << endln;
    s << "        set success  [subStepAnalyze [expr $dT/2.0] [incr subStep]]" << endln;
    s << "        set curStep  [expr int($curTime/$dT + 1)]" << endln;
    s << "       set remStep  [expr int($nSteps-$curStep)]" << endln;
    s << "        puts \"Current step: $curStep , Remaining steps: $remStep\"" << endln;
    s << "    }" << endln;
    s << "}" << endln;
    s << "}" << endln;
    */



	s << "set endT [clock seconds]" << endln << endln;
	s << "puts \"loading analysis execution time: [expr $endT-$startT] seconds.\"" << endln << endln;
	s << "puts \"Finished with dynamic analysis...\"" << endln << endln;

	s << endln;
    //s << "print -file out_tcl/Domain.out" << endln << endln;
	
	s << "wipe" << endln;
    s << "puts \"Site response analysis is finished.\""<< endln;
	s << "exit" << endln << endln;

	s.close();
	ns.close();
	es.close();

    /*
    // write domain
    OPS_Stream* theOutputStreamAll;
    theOutputStreamAll = new DataFileStream("Domain.out", OVERWRITE, 2, 0, false, 6, false);
    theDomain->Print(*theOutputStreamAll);
    opserr << theOutputStreamAll;
    delete theOutputStreamAll;
    */


    m_dT = dT;
    m_nSteps = nSteps;
    m_remStep = remStep;


    //return 100;
    return trueRun();
}

int SiteResponseModel::trueRun()
{
    bool doAnalysis = m_doAnalysis;

    double dT = m_dT;
    int nSteps = m_nSteps;
    int remStep = m_remStep;

    if(doAnalysis)
    {

        bool useSubstep = true;
        int stepLag = 2;

        if(!useSubstep)
        {   // no substepping
            double totalTime = dT * nSteps;
            int success = 0;

            opserr << "Analysis started:" << endln;
            std::stringstream progressBar;
            for (int analysisCount = 0; analysisCount < remStep; ++analysisCount)
            {
                std::cout << "step: " << analysisCount << " / " << remStep-1 << endln;
                if (!forward) break;
                //int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
                //double stepDT = dt[analysisCount];
                //int converged = theTransientAnalysis->analyze(1, stepDT, stepDT / 2.0, stepDT * 2.0, 1); // *
                //int converged = theTransientAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
                int converged = theTransientAnalysis->analyze(1, dT);
                if (!converged)
                {
                    std::cout << "Converged at time " << theDomain->getCurrentTime() << endln;
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

                        if (callback && forward)
                            forward = m_callbackFunction(100.0 * analysisCount / remStep);
                    }
                }
                else
                {
                    std::cout << "Site response analysis did not converge. Trying substepping..." << endln;
                    opserr << "Site response analysis did not converge. Trying substepping..." << endln;

                    int subStep = 0;
                    success = subStepAnalyze(dT/2., subStep+1, theTransientAnalysis);
                    if(!fabs(success)<1)
                    {
                        std::cout << "Substepping didn't work... exit" << endln;
                        opserr << "Substepping didn't work... exit" << endln;
                        exit(-1);
                    }
                }
            }
            opserr << "Site response analysis done..." << endln;
            if (callback && forward) m_callbackFunction(100.0);
            progressBar << "\r[";
            for (int ii = 0; ii < 20; ii++)
                progressBar << "-";

            progressBar << "]  🚌   100%\n";
            opsout << progressBar.str().c_str();
            opsout.flush();
            opsout << endln;
        } else { // substep
            opserr << "Analysis started (substepping):" << endln;
            double finalTime = dT * remStep;
            int success = 0;
            double currentTime = 0.;
            std::stringstream progressBar;
            int currentProgress = 0;
            int remStept = 0;
            double timeMaker = 0.;

            while(fabs(success)<1 && currentTime < finalTime && forward)
            {

                int subStep = 0;
                success = theTransientAnalysis->analyze(1, dT);
                std::cout << "current time is: " << currentTime << " \n";
                if(fabs(success)>0)
                {   // analysisi failed at currenttime
                    std::cout << "analysisi failed at time: " << currentTime << ". Try substepping ... \n";
                    success = subStepAnalyze(dT/2., subStep+1, theTransientAnalysis);
                } else {
                    currentProgress = int(currentTime/finalTime *100.);
                    if (currentProgress > timeMaker)
                    {
                        timeMaker += 1;
                        //std::cout << currentProgress << endln;
                        remStept = 100-currentProgress;
                        if (currentProgress % stepLag == 0 && currentProgress > stepLag)
                        {
                            progressBar << "\r[";
                            for (int ii = 0; ii < currentProgress/stepLag; ii++)
                                progressBar << "-";
                            progressBar << " 🚌  ";
                            for (int ii = 0; ii < remStept/stepLag; ii++)
                                progressBar << ".";

                            progressBar << "]  " << currentProgress << "%";
                            opsout << progressBar.str().c_str();
                            opsout.flush();

                            if (callback && forward)
                                forward = m_callbackFunction(double(currentTime/finalTime *100.));
                        }
                    }

                    currentTime = theDomain->getCurrentTime();
                }
            }


            opserr << "Site response analysis done..." << endln;
            if (callback && forward) m_callbackFunction(100.0);
            progressBar << "\r[";
            for (int ii = 0; ii < 100/stepLag; ii++)
                progressBar << "-";

            progressBar << "]  🚌   100%\n";
            opsout << progressBar.str().c_str();
            opsout.flush();
            opsout << endln;

        }

    } else{
        std::cout << "tcl file built." << std::endl;
    }

    theDomain->removeRecorders();

    /*
    // write domain
    OPS_Stream* theOutputStreamAll;
    theOutputStreamAll = new DataFileStream("/Users/simcenter/Codes/SimCenter/s3hark/bin/s3hark.out", OVERWRITE, 2, 0, false, 6, false);
    theDomain->Print(*theOutputStreamAll);
    opserr << theOutputStreamAll;
    delete theOutputStreamAll;
    */

    return 100;

}


int SiteResponseModel::subStepAnalyze(double dT, int subStep, DirectIntegrationAnalysis* theTransientAnalysis)
{
    int maxsubstep = 10;
    if (subStep > maxsubstep)
    {
        std::cout << "subStep greater than max substeps " << maxsubstep << "existing substepping. \n";
        return -maxsubstep;
    }
    int success = 0;
    for (int i=1; i < 3; i++)
	{
		opserr << "Try dT = " << dT << endln;
        success = theTransientAnalysis->analyze(1, dT);// 0 means success
        //success = subStepAnalyze(dT/2, subStep +1,);
        if(fabs(success) > 0.0 )
        {
            success = subStepAnalyze(dT/2.0, subStep+1,theTransientAnalysis);
            if(success == -maxsubstep)
            {
                std::cout << "reached max substeps " << maxsubstep << "existing substepping. \n";
                return success;
            }
        } else {
            if (i==1)
                opserr << "Substep " << subStep << " : Left side converged with dT = " << dT;
            else
                opserr << "Substep " << subStep << " : Right side converged with dT = " << dT;
        }
	}
	
    return success;

}


int SiteResponseModel::buildEffectiveStressModel3D(bool doAnalysis)
{
    m_doAnalysis = doAnalysis;

    Vector zeroVec(3);
    zeroVec.Zero();

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
    //std::string configFile = "SRT.json";
    std::ifstream i(theConfigFile);
    if(!i)
        return false;// failed to open SRT.json TODO: print to log
    json SRT;
    i >> SRT;

    // set outputs for tcl
    //ofstream s ("/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/model.tcl", std::ofstream::out);
    /*
    ofstream s ("model.tcl", std::ofstream::out);
    ofstream ns ("out_tcl/nodesInfo.dat", std::ofstream::out);
    ofstream es ("out_tcl/elementInfo.dat", std::ofstream::out);
    */
    ofstream s (theAnalysisDir + "/model.tcl", std::ofstream::out);//TODO: may not work on windows
    s.precision(16);
    ofstream ns (theTclOutputDir+"/nodesInfo.dat", std::ofstream::out);
    ofstream es (theTclOutputDir+"/elementInfo.dat", std::ofstream::out);
    ofstream esmat3D (theTclOutputDir+"/elementMatInfo3D.dat", std::ofstream::out);
    //ofstream s ("/Users/simcenter/Codes/SimCenter/build-SiteResponseTool-Desktop_Qt_5_11_1_clang_64bit-Debug/SiteResponseTool.app/Contents/MacOS/model.tcl", std::ofstream::out);
    s << "# #########################################################" << "\n\n";
    s << "wipe \n\n";

    s << "set g " << g << endln;
    s << "set pi " << std::setprecision(22)  << pi << endln;


    // basic settings
    int numLayers = 0;
    int numNodes = 0;
    int numElems = 0;
    double totalHeight = 0.0;
    double sElemX = 0.0;
    double slopex1 = 0.0;
    double slopex2 = 0.0;
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
        sElemX = 1.0;  // force element size to be 1.0 in the horizontal direction
        slopex1 = basicSettings["slopex1"];
        slopex2 = basicSettings["slopex2"];
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

    s << "set slopex1 " << slopex1 << endln;
    s << "set slopex2 " << slopex2 << endln;


    s << "\n# ------------------------------------------ \n";
    s << "# 1. Build nodes and elements                \n";
    s << "# ------------------------------------------ \n \n";
    double yCoord = 0;
    double zthick = colThickness;
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
    std::vector<double> vPermVec;
    std::vector<double> hPermVec;

    // TODO: ndof is 2 or 3? 
    // 2D: 3   3D: 4
    theNode = new Node(numNodes + 1, 4, 0.0, yCoord, 0.0); theDomain->addNode(theNode);
    theNode = new Node(numNodes + 2, 4, 0.0, yCoord, zthick); theDomain->addNode(theNode);
    theNode = new Node(numNodes + 3, 4, sElemX, yCoord, zthick); theDomain->addNode(theNode);
    theNode = new Node(numNodes + 4, 4, sElemX, yCoord, 0.0); theDomain->addNode(theNode);



    s << "model BasicBuilder -ndm 3 -ndf 4  \n\n";
    s << "node " << numNodes + 1 << " 0.0 " << yCoord << " 0.0 " << endln;
    s << "node " << numNodes + 2 << " 0.0 " << yCoord << " " << zthick << " " << endln;
    s << "node " << numNodes + 3 << " " << sElemX << " " << yCoord << " " << zthick << " " << endln;
    s << "node " << numNodes + 4 << " " << sElemX << " " << yCoord << " 0.0 " << endln;


    ns << numNodes + 1 << " 0.0 " << yCoord << " 0.0 " << endln;
    ns << numNodes + 2 << " 0.0 " << yCoord << " " << zthick << " " << endln;
    ns << numNodes + 3 << " " << sElemX << " " << yCoord << " " << zthick << " " << endln;
    ns << numNodes + 4 << " " << sElemX << " " << yCoord << " 0.0 "  << endln;

    std::map<int, std::string> eleTypeDict;

    s << std::scientific << std::setprecision(14);

    numNodes += 4;
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
            if (eSizeV<minESizeV) {
                //std::string err = "eSize is tool small. change it in the json file.";throw err;
                eSizeV = minESizeV;
            }
            double thickness = l["thickness"];
            double vs = l["vs"];
            double Dr = l["Dr"];
            double vPerm = l["vPerm"];
            double hPerm = l["hPerm"];
            double uBulk = l["uBulk"];
            double evoid = l["void"];
            std::string color = l["color"];
            std::string lname = l["name"];
            if (!lname.compare("Rock"))
            {
                //rockVs = vs;
                //rockDen = l["density"];
                continue;
            }

            //double evoid = 0.0;
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
                //evoid  = emax - Dr * (emax - emin);
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

                //evoid  = emax - thisDr * (emax - emin);
                rho_d = Gs / (1 + evoid);
                rho_s = rho_d *(1.0+evoid/Gs);

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                theMat = new PM4Sand(matTag, thisDr,G0,hpo,thisDen,P_atm,h0,emax,emin,nb,nd,Ado,z_max,cz,ce,phic,nu,cgd,cdr,ckaf,Q,R,m,Fsed_min,p_sedo);
                s << "nDMaterial PM4Sand " << matTag<< " " << thisDr<< " " <<G0<< " " <<hpo<< " " <<thisDen<< " " <<P_atm<< " " <<h0<< " "<<emax<< " "<<emin<< " " <<
                nb<< " " <<nd<< " " <<Ado<< " " <<z_max<< " " <<cz<< " " <<ce<< " " <<phic<< " " <<nu<< " " <<cgd<< " " <<cdr<< " " <<ckaf<< " " <<
                Q<< " " <<R<< " " <<m<< " " <<Fsed_min<< " " <<p_sedo << endln;

            }else if(!matType.compare("PM4Silt"))
            {
                double thisDr = mat["Dr"];
                double S_u = mat["S_u"];
                double Su_Rat = mat["Su_Rat"];
                double G_o = mat["G_o"];
                double h_po = mat["h_po"];
                double thisDen = mat["Den"];

                double Su_factor = mat["Su_factor"];
                double P_atm = mat["P_atm"];
                double nu = mat["nu"];
                double nG = mat["nG"];
                double h0 = mat["h0"];
                double eInit = mat["eInit"];
                double lambda = mat["lambda"];
                double phicv = mat["phicv"];
                double nb_wet = mat["nb_wet"];
                double nb_dry = mat["nb_dry"];
                double nd = mat["nd"];
                double Ado = mat["Ado"];
                double ru_max = mat["ru_max"];
                double z_max = mat["z_max"];
                double cz = mat["cz"];
                double ce = mat["ce"];
                double cgd = mat["cgd"];
                double ckaf = mat["ckaf"];
                double m_m = mat["m_m"];
                double CG_consol = mat["CG_consol"];

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                theMat = new PM4Silt(matTag, S_u, Su_Rat, G_o, h_po, thisDen, Su_factor, P_atm,nu, nG, h0, eInit, lambda, phicv, nb_wet, nb_dry, nd, Ado, ru_max, z_max,cz, ce, cgd, ckaf, m_m, CG_consol);
                s << "nDMaterial PM4Silt " << matTag<< " " << S_u<< " " <<Su_Rat<< " " <<G_o<< " " <<h_po<< " " <<thisDen<< " "
                  <<Su_factor<< " " <<P_atm<< " " <<nu<< " " <<nG<< " " <<h0<< " " <<eInit<< " " <<lambda<< " " <<phicv<< " "
                 <<nb_wet<< " " <<nb_dry<< " " <<nd<< " " <<Ado<< " " <<ru_max<< " " <<z_max<< " " <<cz<< " " <<ce<< " " <<cgd
                << " " <<ckaf<< " " <<m_m<< " " <<CG_consol << endln;

            }else if(!matType.compare("PIMY"))
            {
                double thisDr = mat["Dr"];
                int nd = 3;//mat["nd"];
                double rho = mat["rho"];
                double refShearModul = mat["refShearModul"];
                double refBulkModul = mat["refBulkModul"];
                double cohesi = mat["cohesi"];
                double peakShearStra = mat["peakShearStra"];

                double frictionAng = mat["frictionAng"];
                double refPress = mat["refPress"];
                double pressDependCoe = mat["pressDependCoe"];
                double noYieldSurf = mat["noYieldSurf"];

                double emax = 0.8;
                double emin = 0.5;
                //evoid  = emax - Dr * (emax - emin);

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                //TODO: PM4Silt->PIMY
                //TODO: deal with noYieldSurf
                theMat = new PressureIndependMultiYield(matTag,nd,rho,refShearModul,refBulkModul,cohesi,peakShearStra,
                                                        frictionAng, refPress,  pressDependCoe);
                s << "nDMaterial PressureIndependMultiYield "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "<<refBulkModul<<" "<<cohesi<<" "<<peakShearStra<<" "<<
                        frictionAng<<" "<< refPress<<" "<<pressDependCoe<<endln;
            }else if(!matType.compare("PDMY"))
            {

                double thisDr = mat["Dr"];
                int nd = 3;//mat["nd"];
                double rho = mat["rho"];
                double refShearModul = mat["refShearModul"];
                double refBulkModul = mat["refBulkModul"];
                double frictionAng = mat["frictionAng"];
                double peakShearStra = mat["peakShearStra"];

                double refPress = mat["refPress"];
                double pressDependCoe = mat["pressDependCoe"];
                double PTAng = mat["PTAng"];
                double contrac = mat["contrac"];
                double dilat1 = mat["dilat1"];
                double dilat2 = mat["dilat2"];
                double liquefac1 = mat["liquefac1"];
                double liquefac2 = mat["liquefac2"];
                double liquefac3 = mat["liquefac3"];
                double e = mat["e"];
                double cs1 = mat["cs1"];
                double cs2 = mat["cs2"];
                double cs3 = mat["cs3"];
                double pa = mat["pa"];
                double c = mat["c"];
                double noYieldSurf = mat["noYieldSurf"];

                double emax = 0.8;
                double emin = 0.5;
                //evoid  = emax - Dr * (emax - emin);

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                //TODO: PM4Silt->PDMY
                //TODO: deal with noYieldSurf
                /*
                theMat = new PressureDependMultiYield(matTag,nd,rho,refShearModul,refBulkModul,frictionAng,
                                                      peakShearStra, refPress,pressDependCoe,PTAng,contrac,dilat1,dilat2,liquefac1,liquefac2,liquefac3,
                                                      20,0,e,cs1,cs2,cs3,pa,c);
                s << "nDMaterial PressureDependMultiYield "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "<<refBulkModul<<" "<<frictionAng<<" "<<peakShearStra<<" "<<
                        refPress<<" "<<pressDependCoe<<" "<<PTAng<<" "<<contrac<<" "<<dilat1<<" "<<dilat2<<" "<<liquefac1<<" "<<liquefac2<<" "<<liquefac3
                          <<" 20 "<<e<<" "<<cs1<<" "<<cs2<<" "<<cs3<<" "<<pa<<" "<<c<<endln;
                */
                theMat = new PressureDependMultiYield(matTag,nd,rho,refShearModul,refBulkModul,frictionAng,peakShearStra,
                                                      refPress,pressDependCoe,PTAng,contrac,dilat1,dilat2,liquefac1,liquefac2,liquefac3,20,0,
                                                      e, cs1,cs2,cs3,pa,c);
                s << "nDMaterial PressureDependMultiYield "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "<<refBulkModul<<" "<<frictionAng<<" "<<peakShearStra<<" "<<
                        refPress<<" "<<pressDependCoe<<" "<<PTAng<<" "<<contrac<<" "<<dilat1<<" "<<dilat2<<" "<<liquefac1<<" "<<liquefac2<<" "<<liquefac3 << " " << "20"
                          <<" "<<e<<" "<<cs1<<" "<<cs2<<" "<<cs3<<" "<<pa<<" "<<c <<endln;

            }else if(!matType.compare("PDMY02"))
            {

                double emax = 0.8;
                double emin = 0.5;
                //evoid  = emax - Dr * (emax - emin);

                double thisDr = mat["Dr"];
                int nd = 3;//mat["nd"];
                double rho = mat["rho"];
                double refShearModul = mat["refShearModul"];
                double refBulkModul = mat["refBulkModul"];
                double frictionAng = mat["frictionAng"];
                double peakShearStra = mat["peakShearStra"];

                double refPress = mat["refPress"];
                double pressDependCoe = mat["pressDependCoe"];
                double PTAng = mat["PTAng"];
                double contrac1 = mat["contrac1"];
                double contrac3 = mat["contrac3"];
                double dilat1 = mat["dilat1"];
                double dilat3 = mat["dilat3"];
                double contrac2 = mat["contrac2"];
                double dilat2 = mat["dilat2"];
                double liquefac1 = mat["liquefac1"];
                double liquefac2 = mat["liquefac2"];
                double e = mat["e"];
                double cs1 = mat["cs1"];
                double cs2 = mat["cs2"];
                double cs3 = mat["cs3"];
                double pa = mat["pa"];
                double c = mat["c"];
                double noYieldSurf = mat["noYieldSurf"];

                //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                //TODO: PM4Silt->PDMY02
                //TODO: deal with noYieldSurf
                theMat = new PressureDependMultiYield02(matTag,nd,rho,refShearModul,refBulkModul,frictionAng,
                       peakShearStra, refPress,  pressDependCoe,PTAng,contrac1,contrac3,  dilat1,dilat3,20,0,
                              contrac2, dilat2,liquefac1,liquefac2,e,cs1,cs2,cs3,pa);
                /*
                theMat = new PressureDependMultiYield02(matTag,nd,rho,refShearModul,refBulkModul,frictionAng,
                       peakShearStra, refPress,  pressDependCoe,PTAng,contrac1,contrac3,  dilat1,dilat3,20);
                */
                s << "nDMaterial PressureDependMultiYield02 "<<matTag << " "<<nd<<" "<<rho<<" "<<refShearModul<<" "
                  <<refBulkModul<<" "<<frictionAng<<" "<<peakShearStra<<" "<<refPress<<" "<<pressDependCoe<<" "
                 <<PTAng<<" "<<contrac1<<" "<<contrac3<<" "<<dilat1<<" "<<dilat3<<" 20 "<<contrac2<<" "<<dilat2
                 <<" "<<liquefac1<<" "<<liquefac2<<" "<<e<<" "<<cs1<<" "<<cs2<<" "<<cs3<<" "<<pa<<" "<<endln;
            }
            else if(!matType.compare("ManzariDafalias"))
                        {

                            double emax = 0.8;
                            double emin = 0.5;
                            //evoid  = emax - Dr * (emax - emin);

                            double Dr = mat["Dr"];
                            double G0 = mat["G0"];
                            double nu = mat["nu"];
                            double e_init = mat["e_init"];
                            double Mc = mat["Mc"];
                            double c = mat["c"];

                            double lambda_c = mat["lambda_c"];
                            double e0 = mat["e0"];
                            double ksi = mat["ksi"];
                            double P_atm = mat["P_atm"];
                            double m = mat["m"];
                            double h0 = mat["h0"];
                            double ch = mat["ch"];
                            double nb = mat["nb"];
                            double A0 = mat["A0"];
                            double nd = mat["nd"];
                            double z_max = mat["z_max"];
                            double cz = mat["cz"];
                            double Den = mat["Den"];

                            //theMat = new ElasticIsotropicMaterial(matTag, 20000.0, 0.3, thisDen);
                            //TODO: PM4Silt->ManzariDafalias
                            theMat = new ManzariDafalias(matTag, G0, nu, e_init, Mc, c, lambda_c, e0, ksi, P_atm, m, h0, ch, nb, A0, nd, z_max, cz, Den);
                            s << "nDMaterial ManzariDafalias " << matTag<< " " << G0<< " " <<nu<< " " <<e_init<< " " <<Mc<< " " <<c<< " " <<lambda_c<< " " <<e0<< " " <<ksi<< " " <<P_atm<< " " <<m<< " " <<h0<< " " <<ch<< " " <<nb<< " " <<A0<< " " <<nd<< " " <<z_max<< " " <<cz<< " " <<Den << endln;

            }
            else if(!matType.compare("J2Bounding"))
                        {
                            double emax = 0.8;
                            double emin = 0.5;
                            //evoid  = emax - Dr * (emax - emin);

                            double Dr = mat["Dr"];
                            double G = mat["G"];
                            double K = mat["K"];
                            double su = mat["su"];
                            double rho = mat["rho"];
                            double h = mat["h"];
                            double m = mat["m"];
                            double k_in = mat["k_in"];
                            double beta = mat["beta"];

                            double h0 = 0.0;


                            // new J2
                            //TODO: k_in -> chi  ?
                            theMat = new J2CyclicBoundingSurface(matTag, G, K, su, rho, h, m,h0, k_in, beta);
                            s << "nDMaterial J2CyclicBoundingSurface " << matTag<< " " << G<< " " <<K<< " "
                              <<su<< " " <<rho<< " " <<h<< " " <<m<< " "<< h0 <<" " <<k_in<< " " <<beta << endln;

                            /*
                            theMat = new J2CyclicBoundingSurface(matTag, G, K, su, rho, h, m, k_in, beta);
                            s << "nDMaterial J2CyclicBoundingSurface " << matTag<< " " << G<< " " <<K<< " "
                              <<su<< " " <<rho<< " " <<h<< " " <<m << " "<< h0 << " " <<k_in<< " " <<beta << endln;
                            */
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
                theNode = new Node(numNodes + 1, 4, 0.0, yCoord, 0.0);
                theDomain->addNode(theNode);
                theNode = new Node(numNodes + 2, 4, 0.0, yCoord, zthick);
                theDomain->addNode(theNode);
                theNode = new Node(numNodes + 3, 4, sElemX, yCoord, zthick);
                theDomain->addNode(theNode);
                theNode = new Node(numNodes + 4, 4, sElemX, yCoord, 0.0);
                theDomain->addNode(theNode);


                /*
                s << "node " << numNodes + 1 << " 0.0 " << yCoord << endln;
                s << "node " << numNodes + 2 << " " << sElemX << " " << yCoord << endln;
                ns << numNodes + 1 << " 0.0 " << yCoord << endln;
                ns << numNodes + 2 << " " << sElemX << " " << yCoord << endln;
                */


                s << "node " << numNodes + 1 << " 0.0 " << yCoord << " 0.0 " << endln;
                s << "node " << numNodes + 2 << " 0.0 " << yCoord << " " << zthick << " " << endln;
                s << "node " << numNodes + 3 << " " << sElemX << " " << yCoord << " " << zthick << " " << endln;
                s << "node " << numNodes + 4 << " " << sElemX << " " << yCoord << " 0.0 " << endln;


                ns << numNodes + 1 << " 0.0 " << yCoord << " 0.0 " << endln;
                ns << numNodes + 2 << " 0.0 " << yCoord << " " << zthick << " " << endln;
                ns << numNodes + 3 << " " << sElemX << " " << yCoord << " " << zthick << " " << endln;
                ns << numNodes + 4 << " " << sElemX << " " << yCoord << " 0.0 "  << endln;





                double alpha = 1.5e-6; // seems different from 2D
                /*
                s << "element SSPbrickUP "<<numElems + 1<<" "
                    <<numNodes - 3 <<" "<<numNodes - 2<<" "<< numNodes +2<<" "<< numNodes+1 <<" "
                      <<numNodes <<" "<<numNodes-1<<" "<< numNodes + 3<<" "<< numNodes + 4<<" "
                    << theMat->getTag() << " " <<uBulk<< " 1.0 "<<" 1.0 1.0 1.0 " <<evoid << " "<< alpha<< " ";
                    */
/*
                double k1 = -hPerm/g; // default 1.0
                double k2 = -vPerm/g; // default 1.0
                double k3 = -hPerm/g; // default 1.0
*/


                double k1 = 1.0; // default 1.0
                double k2 = 1.0; // default 1.0
                double k3 = 1.0; // default 1.0



                s << "element SSPbrickUP "<<numElems + 1<<" "
                    <<numNodes - 3 <<" "<<numNodes-2 <<" "<< numNodes -1<<" "<< numNodes <<" "
                      <<numNodes+1 <<" "<<numNodes+2<<" "<< numNodes + 3<<" "<< numNodes + 4<<" "
                    << theMat->getTag() << " " <<uBulk<< " 1.0 "<<k1<<" "<<k2<<" "<<k3 <<" "<<evoid << " "<< alpha<< " ";


                double epsilon = 1.0e-19;
                double b1 = 0.0, b2 = 0.0, b3 = 0.0;
                if (fabs(-1.0*sin(slopex1*pi/180.) * cos(slopex2*pi/180.) * g)<epsilon)
                    s << std::setprecision(7) << 0.0 <<" ";
                else {
                b1 = -1.0*sin(slopex1*pi/180.) * cos(slopex2*pi/180.) * g;
                s << std::setprecision(7) << b1 <<" ";}

                if (fabs(cos(slopex1*pi/180.) * g)<epsilon)
                    s << std::setprecision(7) << 0.0 <<" ";
                else {
                    b2 = cos(slopex1*pi/180.) * g;
                    s << std::setprecision(7) << b2 <<" ";}

                if (fabs(-1.0*sin(slopex1*pi/180.)*sin(slopex2*pi/180.) * g)<epsilon)
                    s << std::setprecision(7) << 0.0 <<endln;
                else {
                    b3 = -1.0*sin(slopex1*pi/180.)*sin(slopex2*pi/180.) * g ;
                    s << std::setprecision(7) << b3 <<endln;}

                theEle = new SSPbrickUP(numElems + 1,
                                        numNodes - 3, numNodes - 2, numNodes - 1, numNodes ,
                                        numNodes + 1, numNodes + 2, numNodes + 3, numNodes + 4,
                                       *theMat, uBulk, 1.0, k1, k2, k3, evoid, alpha,
                                        b1, b2 ,b3); // -9.81 * theMat->getRho() TODO: theMat->getRho()
                // TODO: use SSPbrickUP
                //theEle = new SSPquadUP(numElems + 1, numNodes - 1, numNodes, numNodes + 2, numNodes + 1,
                //                       *theMat, 1.0, uBulk, 1.0, 1.0, 1.0, evoid, alpha, 0.0, g * 1.0); // -9.81 * theMat->getRho() TODO: theMat->getRho()
                hPermVec.push_back(hPerm);
                vPermVec.push_back(vPerm);


                /*
                double epsilon = 1.0e-19;

                if (fabs(-1.0*sin(slopex1*pi/180.) * cos(slopex2*pi/180.) * g)<epsilon)
                    s << std::setprecision(16) << 0.0 <<" ";
                else s << "[expr -1.0*sin($slopex1*$pi/180.) * cos($slopex2*$pi/180.) * $g]" <<" ";

                if (fabs(cos(slopex1*pi/180.) * g)<epsilon)
                    s << std::setprecision(16) << 0.0 <<" ";
                else s <<  "[expr cos($slopex1*$pi/180.) * $g]" <<" ";

                if (fabs(-1.0*sin(slopex1*pi/180.)*sin(slopex2*pi/180.) * g)<epsilon)
                    s << std::setprecision(16) << 0.0 <<endln;
                else s << "[expr -1.0*sin($slopex1*$pi/180.)*sin($slopex2*$pi/180.) * $g]" <<endln;
                */



                //s << std::setprecision(9) << std::to_string(-1.0*sin(slopex1*pi/180.) * cos(slopex2*pi/180.) * g) <<" "<< std::to_string(cos(slopex1*pi/180.) * g) <<" " << std::to_string(-1.0*sin(slopex1*pi/180.)*sin(slopex2*pi/180.) * g) << endln;
                //s << std::setprecision(16) << -1.0*sin(slopex1*pi/180.) * cos(slopex2*pi/180.) * g <<" "<< cos(slopex1*pi/180.) * g <<" " << -1.0*sin(slopex1*pi/180.)*sin(slopex2*pi/180.) * g << endln;
                //s << std::setprecision(16) << -1.0*sin(atan(slopex1)) * cos(atan(slopex2)) * g <<" "<< cos(atan(slopex1)) * g <<" " << -1.0*sin(atan(slopex1))*sin(atan(slopex2)) * g << endln;

                /*
                es << numElems + 1<<" " <<numNodes - 3 <<" "<< numNodes - 2<<" "<< numNodes +2<<" "<<numNodes+1<<" "
                     <<numNodes  <<" "<<numNodes-1<<" "<< numNodes + 3<<" "<< numNodes + 4<<" "
                    << theMat->getTag() << endln;
                */
                es << numElems + 1<<" " <<numNodes - 3 <<" "<< numNodes-2 <<" "<< numNodes -1<<" "<<numNodes<<" "
                     <<numNodes+1  <<" "<<numNodes+2<<" "<< numNodes + 3<<" "<< numNodes + 4<<" "
                    << theMat->getTag() << endln;
                esmat3D << numElems + 1 << " " << matType << endln;

                theDomain->addElement(theEle);
/*
                // TODO: not sure...
                theParameter = new Parameter(numElems + 1, 0, 0, 0);
                sprintf(paramArgs[1], "%d", theMat->getTag());
                theEle->setParameter(const_cast<const char**>(paramArgs), 2, *theParameter);
                theDomain->addParameter(theParameter);
*/
                matNumDict[numElems + 1] = theMat->getTag();

                eleTypeDict[numElems + 1] = matType;

                if (yCoord >= (totalHeight - groundWaterTable))
                { 	//record dry nodes above ground water table
                    dryNodes.push_back(numNodes + 1);
                    dryNodes.push_back(numNodes + 2);
                    dryNodes.push_back(numNodes + 3);
                    dryNodes.push_back(numNodes + 4);
                }
                numNodes += 4;
                numElems += 1;
            }
            std::cout << "layer tag: " << lTag << std::endl;
        }

        if (0 >= (totalHeight - groundWaterTable))
        { 	//record dry nodes above ground water table
            dryNodes.push_back(1);
            dryNodes.push_back(2);
            dryNodes.push_back(3);
            dryNodes.push_back(4);
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
    int sizeTheSPtoRemove = 2 ; // for 3D it's 2; for 2D it's 2
    ID theSPtoRemove(sizeTheSPtoRemove); // these fixities should be removed later on if compliant base is used

    theSP = new SP_Constraint(1, 0, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    theSPtoRemove(0) = theSP->getTag();

    theSP = new SP_Constraint(1, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);

    theSP = new SP_Constraint(1, 2, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    theSPtoRemove(1) = theSP->getTag();

    s << "fix 1 1 1 1 0" << endln;

    theSP = new SP_Constraint(2, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);

    s << "fix 2 0 1 0 0" << endln << endln;

    theSP = new SP_Constraint(3, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);

    s << "fix 3 0 1 0 0" << endln;

    theSP = new SP_Constraint(4, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);

    s << "fix 4 0 1 0 0" << endln << endln;

    MP_Constraint *theMP_base;
    int crrdim_base = 2 ;//For 3D it's 3; for 2d, it's 2
    Matrix Ccr_base(crrdim_base, crrdim_base);
    ID rcDOF_base(crrdim_base);
    Ccr_base(0, 0) = 1.0;
    Ccr_base(1, 1) = 1.0;
    rcDOF_base(0) = 0;
    rcDOF_base(1) = 2;
    theMP_base = new MP_Constraint(1, 2, Ccr_base, rcDOF_base, rcDOF_base); theDomain->addMP_Constraint(theMP_base);
    theMP_base = new MP_Constraint(1, 3, Ccr_base, rcDOF_base, rcDOF_base); theDomain->addMP_Constraint(theMP_base);
    theMP_base = new MP_Constraint(1, 4, Ccr_base, rcDOF_base, rcDOF_base); theDomain->addMP_Constraint(theMP_base);

    s << "equalDOF  1 2 1 3" << endln ;
    s << "equalDOF  1 3 1 3" << endln ;
    s << "equalDOF  1 4 1 3" << endln << endln;

    s << "# 2.2 Apply periodic boundary conditions    \n\n";
    MP_Constraint *theMP;
    int crrdim = 3 ;//For 3D it's 3; for 2d, it's 2
    Matrix Ccr(crrdim, crrdim);
    ID rcDOF(crrdim);
    // TODO: get clarified about the dimensions of Crr and rfDOF
    // Confirmed for 3D
    Ccr(0, 0) = 1.0;
    Ccr(1, 1) = 1.0;
    Ccr(2, 2) = 1.0;
    rcDOF(0) = 0;
    rcDOF(1) = 1;
    rcDOF(2) = 2;
    for (int nodeCount = 4; nodeCount < numNodes; nodeCount += 4)
    {
        theMP = new MP_Constraint(nodeCount + 1, nodeCount + 2, Ccr, rcDOF, rcDOF);
        theDomain->addMP_Constraint(theMP);
        theMP = new MP_Constraint(nodeCount + 1, nodeCount + 3, Ccr, rcDOF, rcDOF);
        theDomain->addMP_Constraint(theMP);
        theMP = new MP_Constraint(nodeCount + 1, nodeCount + 4, Ccr, rcDOF, rcDOF);
        theDomain->addMP_Constraint(theMP);
        s << "equalDOF " << nodeCount + 1 << " "<< nodeCount + 2 << " 1 2 3" << endln;
        s << "equalDOF " << nodeCount + 1 << " "<< nodeCount + 3 << " 1 2 3" << endln;
        s << "equalDOF " << nodeCount + 1 << " "<< nodeCount + 4 << " 1 2 3" << endln;
    }
    s << "\n\n";



    s << "# 2.3 Apply pore pressure boundaries for nodes above water table. \n\n";
    for (int i = 0; i < dryNodes.size(); i++)
    {
        theSP = new SP_Constraint(dryNodes[i], 3, 0.0, true);
        theDomain->addSP_Constraint(theSP);
        s << "fix " << dryNodes[i] << " 0 0 0 1" << endln;
    }
    s << "\n\n";



    /*
    if (theModelType.compare("2D")) // 3D
    {
        theViscousMats[1] = new ViscousMaterial(numLayers + 20, vis_C, 1.0);
        OPS_addUniaxialMaterial(theViscousMats[1]);
        // TODO: s <<
    }
    */


    s << "# ------------------------------------------ \n";
    s << "# 3. Gravity analysis.                       \n";
    s << "# ------------------------------------------ \n \n";

    s << "model BasicBuilder -ndm 3 -ndf 4 \n\n";

    // create the output streams
    OPS_Stream *theOutputStream;
    Recorder *theRecorder;

    // record last node's results, surface
    ID nodesToRecord(1);
    nodesToRecord(0) = numNodes;

    s << "# 3.1 elastic gravity analysis (transient) \n\n";

    /*
    double gamma = 0.5;
    double beta = 0.25;

    s << "constraints Transformation" << endln;
    s << "test NormDispIncr 1.0e-4 35 1" << endln;
    s << "algorithm   Newton" << endln;
    s << "numberer RCM" << endln;
    s << "system SparseGeneral" << endln;//BandGeneral
    s << "set gamma " << gamma << endln;
    s << "set beta " << beta << endln;
    s << "integrator  Newmark $gamma $beta" << endln;
    s << "analysis Transient" << endln << endln;
    */

    double gamma = 0.8333;// 5./6.;
    double beta = 0.4444;//4./9.;
    s << "set gamma " << gamma << endln;
    s << "set beta " << beta << endln;
    s << "constraints Penalty 1.e14 1.e14" << endln;
    s << "test        NormDispIncr 1e-5 30 " << endln;
    s << "algorithm   Newton" << endln;
    s << "#numberer    Plain" << endln;
    s << "numberer    RCM ;#same with cpp" << endln;
    s << "#system      SparseGeneral" << endln;
    s << "system      BandGeneral ;#same with cpp" << endln;
    s << "integrator  Newmark $gamma $beta " << endln;
    s << "analysis    Transient" << endln;


    s << "set startT  [clock seconds]" << endln;
    s << "analyze     20 5e2" << endln;
    s << "puts \"Finished with elastic gravity analysis...\"" << endln << endln;

    // create analysis objects - I use static analysis for gravity
    //AnalysisModel *
    theModel = new AnalysisModel();
    //CTestNormDispIncr *
    theTest = new CTestNormDispIncr(1.0e-5, 30, 0);
    //EquiSolnAlgo *
    theSolnAlgo = new NewtonRaphson();
    // 2. test NormDispIncr 1.0e-7 30 1
    //EquiSolnAlgo *theSolnAlgo = new NewtonRaphson(*theTest);                              // 3. algorithm   Newton (TODO: another option: KrylovNewton)
    //StaticIntegrator *theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0); // *
    //TransientIntegrator*
    theIntegrator = new Newmark(gamma, beta);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
    //ConstraintHandler*
    theHandler = new PenaltyConstraintHandler(1.0e14, 1.0e14);          // 1. constraints Penalty 1.0e15 1.0e15
    //ConstraintHandler *theHandler = new TransformationConstraintHandler(); // *
    //theHandler = new TransformationConstraintHandler(); // *
    //RCM *
    theRCM = new RCM();
    //DOF_Numberer *
    theNumberer = new DOF_Numberer(*theRCM);                                 // 4. numberer RCM (another option: Plain)
    //BandGenLinSolver *
    theSolver = new BandGenLinLapackSolver();                            // 5. system BandGeneral (TODO: switch to SparseGeneral)
    //LinearSOE *
    theSOE = new BandGenLinSOE(*theSolver);

    //DirectIntegrationAnalysis* theAnalysis;												   // 7. analysis    Transient
    theAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

    //VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
    //theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

    //StaticAnalysis *theAnalysis; // *
    //theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator); // *

    theAnalysis->setConvergenceTest(*theTest);

    int converged;
    if(doAnalysis)
    {

    // transient
    converged = theAnalysis->analyze(20,5.0e2);
    if (!converged)
    {
        opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
    } else
    {
        opserr << "Didn't converge at time " << theDomain->getCurrentTime() << endln;
    }
    opserr << "Finished with elastic gravity analysis..." << endln << endln;
     /*
    // static
    for (int analysisCount = 0; analysisCount < 2; ++analysisCount) {
            //int converged = theAnalysis->analyze(1, 0.01, 0.005, 0.02, 1);
            int converged = theAnalysis->analyze(1);
            if (!converged) {
                opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
            }
        }
     */
    }




    s << "# 3.2 plastic gravity analysis (transient)" << endln << endln;

    ElementIter &theElementIter = theDomain->getElements();
    while ((theEle = theElementIter()) != 0)
    {
        //Information stateInfo(1);
        //theEle->updateParameter(1,stateInfo);
        int theEleTag = theEle->getTag();
        if(!eleTypeDict[theEleTag].compare("PM4Sand")
         || !eleTypeDict[theEleTag].compare("PM4Silt")
         || !eleTypeDict[theEleTag].compare("ManzariDafalias")
         || !eleTypeDict[theEleTag].compare("Elastic"))
        {
            Information stateInfo(1.0);
            theEle->updateParameter(5,stateInfo);
        } else if(!eleTypeDict[theEleTag].compare("PDMY")
                  || !eleTypeDict[theEleTag].compare("PDMY02")
                  || !eleTypeDict[theEleTag].compare("PIMY")
                  || !eleTypeDict[theEleTag].compare("J2Bounding"))
        {
            Information stateInfo(1);
            theEle->updateParameter(1,stateInfo);
        }
    }

    s << endln;
    for (int i=0; i != soilMatTags.size(); i++)
        s << "updateMaterialStage -material "<< soilMatTags[i] <<" -stage 1" << endln ;


    if(doAnalysis)
    {
        converged = theAnalysis->analyze(40, 5.0e2);

        if (!converged)
        {
            opserr << "Converged at time " << theDomain->getCurrentTime() << endln;
        } else
        {
            opserr << "Didn't converge at time " << theDomain->getCurrentTime() << endln;
        }
        opserr << "Finished with plastic gravity analysis..." endln;
    }
    s << "analyze     40 5e2" << endln;
    s << "puts \"Finished with plastic gravity analysis...\"" << endln << endln;


    s << "# 3.3 Update element permeability for post gravity analysis"<< endln << endln;

    //ElementIter &
    theElementIter = theDomain->getElements();
    int tmpcount = 0;
    while ((theEle = theElementIter()) != 0 && tmpcount <= numElems)
    {
        tmpcount += 1;
        int theEleTag = theEle->getTag();
        double thishPerm = hPermVec[theEleTag-1];
        double thisvPerm = vPermVec[theEleTag-1];

        Information myInfox(-thishPerm/g);
        theEle->updateParameter(3,myInfox);
        Information myInfoy(-thisvPerm/g);
        theEle->updateParameter(4,myInfoy);
        Information myInfoz(-thishPerm/g);
        theEle->updateParameter(6,myInfoz);

        //setParameter -value 1 -ele $elementTag hPerm $matTag
        s << "setParameter -value "<<-thishPerm/g<<" -ele "<< theEleTag<<" xPerm "<<endln;
        s << "setParameter -value "<<-thisvPerm/g<<" -ele "<< theEleTag<<" yPerm "<<endln;
        s << "setParameter -value "<<-thishPerm/g<<" -ele "<< theEleTag<<" zPerm "<<endln;

    }
    s << endln << endln << endln;

    s << "setTime 0.0" << endln;
    s << "wipeAnalysis" << endln;
    s << "remove recorders" << endln << endln << endln;

    theDomain->setCommittedTime(0.0);
    //delete theIntegrator;
    delete theAnalysis;
    theDomain->removeRecorders();


    s << "# ------------------------------------------------------------\n";
    s << "# 4. Add the compliant base                                   \n";
    s << "# ------------------------------------------------------------\n\n";

    s << "# 4.1 Set basic properties of the base. \n\n";
    int dashMatTag = soilMatTags.size() + 1;
    double colArea = sElemX * colThickness;
    double vis_C = dashpotCoeff * colArea;
    double cFactor = colArea * dashpotCoeff;

    const int numberTheViscousMats = 2; // for 3D it's 2, for 2d it's 1
    UniaxialMaterial* theViscousMats[numberTheViscousMats];

    theViscousMats[0] = new ViscousMaterial(dashMatTag, vis_C, 1.0);
    OPS_addUniaxialMaterial(theViscousMats[0]);

    theViscousMats[1] = new ViscousMaterial(dashMatTag+1, vis_C, 1.0);
    OPS_addUniaxialMaterial(theViscousMats[1]);

    s << "set colThickness "<< colThickness << endln;
    s << "set sElemX " << sElemX << endln;
    s << "set colArea [expr $sElemX*$colThickness]" << endln; // [expr $sElemX*$thick(1)]
    s << "set rockVs "<< rockVs << endln;
    s << "set rockDen " << rockDen << endln;
    s << "set dashpotCoeff  [expr $rockVs*$rockDen]" << endln; // [expr $rockVs*$rockDen]
    s << "uniaxialMaterial Viscous " << dashMatTag <<" "<<"[expr $dashpotCoeff*$colArea] 1"<<endln;
    s << "set cFactor [expr $colArea*$dashpotCoeff]" << endln;



    s << "\n\n# 4.2 Create dashpot nodes and apply proper fixities. \n\n";

    theNode = new Node(numNodes + 1, 3, 0.0, 0.0, 0.0);
    theDomain->addNode(theNode); 
    theNode = new Node(numNodes + 2, 3, 0.0, 0.0, 0.0);
    theDomain->addNode(theNode); 
    theNode = new Node(numNodes + 3, 3, 0.0, 0.0, 0.0);
    theDomain->addNode(theNode); 


    s << "model BasicBuilder -ndm 3 -ndf 3" << endln << endln;
    s << "node " << numNodes + 1 << " 0.0 0.0 0.0" << endln;
    s << "node " << numNodes + 2 << " 0.0 0.0 0.0" << endln;
    s << "node " << numNodes + 3 << " 0.0 0.0 0.0" << endln;


    theSP = new SP_Constraint(numNodes + 1, 0, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    theSP = new SP_Constraint(numNodes + 1, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    theSP = new SP_Constraint(numNodes + 1, 2, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    s << "fix " << numNodes + 1 << " 1 1 1" << endln;

    theSP = new SP_Constraint(numNodes + 2, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    theSP = new SP_Constraint(numNodes + 2, 2, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    s << "fix " << numNodes + 2 << " 0 1 1" << endln;

    theSP = new SP_Constraint(numNodes + 3, 0, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    theSP = new SP_Constraint(numNodes + 3, 1, 0.0, true);
    theDomain->addSP_Constraint(theSP);
    s << "fix " << numNodes + 3 << " 1 1 0" << endln;

    s << endln;


    s << "# 4.3 Apply equalDOF to the node connected to the column. \n\n";

    int numConn = 1; 
    Matrix Ccrconn(numConn, numConn);
    Ccrconn(0, 0) = 1.0;

    ID rcDOFconn(numConn);
    rcDOFconn(0) = 0;
    theMP = new MP_Constraint(1, numNodes + 2, Ccrconn, rcDOFconn, rcDOFconn);
    theDomain->addMP_Constraint(theMP); 
    rcDOFconn(0) = 2;
    theMP = new MP_Constraint(1, numNodes + 3, Ccrconn, rcDOFconn, rcDOFconn);
    theDomain->addMP_Constraint(theMP); 

    s << "equalDOF " << 1 << " "<< numNodes + 2 << " 1" << endln;
    s << "equalDOF " << 1 << " "<< numNodes + 3 << " 3" << endln;


    for (int i_remove = 0; i_remove < sizeTheSPtoRemove; i_remove++)
    {
        theSP = theDomain->removeSP_Constraint(theSPtoRemove(i_remove));
        delete theSP;
    }
    // TODO:
    s << "remove sp 1 1" << endln;
    s << "remove sp 1 3" << endln;
    /*
    s << "remove sp 2 1" << endln;
    s << "remove sp 3 1" << endln;
    s << "remove sp 4 1" << endln;
    s << "remove sp 1 3" << endln;
    s << "remove sp 2 3" << endln;
    s << "remove sp 3 3" << endln;
    s << "remove sp 4 3" << endln;
    */

    /*


    s << "\n\n# 4.5 Apply equalDOF for the first 4 nodes (3D) or 2 nodes (2D). \n\n";

    int numMP1 = 2;// for 3D it's 2, for 2D it's 1
    Matrix constrainInXZ(numMP1, numMP1);
    ID constDOF(numMP1);
    */
    /*
    if (!theModelType.compare("2D")) //2D
    {
        constrainInXZ(0, 0) = 1.0;
        constDOF(0) = 0;
        theMP = new MP_Constraint(1, 2, constrainInXZ, constDOF, constDOF);
        theDomain->addMP_Constraint(theMP);
        s << "equalDOF " << 1 << " "<< 2 << " 1 " << endln;
    }
    */
    /*
    //theModelType.compare("3D")
    constrainInXZ(0, 0) = 1.0; constrainInXZ(1, 1) = 1.0;
    constDOF(0) = 0; constDOF(1) = 2;
    theMP = new MP_Constraint(1, 2, constrainInXZ, constDOF, constDOF);
    theDomain->addMP_Constraint(theMP);
    theMP = new MP_Constraint(1, 3, constrainInXZ, constDOF, constDOF);
    theDomain->addMP_Constraint(theMP);
    theMP = new MP_Constraint(1, 4, constrainInXZ, constDOF, constDOF);
    theDomain->addMP_Constraint(theMP);
    s << "equalDOF " << 1 << " "<< 2 << " 1 3" << endln;
    s << "equalDOF " << 1 << " "<< 3 << " 1 3" << endln;
    s << "equalDOF " << 1 << " "<< 4 << " 1 3" << endln;
    */



    s << "\n\n# 4.5 Create the dashpot element. \n\n";

    Vector x(3);
    Vector y(3);
    // default values for x and y can be found in zerolength.cpp
    x(0) = 1.0;
    x(1) = 0.0;
    x(2) = 0.0;
    y(0) = 0.0;
    y(1) = 1.0;
    y(2) = 0.0;

    int numberDirections = 1;// for 3D it's 2, for 2D it's 1
    ID directions(numberDirections);
    directions(0) = 0;
    //directions(1) = 2; // 3D
    //element zeroLength [expr $nElemT+1]  $dashF $dashS -mat [expr $numLayers+1]  -dir 1
    // TODO: double check
    UniaxialMaterial* theViscousMats_x[1]; theViscousMats_x[0] = theViscousMats[0];
    UniaxialMaterial* theViscousMats_z[1]; theViscousMats_z[0] = theViscousMats[1];
    theEle = new ZeroLength(numElems + 1, 3, numNodes + 1, numNodes + 2, x, y, 1, theViscousMats_x, directions); //TODO ?
    theDomain->addElement(theEle);
    directions(0) = 2;
    //directions(1) = 2; // 3D
    //element zeroLength [expr $nElemT+1]  $dashF $dashS -mat [expr $numLayers+1]  -dir 1
    // TODO: double check
    theEle = new ZeroLength(numElems + 2, 3, numNodes + 1, numNodes + 3, x, y, 1, theViscousMats_z, directions); //TODO ?
    theDomain->addElement(theEle);



    s << "element zeroLength "<<numElems + 1 <<" "<< numNodes + 1 <<" "<< numNodes + 2<<" -mat "<<dashMatTag<<"  -dir 1" << endln;
    s << "element zeroLength "<<numElems + 2 <<" "<< numNodes + 1 <<" "<< numNodes + 3<<" -mat "<<dashMatTag<<"  -dir 3" << endln;
    s << "\n\n\n";


    s << "# ------------------------------------------------------------\n";
    s << "# 5. Dynamic analysis                                         \n";
    s << "# ------------------------------------------------------------\n\n";

    //s << "model BasicBuilder -ndm 3 -ndf 4" << endln; // TODO: it seems this is not necessary.


    s << "# ------------------------------------------------------------\n";
    s << "# 5.1 Apply the rock motion                                    \n";
    s << "# ------------------------------------------------------------\n\n";
    int numSteps = 0;
    std::vector<double> dt;


    double dT = 0.005; // This is the time step in solution
    double motionDT = theMotionX->getDt();//  0.005; // This is the time step in the motion record. TODO: use a funciton to get it
    int nStepsMotion = theMotionX->getNumSteps();//1998;//theMotionX->getNumSteps() ; //1998; // number of motions in the record. TODO: use a funciton to get it
    int nSteps = int((nStepsMotion-1) * motionDT / dT +1);
    int remStep = nSteps;
    s << "set dT " << dT << endln;
    s << "set motionDT " << motionDT << endln;
    //s << "set mSeries \"Path -dt $motionDT -filePath /Users/simcenter/Codes/SimCenter/SiteResponseTool/test/RSN766_G02_000_VEL.txt -factor $cFactor\""<<endln;
    s << "set mSeries \"Path -dt $motionDT -filePath Rock-x.vel -factor $cFactor\""<<endln;
    s << "set mSeriesx2 \"Path -dt $motionDT -filePath Rock-y.vel -factor $cFactor\""<<endln;

    // using a stress input with the dashpot
    if (theMotionX->isInitialized() && theMotionZ->isInitialized())
    {
        LoadPattern *theLP = new LoadPattern(10, vis_C);
        theLP->setTimeSeries(theMotionX->getVelSeries());

        LoadPattern *theLPz = new LoadPattern(11, vis_C);
        theLPz->setTimeSeries(theMotionZ->getVelSeries());

        NodalLoad *theLoad;
        int numLoads = 3; // for 3D it's 4, for 2D it's 3
        Vector load(numLoads);
        load(0) = 1.0;
        load(1) = 0.0;
        load(2) = 0.0;
        //load(3) = 0.0;

        //theLoad = new NodalLoad(1, numNodes + 2, load, false); theLP->addNodalLoad(theLoad);
        theLoad = new NodalLoad(99999998, numNodes + 2, load, false);
        theLP->addNodalLoad(theLoad);
        theDomain->addLoadPattern(theLP);

        NodalLoad *theLoadz;
        Vector loadz(numLoads);
        loadz(0) = 0.0;
        loadz(1) = 0.0;
        loadz(2) = 1.0;
        //loadz(3) = 0.0;

        theLoadz = new NodalLoad(99999999, numNodes + 3, loadz, false);
        theLPz->addNodalLoad(theLoadz);
        theDomain->addLoadPattern(theLPz);

        s << "pattern Plain 10 $mSeries {"<<endln;
        s << "    load 1  1.0 0.0 0.0 0.0" << endln;
        s << "}" << endln << endln;

        s << "pattern Plain 11 $mSeriesx2 {"<<endln;
        s << "    load 1  0.0 0.0 1.0 0.0" << endln;
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
    //delete theIntegrator;
    //delete theAnalysis;

    //theTest->setTolerance(1.0e-5);


    
    //s << "constraints Transformation" << endln;
    //s << "test NormDispIncr 1.0e-4 35 0" << endln; // TODO
    //s << "algorithm   Newton" << endln;
    //s << "numberer    RCM" << endln;
    //s << "system SparseGeneral" << endln;//BandGeneral


    // create analysis objects - 3D solver
    theModel = new AnalysisModel();
    theTest = new CTestNormDispIncr(1.0e-3, 55, 0);                    // 2. test NormDispIncr 1.0e-7 30 1
    theSolnAlgo = new NewtonRaphson(*theTest);                              // 3. algorithm   Newton (TODO: another option: KrylovNewton)
    //Accelerator *theAccel = new KrylovAccelerator(3, 0);theSolnAlgo = new AcceleratedNewton(*theTest, theAccel, 0);
    //StaticIntegrator *theIntegrator = new LoadControl(0.05, 1, 0.05, 1.0); // *
    //ConstraintHandler *theHandler = new TransformationConstraintHandler(); // *
    // *
    //TransientIntegrator* theIntegrator = new Newmark(5./6., 4./9.);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
    //theIntegrator = new Newmark(0.5, 0.25);// * Newmark(0.5, 0.25)
    theHandler = new TransformationConstraintHandler();
    //theHandler = new PenaltyConstraintHandler(1.0e14, 1.0e14);          // 1. constraints Penalty 1.0e15 1.0e15
    theRCM = new RCM(false);theNumberer = new DOF_Numberer(*theRCM);    // 4. numberer RCM (another option: Plain)
    //theHandler = new PlainHandler();
    theSolver = new BandGenLinLapackSolver();  theSOE = new BandGenLinSOE(*theSolver);// 5. system BandGeneral (TODO: switch to SparseGeneral)
    //theSolver = new SuperLU(0, 0, 6, 6, 'N');theSOE = new SparseGenColLinSOE(*theSolver);



    //VariableTimeStepDirectIntegrationAnalysis* theAnalysis;
    //theAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator, theTest);

    //StaticAnalysis *theAnalysis; // *
    //theAnalysis = new StaticAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theIntegrator); // *




    double gamma_dynm = 0.5;
    double beta_dynm = 0.25;
    //TransientIntegrator*
    theTransientIntegrator = new Newmark(gamma_dynm, beta_dynm);// * Newmark(0.5, 0.25) // 6. integrator  Newmark $gamma $beta
    //theTransientIntegrator->setConvergenceTest(*theTest);

    // setup Rayleigh damping   TODO: calcualtion of these paras
    // apply 2% at the natural frequency and 5*natural frequency
    //double natFreq = SRM_layering.getNaturalPeriod();
    double pi = 4.0 * atan(1.0);

    // try no Rayleigh Damping
    // method in N10_T3
    double fmin = 5.01;
    double Omegamin  = fmin * 2.0 * pi;
    double ximin = 0.025;
    double a0 = ximin * Omegamin; //# factor to mass matrix
    double a1 = ximin / Omegamin; //# factor to stiffness matrix

    if (PRINTDEBUG)
    {
        //opserr << "f1 = " << natFreq << "    f2 = " << 5.0 * natFreq << endln;
        opserr << "a0 = " << a0 << "    a1 = " << a1 << endln;
    }
    //theDomain->setRayleighDampingFactors(a0, 0.0, a1, 0.0);
    

    //DirectIntegrationAnalysis* theTransientAnalysis;
    theTransientAnalysis = new DirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

    //VariableTimeStepDirectIntegrationAnalysis *theTransientAnalysis;
    //theTransientAnalysis = new VariableTimeStepDirectIntegrationAnalysis(*theDomain, *theHandler, *theNumberer, *theModel, *theSolnAlgo, *theSOE, *theTransientIntegrator, theTest);

    // reset time in the domain
    theDomain->setCurrentTime(0.0);


    s << "set gamma_dynm " << gamma_dynm << endln;
    s << "set beta_dynm " << beta_dynm << endln;
    s << "integrator  Newmark $gamma_dynm $beta_dynm" << endln;
    //s << "set a0 " << a0 << endln;
    //s << "set a1 " << a1 << endln;
    //s << "rayleigh    $a0 $a1 0.0 0.0" << endln;
    ////s << "analysis Transient" << endln << endln;
    //s << "analysis Transient" << endln << endln;

    





    // benchmark solver

    s << "#constraints Penalty 1.e14 1.e14"<<endln;
    s << "constraints Transformation ;#same with cpp"<<endln;
    s << "test        NormDispIncr 1.0e-3 55 "<<endln;
    s << "#algorithm   KrylovNewton"<<endln;
    s << "algorithm   Newton ;#same with cpp"<<endln;
    s << "#numberer    Plain"<<endln;
    s << "numberer    Plain ;#same with cpp"<<endln;
    s << "#system      SparseGeneral"<<endln;
    s << "system      BandGeneral ;#same with cpp"<<endln;
    // s << "integrator  Newmark $gamma $beta"<<endln;
    s << "#rayleigh    $a0 $a1 0.0 0.0"<<endln;
    s << "analysis    Transient"<<endln;



    // count brick elements
    ElementIter &theElementIterh = theDomain->getElements();
    std::vector<int> brickUPElem;
    while ((theEle = theElementIterh()) != 0)
    {
        int theEleTag = theEle->getTag();
        if (theEle->getNumDOF() == 32) // brickup ele TODO
            brickUPElem.push_back(theEleTag);
    }
    int numBrickUPEles = brickUPElem.size();


    s << "\n";
    s << "# ------------------------------------------------------------\n";
    s << "# 5.3 Define outputs and recorders                            \n";
    s << "# ------------------------------------------------------------\n\n";
    
    int dimDofToRecord = 3;
    ID dofToRecord(dimDofToRecord);
    dofToRecord(0) = 0;
    dofToRecord(1) = 1;
    dofToRecord(2) = 2;
    std::string outFile;
    if(doAnalysis)
    {
        // Record the response at the surface
        outFile = theOutputDir + PATH_SEPARATOR + "surface.acc";
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
    }
    s << "file mkdir out_tcl" << endln;
    double recDT = motionDT;
    s << "set recDT " << recDT << endln;
    s<< "eval \"recorder Node -file out_tcl/surface.disp -time -dT $recDT -node "<<numNodes<<" -dof 1 2 3  disp\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/surface.acc -time -dT $recDT -node "<<numNodes<<" -dof 1 2 3  accel\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/surface.vel -time -dT $recDT -node "<<numNodes<<" -dof 1 2 3 vel\""<<endln;// 3


    if(doAnalysis)
    {
        // Record the response of base node
        nodesToRecord.resize(1);
        nodesToRecord(0) = 1;

        dofToRecord.resize(3);
        dofToRecord(0) = 0; dofToRecord(1) = 1; dofToRecord(2) = 2;

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
    }

    s<< "eval \"recorder Node -file out_tcl/base.disp -time -dT $recDT -node 1 -dof 1 2 3  disp\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/base.acc -time -dT $recDT -node 1 -dof 1 2 3  accel\""<<endln;// 1 2
    s<< "eval \"recorder Node -file out_tcl/base.vel -time -dT $recDT -node 1 -dof 1 2 3 vel\""<<endln;// 3


    /*
    // Record pwp at node 17
    dofToRecord.resize(1);
    dofToRecord(0) = 2; // only record the pore pressure dof
    ID pwpNodesToRecord(1);
    pwpNodesToRecord(0) = 17;
    outFile = theOutputDir + PATH_SEPARATOR + "pwpLiq.out";
    theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
    theRecorder = new NodeRecorder(dofToRecord, &pwpNodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
    theDomain->addRecorder(*theRecorder);

    s<< "eval \"recorder Node -file out_tcl/pwpLiq.out -time -dT $recDT -node 17 -dof 4 vel\""<<endln;
    */

    if(doAnalysis)
    {
        // Record the response of all nodes
        nodesToRecord.resize(numNodes);
        for (int i=0;i<numNodes;i++)
            nodesToRecord(i) = i+1;
        dofToRecord.resize(2);
        dofToRecord(0) = 0;
        dofToRecord(1) = 2;

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
        dofToRecord(0) = 3;
        outFile = theOutputDir + PATH_SEPARATOR + "porePressure.out";
        theOutputStream = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new NodeRecorder(dofToRecord, &nodesToRecord, 0, "vel", *theDomain, *theOutputStream, motionDT, true, NULL);
        theDomain->addRecorder(*theRecorder);
    }

    s<< "eval \"recorder Node -file out_tcl/displacement.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 1 3  disp\""<<endln;
    s<< "eval \"recorder Node -file out_tcl/velocity.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 1 3  vel\""<<endln;
    s<< "eval \"recorder Node -file out_tcl/acceleration.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 1 3  accel\""<<endln;
    s<< "eval \"recorder Node -file out_tcl/porePressure.out -time -dT $recDT -nodeRange 1 "<<numNodes<<" -dof 4 vel\""<<endln;


    if(doAnalysis)
    {
        // Record element results
        OPS_Stream* theOutputStream2;
        ID elemsToRecord(brickUPElem.size());
        for (int i=0;i<brickUPElem.size();i+=1)
            elemsToRecord(i) = brickUPElem[i];
        //const char* eleArgs = "stress";

        const char *eleArgs[2];
        eleArgs[0] = "stress";
        eleArgs[1] = "6";

        outFile = theOutputDir + PATH_SEPARATOR + "stress.out";
        theOutputStream2 = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new ElementRecorder(&elemsToRecord, eleArgs, 2, true, *theDomain, *theOutputStream2, motionDT, NULL);
        theDomain->addRecorder(*theRecorder);

        const char* eleArgsStrain = "strain";
        outFile = theOutputDir + PATH_SEPARATOR + "strain.out";
        theOutputStream2 = new DataFileStream(outFile.c_str(), OVERWRITE, 2, 0, false, 6, false);
        theRecorder = new ElementRecorder(&elemsToRecord, &eleArgsStrain, 1, true, *theDomain, *theOutputStream2, motionDT, NULL);
        theDomain->addRecorder(*theRecorder);
    }


    //s<< "recorder Element -file out_tcl/stress2.out -time -dT $recDT  -eleRange 1 "<<numBrickUPEles<<"  stress "<<endln;
    //s<< "recorder Element -file out_tcl/stress1.xxxout -time -dT $recDT  -eleRange 1 "<<numBrickUPEles<<"  stress 6 \n";
    s<< "recorder Element -file out_tcl/stress.out -time -dT $recDT  -eleRange 1 "<<numBrickUPEles<<"  stress 6 \n";
    //s<< "recorder Element -file out_tcl/stress4.out -time -dT $recDT  -eleRange 1 "<<numBrickUPEles<<"  stress "<<endln;

    s<< "recorder Element -file out_tcl/strain.out -time -dT $recDT  -eleRange 1 "<<numBrickUPEles<<"  strain"<<endln;
    s<< endln;






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
    s << "	for {set i 0} {$i < 3} {incr i} {" << endln;
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


    /*
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
    */


    /*
    s << "puts \"Start analysis\"" << endln;
    s << "set startT [clock seconds]" << endln;
    //s << "if {1} {" << endln;
    s << "set finalTime [expr $remStep * $dT]" << endln;
    s << "set success 0" << endln;
    s << "set currentTime 0." << endln;
    s << "set timeMarker 0." << endln;
    s << "while {$success == 0 && $currentTime < $finalTime} {" << endln;
    s << "	set subStep 0" << endln;
    s << "	set success [analyze 1  $dT]" << endln;
    s << "	if {$success != 0} {" << endln;
    s << "	set curTime  [getTime]" << endln;
    s << "	puts \"Analysis failed at $curTime . Try substepping.\"" << endln;
    s << "	set success  [subStepAnalyze [expr $dT/2.0] [incr subStep]]" << endln;
    s << "	set curStep  [expr int($curTime/$dT + 1)]" << endln;
    s << "	set remStep  [expr int($nSteps-$curStep)]" << endln;
    s << "	puts \"Current step: $curStep , Remaining steps: $remStep\"" << endln;
    s << "    } else {" << endln;
    s << "          set progress [expr $currentTime/$finalTime * 100.]" << endln;
    s << "          if { $progress > $timeMarker} {" << endln;
    s << "              set timeMarker [expr $timeMarker+2]" << endln;
    s << "              puts \"$progress%\"" << endln;
    s << "              }" << endln;
    s << "              set currentTime [getTime]" << endln;
    s << "	}" << endln;
    s << "}" << endln << endln;
    //s << "}" << endln << endln;
    */


    /*
    s << "if {0} {" << endln;
    s << "while {$success != -10} {" << endln;
    s << "    set subStep 0" << endln;
    s << "    set success [analyze $remStep  $dT]" << endln;
    s << "    if {$success == 0} {" << endln;
    s << "        puts \"Analysis Finished\"" << endln;
    s << "        break" << endln;
    s << "    } else {" << endln;
    s << "        set curTime  [getTime]" << endln;
    s << "        puts \"Analysis failed at $curTime . Try substepping.\"" << endln;
    s << "        set success  [subStepAnalyze [expr $dT/2.0] [incr subStep]]" << endln;
    s << "        set curStep  [expr int($curTime/$dT + 1)]" << endln;
    s << "       set remStep  [expr int($nSteps-$curStep)]" << endln;
    s << "        puts \"Current step: $curStep , Remaining steps: $remStep\"" << endln;
    s << "    }" << endln;
    s << "}" << endln;
    s << "}" << endln;
    */

    /*
    s << "set endT [clock seconds]" << endln << endln;
    s << "puts \"loading analysis execution time: [expr $endT-$startT] seconds.\"" << endln << endln;
    s << "puts \"Finished with dynamic analysis...\"" << endln << endln;

    s << endln;
    s << "print -file out_tcl/Domain.out" << endln << endln;

    s << "wipe" << endln;
    s << "puts \"Site response analysis is finished.\n\""<< endln;
    s << "exit" << endln << endln;
    */




    /*
    s << "# perform analysis with timestep reduction loop" <<endln;
    s << "set ok [analyze $nSteps  $dT]" <<endln;
    //s << "puts $cmd \"set ok analyze $nSteps  $dT\"" <<endln;
    s << "" <<endln;
    s << "# if analysis fails, reduce timestep and continue with analysis" <<endln;
    s << "if {$ok != 0} {" <<endln;
    //s << "    puts \"did not converge, reducing time step\"" <<endln;
    s << "    set curTime  [getTime]" <<endln;
    s << "    set mTime $curTime" <<endln;
    //s << "    puts \"curTime: $curTime\"" <<endln;
    s << "    set curStep  [expr $curTime/$dT]" <<endln;
    //s << "    puts \"curStep: $curStep\"" <<endln;
    s << "    set rStep  [expr ($nSteps-$curStep)*2.0]" <<endln;
    s << "    set remStep  [expr int(($nSteps-$curStep)*2.0)]" <<endln;
    //s << "    puts \"remStep: $remStep\"" <<endln;
    s << "    set dT       [expr $dT/2.0]" <<endln;
    //s << "    puts \"dT: $dT\"" <<endln;
    s << " " <<endln;
    s << "    set ok [analyze  $remStep  $dT]" <<endln;
    s << "    set progress [expr ($nSteps - $remStep)/$nSteps * 100.]" << endln;
    s << "    puts \"$progress%\"" << endln;
    s << " " <<endln;
    s << "    # if analysis fails again, reduce timestep and continue with analysis" <<endln;
    s << "    if {$ok != 0} {" <<endln;
    //s << "        puts \"did not converge, reducing time step\"" <<endln;
    s << "        set curTime  [getTime]" <<endln;
    //s << "        puts \"curTime: $curTime\"" <<endln;
    s << "        set curStep  [expr ($curTime-$mTime)/$dT]" <<endln;
    //s << "        puts \"curStep: $curStep\"" <<endln;
    s << "        set remStep  [expr int(($rStep-$curStep)*2.0)]" <<endln;
    //s << "        puts \"remStep: $remStep\"" <<endln;
    s << "        set dT       [expr $dT/2.0]" <<endln;
    //s << "        puts \"dT: $dT\"" <<endln;
    s << " " <<endln;
    s << "        analyze  $remStep  $dT" <<endln;
    s << "    }" <<endln;
    s << "}" <<endln;
    s << "set endT    [clock seconds]" <<endln;
    s << "puts \"Finished with dynamic analysis...\"" <<endln;
    s << "puts \"Analysis execution time: [expr $endT-$startT] seconds\"" <<endln;
    s << "" <<endln;
    */






    s << "puts \"Start analysis\"" << endln;
    s << "set remStep $nSteps" << endln;
    s << "set startT [clock seconds]" << endln;
    s << "set finalTime [expr $remStep * $dT]" << endln;
    s << "set success 0" << endln;
    s << "set currentTime 0." << endln;
    s << "set timeMarker 0." << endln;
    s << "while {$success == 0 && $currentTime < $finalTime} {" << endln;
    s << "	set subStep 0" << endln;
    s << "	set success [analyze 1  $dT]" << endln;
    s << "	if {$success != 0} {" << endln;
    s << "	set curTime  [getTime]" << endln;
    s << "	puts \"Analysis failed at $curTime . Try substepping.\"" << endln;
    s << "	set success  [subStepAnalyze [expr $dT/2.0] [incr subStep]]" << endln;
    s << "	set curStep  [expr int($curTime/$dT + 1)]" << endln;
    s << "	set remStep  [expr int($nSteps-$curStep)]" << endln;
    s << "	puts \"Current step: $curStep , Remaining steps: $remStep\"" << endln;
    s << "    } else {" << endln;
    s << "          set progress [expr $currentTime/$finalTime * 100.]" << endln;
    s << "          if { $progress > $timeMarker} {" << endln;
    s << "              set timeMarker [expr $timeMarker+2]" << endln;
    s << "              puts \"$progress%\"" << endln;
    s << "              }" << endln;
    s << "              set currentTime [getTime]" << endln;
    s << "	}" << endln;
    s << "}" << endln << endln;
    s << "remove recorders" << endln;
    s << "set endT    [clock seconds]" <<endln;
    s << "puts \"Finished with dynamic analysis...\"" <<endln;
    s << "puts \"Analysis execution time: [expr $endT-$startT] seconds\"" <<endln;
    //s << "print -file out_tcl/Domain-3D-s3hark-tcl.out" <<endln;




    s << "" <<endln;
    s << "wipe" <<endln;
    s << "puts \"Site response analysis is finished.\"\n"<< endln;

    s.close();
    ns.close();
    es.close();
    esmat3D.close();



    m_dT = dT;
    m_nSteps = nSteps;
    m_remStep = remStep;
    return trueRun();

    /*
   //doAnalysis = true;

        if(doAnalysis)
    {

        bool useSubstep = false;
        int stepLag = 2;

        if(!useSubstep)
        {   // no substepping
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

                        if (callback) m_callbackFunction(100.0 * analysisCount / remStep);
                    }
                }
                else
                {
                    opserr << "Site response analysis did not converge." << endln;
                    exit(-1);
                }
            }
            opserr << "Site response analysis done..." << endln;
            if (callback) m_callbackFunction(100.0);
            progressBar << "\r[";
            for (int ii = 0; ii < 20; ii++)
                progressBar << "-";

            progressBar << "]  🚌   100%\n";
            opsout << progressBar.str().c_str();
            opsout.flush();
            opsout << endln;
        } else { // substep
            opserr << "Analysis started (substepping):" << endln;
            double finalTime = dT * remStep;
            int success = 0;
            double currentTime = 0.;
            std::stringstream progressBar;
            int currentProgress = 0;
            int remStept = 0;
            double timeMaker = 0.;

            while(fabs(success)<1 && currentTime < finalTime )
            {

                int subStep = 0;
                success = theTransientAnalysis->analyze(1, dT);
                if(fabs(success)>0)
                {   // analysisi failed at currenttime

                    success = subStepAnalyze(dT/2., subStep+1, theTransientAnalysis);
                } else {
                    currentProgress = int(currentTime/finalTime *100.);
                    if (currentProgress > timeMaker)
                    {
                        timeMaker += 1;
                        //std::cout << currentProgress << endln;
                        remStept = 100-currentProgress;
                        if (currentProgress % stepLag == 0 && currentProgress > stepLag)
                        {
                            progressBar << "\r[";
                            for (int ii = 0; ii < currentProgress/stepLag; ii++)
                                progressBar << "-";
                            progressBar << " 🚌  ";
                            for (int ii = 0; ii < remStept/stepLag; ii++)
                                progressBar << ".";

                            progressBar << "]  " << currentProgress << "%";
                            opsout << progressBar.str().c_str();
                            opsout.flush();
                            if (callback) m_callbackFunction(currentTime/finalTime *100.);
                        }
                    }

                    currentTime = theDomain->getCurrentTime();
                }
            }

            OPS_Stream* theOutputStreamAll;
            theOutputStreamAll = new DataFileStream("/Users/simcenter/Codes/SimCenter/s3hark/bin/Domain-3D-s3hark.out", OVERWRITE, 2, 0, false, 6, false);
            theDomain->Print(*theOutputStreamAll);
            opserr << theOutputStreamAll;
            delete theOutputStreamAll;


            opserr << "Site response analysis done..." << endln;
            if (callback) m_callbackFunction(100.0);
            progressBar << "\r[";
            for (int ii = 0; ii < 100/stepLag; ii++)
                progressBar << "-";

            progressBar << "]  🚌   100%\n";
            opsout << progressBar.str().c_str();
            opsout.flush();
            opsout << endln;

        }

    } else{
        std::cout << "tcl file built." << std::endl;
    }

    OPS_Stream* theOutputStreamAll;
    theOutputStreamAll = new DataFileStream("/Users/simcenter/Codes/SimCenter/s3hark/bin/Domain-3D-s3hark.out", OVERWRITE, 2, 0, false, 6, false);
    theDomain->Print(*theOutputStreamAll);
    opserr << theOutputStreamAll;
    delete theOutputStreamAll;



    theDomain->removeRecorders();

    return 100;
    */
}


int SiteResponseModel::runEffectiveStressModel2D()
{
    return 0;
}
