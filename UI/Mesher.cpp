#include "Mesher.h"
#include <exception>
#include <cmath>

Mesher::Mesher()
{

}



bool Mesher::mesh2DColumn(){

    std::string configFile = "SRT.json";
    std::ifstream i(configFile);
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
    int numLayers;
/*
                        {
                            "color": "#1bff86",
                            "density": 1.61,
                            "eSize": 2.0,
                            "id": 1,
                            "material": 1,
                            "name": "Layer 1",
                            "thickness": 2.0,
                            "vs": 98.78
                        },
                        */
    Node* node;
    Quad* elem;
    node = new Node(1, 0.0, 0.0, 0.0);
    nodes.push_back(node);
    node = new Node(2, 0.0, eSizeH, 0.0);
    nodes.push_back(node);
    int numNodes = 2;
    double ycrd = 0.0;
    int numEles = 0;

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
                std::string err = "eSize is tool small. change it in the json file.";
                throw err;
            }
            double thickness = l["thickness"];
            double vs = l["vs"];
            int matTag = l["material"];
            std::string color = l["color"];

            int numEleThisLayer = static_cast<int> (std::round(thickness / eSizeV));
            numEleThisLayer = std::max(1,numEleThisLayer);
            double t = thickness / numEleThisLayer;
            for (int i=1; i<=numEleThisLayer;i++)
            {
                ycrd += t * i;
                node = new Node(numNodes + 1, 0.0, ycrd, 0.0);
                nodes.push_back(node);
                node = new Node(numNodes + 2, eSizeH, ycrd, 0.0);
                nodes.push_back(node);

                elem = new Quad(numEles, numNodes-2, numNodes-1, numNodes+2, numNodes+1, t, color);
                elements.push_back(elem);

                numNodes += 2;
                numEles += 1;

            }

            std::cout << "eleTag: " << eTag << std::endl;
            numLayers += 1;
        }

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



}
