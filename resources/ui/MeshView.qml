
import QtQuick 2.0
import QtQml.Models 2.2


import QtQuick.Shapes 1.11;



Rectangle {
    id: container
    width: 200//parent.width
    height: 500//320
    color: "#e2e2e2"

    property real soilWidth: width*0.6
    property real startXLoc: (1-0.6)*width/2
    property real parentHeight: 500//320
    property real hFactor:  0.8* parentHeight / totalHeight
    property real startYLoc: 0.15*totalHeight*hFactor

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
            border.color: "black"
            border.width: 1
            //Text { text: tag + " " + h + " " + totalHeight + " " + hFactor }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    //soilModel.setActive(index) ;
                    //console.log(mcolor);
                }
                enabled: true
            }
        }


    }



}
}





