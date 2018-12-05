
import QtQuick 2.0
import QtQml.Models 2.2


import QtQuick.Shapes 1.11;

Rectangle {
    id: container
    width: 200//parent.width
    height: 500
    color: "#e2e2e2"

    property real soilWidth: width*0.6
    property real startXLoc: (1-0.6)*width/2
    property real parentHeight: 500



    DelegateModel {
            id: viewsoilModel
            model: soilModel
            groups: [
                DelegateModelGroup { name: "selected" }
            ]
/*
            delegate: Rectangle {
                id: item
                height: 25
                width: 200
                Text {
                    text: {
                        var text = "Name: " + soilModel.getLayerName(index)
                        if (item.DelegateModel.inSelected)
                            text += " (" + item.DelegateModel.selectedIndex + ")"
                        return text;
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: item.DelegateModel.inSelected = !item.DelegateModel.inSelected
                }
            }
            */

            delegate: Shape {
                id: item
                //anchors.centerIn: parent

                property real parentHeight: 500
                property real totalSoilHeight: soilModel.getTotalHeight()
                property real heightFactor: parentHeight*0.8/soilModel.getTotalHeight()

                property real thickness: soilModel.getThickness(index)
                property real toppos: soilModel.getToppos(index)
                property real botompos: soilModel.getBotompos(index)
                property real parentWidth: parent.width

                property string soilColor: soilModel.getSoilColor(index)
                property string soilName: index.toString()

                property real marginTop: 0.15*totalSoilHeight*heightFactor

                property real startYLoc: index<1 ? marginTop : 0 //toppos*heightFactor + 0.15*totalSoilHeight*heightFactor


                width: soilWidth/0.6
                height: index<1 ? thickness*heightFactor + marginTop : thickness*heightFactor


                property string mouseAreaID: index
                property bool isActive: soilModel.isActive(index)


                ShapePath {
                    strokeWidth: isActive ? 2 : 1
                    strokeColor: isActive ? "red" : soilColor
                    fillColor: soilColor
                    strokeStyle: ShapePath.SolidLine
                    dashPattern: [ 1, 4 ]

                    startX: startXLoc; startY: startYLoc
                    PathLine { x: startXLoc+soilWidth; y: startYLoc }
                    PathLine { x: startXLoc+soilWidth; y: startYLoc + thickness*heightFactor -2}
                    PathLine { x: startXLoc; y: startYLoc + thickness*heightFactor -2 }
                    PathLine { x: startXLoc; y: startYLoc }
                }



                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        //soilModel.setActive(index);
                        designTableModel.setActive(index);
                    }
                    enabled: true
                }
            }

        }



        ListView {
            anchors.fill: parent
            model: viewsoilModel
        }



        property real theGWT: designTableModel.m_nGWT
        property real startXGWT: (width-soilWidth)/2
        property real startYGWT: (theGWT + 0.15*soilModel.getTotalHeight())*parentHeight*0.8/soilModel.getTotalHeight()
        property real p1X: width/2+soilWidth/2 + 20
        property real p1Y: (theGWT + 0.15*soilModel.getTotalHeight())*parentHeight*0.8/soilModel.getTotalHeight()
        property real p2X: p1X + 10
        property real p2Y: p1Y - 10
        property real p3X: p1X - 10
        property real p3Y: p1Y - 10
        property real p4X: p1X
        property real p4Y: p1Y
        Shape {// water table

        ShapePath {
            strokeWidth: 2
            strokeColor: "green"
            strokeStyle: ShapePath.SolidLine
            dashPattern: [ 1, 2 ]

            startX:startXGWT
            startY:startYGWT
            PathLine { x:p1X ; y:p1Y  }
            PathLine { x:p2X ; y:p2Y  }
            PathLine { x:p3X ; y:p3Y  }
            PathLine { x:p4X ; y:p4Y  }

        }
        }





}





