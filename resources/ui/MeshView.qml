import QtQuick 2.0
import QtQml.Models 2.2

import QtQuick.Shapes 1.0

Rectangle {
    anchors.centerIn: parent
    id: container
    anchors.fill: parent
    color: "#ffffff" //"#e2e2e2"

    //border.color: "black"
    //border.width: 1
    //radius: 1
    property real eleWidth: elements.getWidth()
    property real totalHeight: elements.getTotalHeight()
    property real parentHeight: height //500//320

    Rectangle {

        property real totalSoilHeight: elements.getTotalHeight()
        property real startYLoc: 0.15 * 0.8 * container.height
        property real soilWidth: elements.getWidth(
                                     ) * 0.8 * container.height / elements.getTotalHeight(
                                     ) // eleWidth*hFactor //width*0.3
        //property real startXLoc: (container.width-elements.getWidth()*0.8* container.height / elements.getTotalHeight())/2 //(1-0.3)*width/2
        property real startXLoc: (container.width * 0.5) / 2 //(1-0.3)*width/2

        id: innerRect
        x: startXLoc
        y: startYLoc
        width: parent.width
        height: parent.height * 0.8

        //color: "#000000"//"#e2e2e2"
        ListView {
            width: elements.getWidth(
                       ) * 0.8 * container.height / elements.getTotalHeight()
            height: container.height * 0.8

            model: elements
            delegate: Rectangle {

                property real hFactor: 0.8 * container.height / elements.getTotalHeight()
                //property real soilWidth: elements.getWidth()*0.8* container.height / elements.getTotalHeight()
                property real soilWidth: (container.width * 0.5)

                height: h * hFactor
                width: soilWidth
                color: mcolor
                border.color: active ? "red" : "black"
                border.width: active ? 2 : 0.5
                property bool active2: active

                Rectangle {
                    x: soilWidth + 10
                    y: -5
                    width: 10
                    height: 10
                    color: "white"
                    radius: 10 / 2
                }



                property real startXGWT: (width - soilWidth) / 2
                property real startYGWT: 0.0//(theGWT + 0.15 * elements.getTotalHeight(

                // right arrow
                Shape {

                    // water table
                    ShapePath {
                        strokeWidth: 2
                        strokeColor: active ? "blue" : "white"
                        strokeStyle: ShapePath.SolidLine
                        dashPattern: [1, 2]

                        startX: soilWidth+1 + 5
                        startY: -5
                        PathLine {
                            x: soilWidth+1
                            y: 0
                        }
                        PathLine {
                            x: soilWidth+1 +5
                            y: 5
                        }


                    }
                }

                // left arrow
                Shape {

                    // water table
                    ShapePath {
                        strokeWidth: 2
                        strokeColor: active ? "blue" : "white"
                        strokeStyle: ShapePath.SolidLine
                        dashPattern: [1, 2]

                        startX: -1 - 5
                        startY: -5
                        PathLine {
                            x: -1
                            y: 0
                        }
                        PathLine {
                            x: -1 -5
                            y: 5
                        }


                    }
                }



                //Text { text: tag + " " + h + " " + totalHeight + " " + hFactor }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        elements.setActive(index)
                        //soilModel.setActive(index) ;
                        //parent.border.color = "red"
                        console.log(elements.getTotalHeight())
                    }
                    enabled: true
                }
            }
        }


    }
}
