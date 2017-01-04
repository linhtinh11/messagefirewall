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

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <QObject>

namespace bb {
    namespace cascades {
        class LocaleHandler;
    }
    namespace system {
        class InvokeManager;
    }
}

class QTranslator;

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI: public QObject
{
    Q_OBJECT
public:
    ApplicationUI();
    virtual ~ApplicationUI() { }

    Q_INVOKABLE void resendNotification();
    Q_INVOKABLE void changedToggleValue(int button, bool value);
    Q_INVOKABLE bool getToggleValue(int button);

private slots:
    void onSystemLanguageChanged();
private:
    QTranslator* m_translator;
    bb::cascades::LocaleHandler* m_localeHandler;
    bb::system::InvokeManager* m_invokeManager;

    // settings init
    static const QString m_author; // for creating settings
    static const QString m_appName; // for creating settings

    // keys used in setting
    static const QString m_s_activeEmail;
    static const QString m_s_activeSMS;
    static const QString m_s_activePhone;
};

#endif /* ApplicationUI_HPP_ */
