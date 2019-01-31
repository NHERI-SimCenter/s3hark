
import QtQuick 2.0
import QtQml.Models 2.2


import QtQuick.Shapes 1.11;



Rectangle {
    anchors.centerIn: parent
    id: container
    width: parent.width//200//parent.width
    height: parent.height//500//320
    color: "#ffffff"//"#e2e2e2"
    //border.color: "black"
    //border.width: 1
    //radius: 1

    property real eleWidth: elements.getWidth()
    property real totalHeight: elements.getTotalHeight()
    property real parentHeight: height//500//320



Rectangle {

    property real totalSoilHeight: elements.getTotalHeight()
    property real startYLoc: 0.15*0.8*container.height
    property real soilWidth: elements.getWidth()*0.8* container.height / elements.getTotalHeight() // eleWidth*hFactor //width*0.3
    //property real startXLoc: (container.width-elements.getWidth()*0.8* container.height / elements.getTotalHeight())/2 //(1-0.3)*width/2
    property real startXLoc: (container.width * 0.5)/2 //(1-0.3)*width/2

    id: innerRect
    x: startXLoc
    y: startYLoc
    width: parent.width
    height: parent.height*0.8
    //color: "#000000"//"#e2e2e2"


    ListView {
        width: elements.getWidth()*0.8* container.height / elements.getTotalHeight()
        height: container.height*0.8



        model: elements
        delegate: Rectangle {

            property real hFactor:  0.8* container.height / elements.getTotalHeight()
            //property real soilWidth: elements.getWidth()*0.8* container.height / elements.getTotalHeight()
            property real soilWidth: (container.width * 0.5)

            height: h*hFactor
            width: soilWidth
            color: mcolor
            border.color: "green"
            border.width: 0.5
            //Text { text: tag + " " + h + " " + totalHeight + " " + hFactor }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    //soilModel.setActive(index) ;
                    console.log(elements.getTotalHeight());
                }
                enabled: true
            }
        }
    }




}
}





