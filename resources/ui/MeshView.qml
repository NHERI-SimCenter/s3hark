
import QtQuick 2.0
import QtQml.Models 2.2


import QtQuick.Shapes 1.11;



Rectangle {
    id: container
    width: 200//parent.width
    height: 500//320
    color: "#e2e2e2"

    property real eleWidth: elements.getWidth()


    property real parentHeight: 500//320
    property real hFactor:  0.8* parentHeight / totalHeight
    property real startYLoc: 0.15*totalHeight*hFactor

    property real soilWidth: eleWidth*hFactor //width*0.3
    property real startXLoc: (width-soilWidth)/2 //(1-0.3)*width/2

Rectangle {
    id: innerRect
    x: container.startXLoc
    y: container.startYLoc
    width: container.soilWidth
    height: container.parentHeight*0.8



    ListView {
        width: container.soilWidth
        height: container.parentHeight*0.8


        model: elements
        delegate: Rectangle {
            height: h*container.hFactor
            width: container.soilWidth
            color: mcolor
            border.color: "green"
            border.width: 0.5
            //Text { text: tag + " " + h + " " + totalHeight + " " + hFactor }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    //soilModel.setActive(index) ;
                    console.log(elements.getWidth());
                }
                enabled: true
            }
        }


    }



}
}





