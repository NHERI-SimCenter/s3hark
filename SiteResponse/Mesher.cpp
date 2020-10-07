#include "Mesher.h"
#include <exception>
#include <cmath>

Mesher::Mesher()
{

}

Mesher::Mesher(std::string jsonFile):
m_configureFile(jsonFile)
{

}


bool Mesher::mesh2DColumn(){

    return mesh2DColumnFromFile();

}

bool Mesher::mesh2DColumnFromJson(json &j){
    nodes.clear();
    elements.clear();


    json basicSettings;
    double dampingCoeff,dashpotCoeff,eSizeH,eSizeV,groundWaterTable,rockDen,rockVs;
    std::string groundMotion;
    try
    {
        basicSettings = j["basicSettings"];
        dampingCoeff = basicSettings["dampingCoeff"];
        dashpotCoeff = basicSettings["dashpotCoeff"];
        eSizeH = basicSettings["eSizeH"];
        eSizeV = basicSettings["eSizeV"];
        groundMotion = basicSettings["groundMotion"];
        groundWaterTable = basicSettings["groundWaterTable"];
        rockDen = basicSettings["rockDen"];
        rockVs = basicSettings["rockVs"];
        if (eSizeH<minESizeH)
        {
            std::string err = "eSizeH is tool small. change it in the json file.";
            throw err;
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        return false;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
        return false;
    }

    json soilProfile,soilLayers,layer;
    Nodex* node;
    Quadx* elem;
    node = new Nodex(1, 0.0, 0.0, 0.0);
    nodes.push_back(node);
    node = new Nodex(2, 0.0, eSizeH, 0.0);
    nodes.push_back(node);
    int numNodes = 2;
    double ycrd = 0.0;
    int numEles = 0;
    m_eSizeH = eSizeH;

    soilProfile = j["soilProfile"];



    try
    {
        soilProfile = j["soilProfile"];
        soilLayers = soilProfile["soilLayers"];
        std::sort(soilLayers.begin(),soilLayers.end(),
                  [](const json &a, const json &b) { return a["id"] > b["id"]; });
        for (auto l:soilLayers)
        {
            int eTag = l["id"];
            double eSizeV = l["eSize"];
            if (eSizeV<minESizeV)
            {
                eSizeV = minESizeV;
                //std::string err = "eSize is tool small. change it in the json file.";
                //throw err;
            }
            double thickness = l["thickness"];
            double vs = l["vs"];
            int matTag = l["material"];
            std::string color = l["color"];

            if (thickness>0.000001)
            {
            int numEleThisLayer = static_cast<int> (std::round(thickness / eSizeV));
            numEleThisLayer = std::max(1,numEleThisLayer);
            double t = thickness / numEleThisLayer;
            for (int i=1; i<=numEleThisLayer;i++)
            {
                ycrd += t ;
                node = new Nodex(numNodes + 1, 0.0, ycrd, 0.0);
                nodes.push_back(node);
                node = new Nodex(numNodes + 2, eSizeH, ycrd, 0.0);
                nodes.push_back(node);

                elem = new Quadx(numEles, numNodes-1, numNodes, numNodes+2, numNodes+1, t, color);
                elements.push_back(elem);

                numNodes += 2;
                numEles += 1;
            }
            }

            //std::cout << "eleTag: " << eTag << std::endl;
            m_numLayers += 1;
        }
        m_totalHeight = ycrd;
        m_numElements = numEles;
        m_numNodes = numNodes;
    }
    catch (std::exception& e)
    {
        std::cout << "Standard exception: " << e.what() << std::endl;
        return false;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
        return false;
    }
    return true;

}



bool Mesher::mesh2DColumnFromFile(){
    nodes.clear();
    elements.clear();

    //std::string configFile = "/Users/simcenter/Codes/SimCenter/SiteResponseTool/bin/SRT.json";
    //std::string configFile = "SRT.json";
    std::ifstream i(m_configureFile);
    if(!i)
        return false;// failed to open SRT.json TODO: print to log

    json j;
    i >> j;

    json basicSettings;
    double dampingCoeff,dashpotCoeff,eSizeH,eSizeV,groundWaterTable,rockDen,rockVs;
    std::string groundMotion;
    try
    {
        basicSettings = j["basicSettings"];
        dampingCoeff = basicSettings["dampingCoeff"];
        dashpotCoeff = basicSettings["dashpotCoeff"];
        eSizeH = basicSettings["eSizeH"];
        eSizeV = basicSettings["eSizeV"];
        groundMotion = basicSettings["groundMotion"];
        groundWaterTable = basicSettings["groundWaterTable"];
        rockDen = basicSettings["rockDen"];
        rockVs = basicSettings["rockVs"];
        if (eSizeH<minESizeH)
        {
            std::string err = "eSizeH is tool small. change it in the json file.";
            throw err;
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        return false;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
        return false;
    }

    json soilProfile,soilLayers,layer;
    Nodex* node;
    Quadx* elem;
    node = new Nodex(1, 0.0, 0.0, 0.0);
    nodes.push_back(node);
    node = new Nodex(2, 0.0, eSizeH, 0.0);
    nodes.push_back(node);
    int numNodes = 2;
    double ycrd = 0.0;
    int numEles = 0;
    m_eSizeH = eSizeH;

    soilProfile = j["soilProfile"];



    try
    {
        soilProfile = j["soilProfile"];
        soilLayers = soilProfile["soilLayers"];
        std::sort(soilLayers.begin(),soilLayers.end(),
                  [](const json &a, const json &b) { return a["id"] > b["id"]; });
        for (auto l:soilLayers)
        {
            int eTag = l["id"];
            double eSizeV = l["eSize"];
            if (eSizeV<minESizeV)
            {
                eSizeV = minESizeV;
                //std::string err = "eSize is tool small. change it in the json file.";
                //throw err;
            }
            double thickness = l["thickness"];
            double vs = l["vs"];
            int matTag = l["material"];
            std::string color = l["color"];

            if (thickness>0.000001)
            {
            int numEleThisLayer = static_cast<int> (std::round(thickness / eSizeV));
            numEleThisLayer = std::max(1,numEleThisLayer);
            double t = thickness / numEleThisLayer;
            for (int i=1; i<=numEleThisLayer;i++)
            {
                ycrd += t ;
                node = new Nodex(numNodes + 1, 0.0, ycrd, 0.0);
                nodes.push_back(node);
                node = new Nodex(numNodes + 2, eSizeH, ycrd, 0.0);
                nodes.push_back(node);

                elem = new Quadx(numEles, numNodes-1, numNodes, numNodes+2, numNodes+1, t, color);
                elements.push_back(elem);

                numNodes += 2;
                numEles += 1;
            }
            }

            //std::cout << "eleTag: " << eTag << std::endl;
            m_numLayers += 1;
        }
        m_totalHeight = ycrd;
        m_numElements = numEles;
        m_numNodes = numNodes;
    }
    catch (std::exception& e)
    {
        std::cout << "Standard exception: " << e.what() << std::endl;
        return false;
    }
    catch(std::string str)
    {
        std::cerr << str << std::endl;
        return false;
    }
    return true;

}
