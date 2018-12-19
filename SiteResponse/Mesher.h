#ifndef MESHER_H
#define MESHER_H

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include <nlohmann/json.hpp>
using json = nlohmann::json;



class Nodex
{

public:
    Nodex(int tag, double x, double y, double z) :
    m_tag(tag),m_x(x),m_y(y),m_z(z)
    {}
    double x(){return m_x;}
    double y(){return m_y;}
    double z(){return m_z;}
    int tag(){return m_tag;}

private:
    int m_tag;
    double m_x;
    double m_y;
    double m_z;
};

class Quadx
{

public:
    Quadx(int tag, int i, int j, int k, int l,double thickness,std::string color) :
    m_tag(tag),m_i(i),m_j(j),m_k(k),m_l(l),m_thickness(thickness),m_color(color)
    {}
    int i(){return m_i;}
    int j(){return m_j;}
    int k(){return m_k;}
    int l(){return m_l;}
    int tag()const {return m_tag;}
    double thickness(){return m_thickness;}
    std::string color(){return m_color;}
private:
    int m_tag;
    int m_i;
    int m_j;
    int m_k;
    int m_l;
    double m_thickness;
    std::string m_color;
};





class Mesher
{

public:
    Mesher();
    Mesher(std::string jsonFile);
    bool mesh2DColumn();

    std::vector<Nodex*> nodes;
    std::vector<Quadx*> elements;
    double minESizeH = 0.001;
    double minESizeV = 0.001;
    double eleThick = 1.0;// thickness of 2D ele


    int size(){return elements.size();}
    int numLayers(){return m_numLayers;}
    int numNodes(){return m_numNodes;}
    int numElements(){return m_numElements;}
    double eSizeH(){return m_eSizeH;}
    double totalHeight(){return m_totalHeight;}
private:
    double m_eSizeH = 0.0;
    double m_totalHeight = 0.0;
    int m_numLayers = 0;
    int m_numNodes = 0;
    int m_numElements = 0;
    std::string m_outPutFile;


};

#endif // MESHER_H
