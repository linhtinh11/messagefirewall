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

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/system/InvokeManager>
#include <QSettings>

using namespace bb::cascades;
using namespace bb::system;

const QString ApplicationUI::m_author = "ht"; // for creating settings
const QString ApplicationUI::m_appName = "MessageFirewall"; // for creating settings

// keys for setting file
const QString ApplicationUI::m_s_activeEmail = "ActiveEmail";
const QString ApplicationUI::m_s_activeSMS = "ActiveSMS";
const QString ApplicationUI::m_s_activePhone = "ActivePhone";

ApplicationUI::ApplicationUI() :
        QObject(),
        m_translator(new QTranslator(this)),
        m_localeHandler(new LocaleHandler(this)),
        m_invokeManager(new InvokeManager(this))
{
    // prepare the localization
    if (!QObject::connect(m_localeHandler, SIGNAL(systemLanguageChanged()),
            this, SLOT(onSystemLanguageChanged()))) {
        // This is an abnormal situation! Something went wrong!
        // Add own code to recover here
        qWarning() << "Recovering from a failed connect()";
    }

    // initial load
    onSystemLanguageChanged();

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    // Make app available to the qml.
    qml->setContextProperty("_app", this);

    // Create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    // Set created root object as the application scene
    Application::instance()->setScene(root);
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_translator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("MessageFirewall_%1").arg(locale_string);
    if (m_translator->load(file_name, "app/native/qm")) {
    QCoreApplication::instance()->installTranslator(m_translator);
    }
}

void ApplicationUI::resendNotification()
{
    InvokeRequest request;
    request.setTarget("com.example.MessageFirewallService");
    request.setAction("com.example.MessageFirewallService.RESET");
    m_invokeManager->invoke(request);
    Application::instance()->minimize();
}

void ApplicationUI::changedToggleValue(int button, bool value)
{
    QSettings settings(m_author, m_appName);
    qDebug() << "button: " << button << " changed: " << value;
    switch(button) {
        case 1:
            settings.setValue(m_s_activeEmail, value);
            break;
        case 2:
            settings.setValue(m_s_activeSMS, value);
            break;
        case 3:
            settings.setValue(m_s_activePhone, value);
            break;
    }
    settings.sync();
}

bool ApplicationUI::getToggleValue(int button)
{
    QSettings settings(m_author, m_appName);
    bool result = true;
    switch(button) {
        case 1:
            result = settings.value(m_s_activeEmail, true).toBool();
            break;
        case 2:
            result = settings.value(m_s_activeSMS, true).toBool();
            break;
        case 3:
            result = settings.value(m_s_activePhone, true).toBool();
            break;
    }
    return result;
}
