
import QtQuick 2.0
import QtQml.Models 2.2


import QtQuick.Controls 1.4

Rectangle {
    id: container
    width: 600//parent.width
    height: 10
    color: "#e2e2e2"

    ListModel {
        id: libraryModel
        ListElement {
            title: "A Masterpiece"
            author: "Gabriel"
        }
        ListElement {
            title: "Brilliance"
            author: "Jens"
        }
        ListElement {
            title: "Outstanding"
            author: "Frederik"
        }
    }
    TableView {
        TableViewColumn {
            role: "title"
            title: "Title"
            width: 100
        }
        TableViewColumn {
            role: "author"
            title: "Author"
            width: 200
        }
        model: libraryModel
    }







}





