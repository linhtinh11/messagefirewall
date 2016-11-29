import bb.cascades 1.4

Page {
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.TopToBottom

        }
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight

            }
            horizontalAlignment: HorizontalAlignment.Left
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1.0

            }
            Label {
                text: qsTr("Active Email Firewall")

            }

            ToggleButton {
                id: emailActivated
                checked: true

            }

        }

    }
}
