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
#include <QProcess>

#include "shellservice.h"
#include "characteristic.h"
#include "common.h"

#include <unistd.h>

void ShellSendChrc::WriteValue(QByteArray ba, QVariantMap vm)
{
    if (mService2) {
	ShellService *shellService = (ShellService *) mService2;
	shellService->send(ba);
	shellService->receive(ba);
    }
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

void ShellRecvChrc::emitPropertiesChanged()
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

void ShellRecvChrc::receive(QByteArray ba)
{
    if (ba.size() > 200) {
	receive(ba.mid(0, 200));
	receive(ba.mid(200, -1));
	return;
    }
    m_value = ba;
    emit valueChanged();
    emitPropertiesChanged();
}

void ShellService::onStandardOutput()
{
    if (!mProcess) {
	return;
    }
    QByteArray ba = mProcess->readAllStandardOutput();
    mRecvChrc->receive(ba);
}

void ShellService::onStandardError()
{
    if (!mProcess) {
	return;
    }
    QByteArray ba = mProcess->readAllStandardError();
    mRecvChrc->receive(ba);
}

void ShellService::send(QByteArray ba)
{
    if (!mProcess) {
	QObject *parent;
	QString program = "/usr/bin/stdbuf";
	QStringList arguments;
	arguments << "-i0";
	arguments << "-e0";
	arguments << "-o0";
	arguments << "/bin/sh";
	arguments << "-i";
	mProcess = new QProcess(parent);
	connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(onStandardOutput()));
	connect(mProcess, SIGNAL(readyReadStandardError()), this, SLOT(onStandardError()));
	mProcess->start(program, arguments, QProcess::Unbuffered | QProcess::ReadWrite);
	if (!mProcess->waitForStarted()) {
	    delete mProcess;
	    mProcess = nullptr;
	    return;
	}
    }

    mProcess->write(ba);
}

ShellService::ShellService(int index, QDBusConnection bus, QObject *parent) : Service(bus, index, SHELL_UUID, parent)
{
    ShellSendChrc *sendChrc = new ShellSendChrc(bus, 0, this);
    ShellRecvChrc *recvChrc = new ShellRecvChrc(bus, 1, this);

    addCharacteristic(sendChrc);
    addCharacteristic(recvChrc);
    mRecvChrc = recvChrc;
}
