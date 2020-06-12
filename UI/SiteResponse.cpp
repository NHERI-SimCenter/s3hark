#include "SiteResponse.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;



// these must be defined here!!
StandardStream sserr;
FileStream ferr("fem.log");
OPS_Stream *opserrPtr = &ferr;
OPS_Stream *opsoutPtr = &sserr;


SiteResponse::SiteResponse(std::string configureFile,std::string anaDir,std::string outDir,std::string femLog, std::function<bool(double)> callbackFunction ) :
    m_configureFile(configureFile),
    m_analysisDir(anaDir),
    m_outputDir(outDir),
    m_femLog(femLog)
{
    m_callbackFunction = callbackFunction;
    init(configureFile,anaDir,outDir);
    model->setCallback(true);
}

SiteResponse::SiteResponse(std::string configureFile,std::string anaDir,std::string outDir,std::string femLog) :
    m_configureFile(configureFile),
    m_analysisDir(anaDir),
    m_outputDir(outDir),
    m_femLog(femLog)
{
    init(configureFile,anaDir,outDir);
}


void SiteResponse::init(std::string configureFile,std::string anaDir,std::string outDir)
{

    // set fem log
    ferr_true = new FileStream(m_femLog.c_str());
    opserrPtr = ferr_true;

    std::ifstream i(configureFile);
    json SRT;
    i >> SRT;
    std::string thisSimType = SRT["basicSettings"]["simType"];

    is3D = false;
    if (!thisSimType.compare("3D2D")) is3D = true;
    if (!thisSimType.compare("3D1D")) is3D = true;

    //./siteresponse ../test/siteLayering.loc -bbp ../test/9130326.nwhp.vel.bbp out thisLog
    // read the layering file
    std::string layersFN("/Users/simcenter/Codes/SimCenter/SiteResponseTool/test/siteLayering.loc");
    std::string bbpOName(".");
    SiteLayering siteLayers(layersFN.c_str());
    int t = siteLayers.getNumLayers();


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


        if (is3D)
        {
            // 3D
            std::string bbpLName = "Log";
            //std::string motionXFN("/Users/simcenter/Codes/SimCenter/SiteResponseTool/test/RSN766_G02_000_VEL");
            std::string motionXFN(anaDir+"/Rock-x");//TODO: may not work on windows
            std::string motionZFN(anaDir+"/Rock-y");//TODO: may not work on windows
            motionX.setMotion(motionXFN.c_str());
            motionZ.setMotion(motionZFN.c_str());
            bbpOName = "out";
            //model = new SiteResponseModel("3D", &motionX, &motionZ, m_callbackFunction);
            model = new SiteResponseModel("3D", &motionX, &motionZ, m_callbackFunction);
            //model = new SiteResponseModel("2D", &motionX);
            model->setOutputDir(outDir);
            model->setAnalysisDir(anaDir);
            model->setTclOutputDir(outDir);
            model->setConfigFile(configureFile);

            //buildTcl();
        } else {//2D
            std::string bbpLName = "Log";
            //std::string motionXFN("/Users/simcenter/Codes/SimCenter/SiteResponseTool/test/RSN766_G02_000_VEL");
            std::string motionXFN(anaDir+"/Rock-x");//TODO: may not work on windows
            motionX.setMotion(motionXFN.c_str());
            bbpOName = "out";
            model = new SiteResponseModel("2D", &motionX, m_callbackFunction);
            //model = new SiteResponseModel("2D", &motionX);
            model->setOutputDir(outDir);
            model->setAnalysisDir(anaDir);
            model->setTclOutputDir(outDir);
            model->setConfigFile(configureFile);

            //buildTcl();
        }
    }

}
void SiteResponse::buildTcl()
{
    bool runAnalysis = false;
    model->buildEffectiveStressModel2D(runAnalysis);
}

void SiteResponse::buildTcl3D()
{
    bool runAnalysis = false;
    model->buildEffectiveStressModel3D(runAnalysis);
}

void SiteResponse::kill()
{
    model->setForward(false);
}


int SiteResponse::run()
{
    if (is3D) run3D();
    else run2D();
    return 0;
}
int SiteResponse::run2D()
{
    bool runAnalysis = true;
    if (model->buildEffectiveStressModel2D(runAnalysis)!=100)
        return -1;
    else
        return 1;
}

int SiteResponse::run3D()
{
    bool runAnalysis = true;
    if (model->buildEffectiveStressModel3D(runAnalysis)!=100)
        return -1;
    else
        return 1;
}


SiteResponse::~SiteResponse()
{

}

