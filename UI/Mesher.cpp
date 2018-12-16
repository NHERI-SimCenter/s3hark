#include "Mesher.h"

Mesher::Mesher()
{

}

Mesher::Mesher(BonzaTableModel* tableModel) :
    m_tableModel(tableModel)
{

}

bool Mesher::mesh2DColumn(){

    int numLayers = m_tableModel->rowCount();
    if (numLayers < 1)
        return false;

    Node* node;
    node = new Node(1, 0.0, 0.0, 0.0);
    nodes.push_back(node);
    //node = new Node(1, eSizeX, 0.0, 0.0);
    //nodes.push_back(node);
    // get
}
