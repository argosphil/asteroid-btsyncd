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

#ifndef SHELLSERVICE_H
#define SHELLSERVICE_H

#include <QObject>

#include "service.h"

class ShellReqChrc : public Characteristic
{
    Q_OBJECT
public:
    ShellReqChrc(QDBusConnection bus, int index, Service *service)
        : Characteristic(bus, index, SHELL_CMD_UUID, {"encrypt-authenticated-write"}, service) {}

public slots:
    void WriteValue(QByteArray, QVariantMap);
};


class ShellResponseChrc : public Characteristic
{
    Q_OBJECT
    Q_PROPERTY(QByteArray Value READ getValue NOTIFY valueChanged)
public:
    ShellResponseChrc(QDBusConnection bus, int index, Service *service) : Characteristic(bus, index, SCREENSH_CON_UUID, {"encrypt-authenticated-read", "notify"}, service)
    {}

public slots:
    QByteArray ReadValue(QVariantMap)
    {
        return m_value;
    }

    void StartNotify() {}
    void StopNotify() {}

private slots:
    void emitPropertiesChanged();
    void onShellTaken(QString);

signals:
    void valueChanged();

private:
    QByteArray m_value;

    QByteArray getValue()
    {
        return m_value;
    }
};

class ShellService : public Service
{
    Q_OBJECT
public:
    explicit ShellService(int index, QDBusConnection bus, QObject *parent = 0);
};

#endif // SHELLSERVICE_H
