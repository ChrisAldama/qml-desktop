/*
 * QML Desktop - Set of tools written in C++ for QML
 * Copyright (C) 2014 Bogdan Cuza <bogdan.cuza@hotmail.com>
 * also Copyright (C) 2014 Ricardo Vieira <ricardo.vieira@tecnico.ulisboa.pt>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MPRIS2PLAYER_H
#define MPRIS2PLAYER_H

#include <QDBusInterface>
#include <QMap>
#include <QVariantMap>
#include <QDBusArgument>

class Mpris2Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap metadata READ metadata NOTIFY metadataNotify)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString playbackStatus READ playbackStatus NOTIFY playbackStatusChanged)

public:

    explicit Mpris2Player(QString serviceName, QObject *parent = 0) : QObject(parent),
        iface(serviceName,
                         "/org/mpris/MediaPlayer2",
                         "org.mpris.MediaPlayer2.Player"), playerInterface(serviceName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2"), name(playerInterface.property("Identity").toString()) {
        QDBusConnection::sessionBus().connect(serviceName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(metadataReceived(QDBusMessage)));
    }

    QVariantMap metadata() const {
        QDBusInterface iface(this->iface.service(), "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties");
        QDBusMessage result = iface.call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
        const QDBusArgument &arg = result.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>();
        arg.beginMap();
        QVariantMap map;
        while (!arg.atEnd()) {
            QVariant var;
            QString str;
            arg.beginMapEntry();
            arg >> str >> var;
            arg.endMapEntry();
            map.insert(str, var);
          }
         arg.endMap();
         return map;
    }

    QString playbackStatus() const {
        return QDBusInterface(this->iface.service(), "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties").call("Get", "org.mpris.MediaPlayer2.Player", "PlaybackStatus").arguments().at(0).value<QDBusVariant>().variant().toString();
    }

    Q_INVOKABLE void playPause() {
        iface.call("PlayPause");
    }

    Q_INVOKABLE void next() {
        iface.call("Next");
    }
    Q_INVOKABLE void previous() {
        iface.call("Previous");
    }

    Q_INVOKABLE void stop() {
        iface.call("Stop");
    }

    Q_INVOKABLE void seek(const QVariant &position) {
        iface.call("Seek", position.toLongLong());
    }

    Q_INVOKABLE void openUri(const QVariant &uri) {
        iface.call("OpenUri", uri.toString());
    }

    Q_INVOKABLE void raise() {
        playerInterface.call("Raise");
    }

    Q_INVOKABLE void quit() {
        playerInterface.call("Quit");
    }


    QDBusInterface iface;
    QDBusInterface playerInterface;
    QString name;
signals:
    void metadataNotify();
    void playbackStatusChanged();
private slots:
    void metadataReceived(QDBusMessage msg) {
        const QDBusArgument &arg = msg.arguments().at(1).value<QDBusArgument>();
        arg.beginMap();
        while (!arg.atEnd()) {
            QString key;
            arg.beginMapEntry();
            arg >> key;
            if (key == "Metadata") {
                emit metadataNotify();
            } else if (key == "PlaybackStatus") {
                emit playbackStatusChanged();
            }
            arg.endMapEntry();
        }
        arg.endMap();
    }
};

#endif // MPRIS2PLAYER_H
