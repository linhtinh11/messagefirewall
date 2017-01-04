/*
 * Copyright (c) 2013-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import bb.cascades 1.4

TabbedPane {
    id: mainTab
    showTabsOnActionBar: true

    Tab {
        title: qsTr("General")
        Page {
            Container {
                layout: StackLayout {

                }
                Container {
                    layout: DockLayout {

                    }
                    horizontalAlignment: HorizontalAlignment.Fill

                    leftPadding: 10.0
                    rightPadding: 10.0
                    topPadding: 10.0
                    Label {
                        text: qsTr("Activate Email Firewall")
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                    ToggleButton {
                        checked: _app.getToggleValue(1)
                        horizontalAlignment: HorizontalAlignment.Right
                        onCheckedChanged: {
                            _app.changedToggleValue(1, checked)
                        }
                    }
                }

                Container {
                    layout: DockLayout {
                    
                    }
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    leftPadding: 10.0
                    rightPadding: 10.0
                    topPadding: 10.0
                    Label {
                        text: qsTr("Activate SMS Firewall")
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                    ToggleButton {
                        checked: _app.getToggleValue(2)
                        horizontalAlignment: HorizontalAlignment.Right
                        onCheckedChanged: {
                            _app.changedToggleValue(2, checked)
                        }
                    }
                }
                
                Container {
                    layout: DockLayout {
                    
                    }
                    horizontalAlignment: HorizontalAlignment.Fill
                    
                    leftPadding: 10.0
                    rightPadding: 10.0
                    topPadding: 10.0
                    Label {
                        text: qsTr("Activate Phone Firewall")
                        verticalAlignment: VerticalAlignment.Center
                        horizontalAlignment: HorizontalAlignment.Left
                    }
                    ToggleButton {
                        checked: _app.getToggleValue(3)
                        horizontalAlignment: HorizontalAlignment.Right
                        onCheckedChanged: {
                            _app.changedToggleValue(3, checked)
                        }
                    }
                }
                
            }

        }

    }
    
    Tab {
        title: qsTr("Email")
        
        delegate: Delegate {
            source: "EmailSettings.qml"
        }
    }

}