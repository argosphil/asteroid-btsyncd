/*
 * Copyright (C) 2017 - Florent Revest <revestflo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>

#include "shellservice.h"
#include "characteristic.h"
#include "common.h"

#include <unistd.h>

void ShellReqChrc::WriteValue(QByteArray ba, QVariantMap vm)
{
  fprintf(stderr, "shellreqchrc::writevalue\n", ba, vm);
  QString string(ba);
  std::string stdstring = string.toStdString();
  const char *cstr = stdstring.c_str();
  system(cstr);
    QList<QVariant> argumentList;
    argumentList << "/tmp/btsyncd-shell.jpg";
    //static QDBusInterface notifyApp(SHELL_SERVICE_NAME, SHELL_PATH_BASE, SHELL_MAIN_IFACE, QDBusConnection::systemBus());
    //QDBusMessage reply = notifyApp.callWithArgumentList(QDBus::AutoDetect, "saveShell", argumentList);
    //if(reply.type() == QDBusMessage::ErrorMessage)
    //  fprintf(stderr, "ShellReqChrc::WriteValue: D-Bus Error: %s\n", reply.errorMessage().toStdString().c_str());
    //emit shellTaken("/tmp/btsyncd-shell.jpg");
}

void ShellResponseChrc::onShellTaken(QString path)
{
    QFile f(path);
    if(!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << path;
        m_value = QByteArray::number(0x0);
        emit valueChanged();
        emitPropertiesChanged();
        return;
    }

    qint64 totalSize = f.bytesAvailable();
    m_value = QByteArray();
    m_value.append((totalSize >> 0) & 0xFF);
    m_value.append((totalSize >> 8) & 0xFF);
    m_value.append((totalSize >> 16) & 0xFF);
    m_value.append((totalSize >> 24) & 0xFF);
    emit valueChanged();
    emitPropertiesChanged();

    while (!f.atEnd()) {
        m_value = f.read(20);
        emit valueChanged();
        emitPropertiesChanged();
    }
    f.close();
}

void ShellResponseChrc::emitPropertiesChanged()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    QDBusMessage message = QDBusMessage::createSignal(getPath().path(),
                                                      "org.freedesktop.DBus.Properties",
                                                      "PropertiesChanged");

    QVariantMap changedProperties;
    changedProperties[QStringLiteral("Value")] = QVariant(m_value);

    QList<QVariant> arguments;
    arguments << QVariant(GATT_CHRC_IFACE) << QVariant(changedProperties) << QVariant(QStringList());
    message.setArguments(arguments);

    if (!connection.send(message))
        qDebug() << "Failed to send DBus property notification signal";
}

ShellService::ShellService(int index, QDBusConnection bus, QObject *parent) : Service(bus, index, SHELL_UUID, parent)
{
    ShellReqChrc *reqChrc = new ShellReqChrc(bus, 0, this);
    ShellResponseChrc *respChrc = new ShellResponseChrc(bus, 1, this);

    // connect(reqChrc, SIGNAL(shellTaken(QString)), contChrc, SLOT(onShellTaken(QString)));

    addCharacteristic(reqChrc);
    addCharacteristic(respChrc);
}
