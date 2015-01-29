/* 
 * QML Desktop - Set of tools written in C++ for QML
 * 
 * Copyright (C) 2014 Bogdan Cuza <bogdan.cuza@hotmail.com>
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

#include "desktopscrobbler.h"

DesktopScrobbler::DesktopScrobbler(QQuickItem *parent) : QQuickItem(parent) {
    QStringList entries;
    QStringList globalEntries = QDir("/usr/local/share/applications").entryList(QStringList() << "*.desktop");
    QStringList anotherGlobalEntries = QDir("/usr/share/applications").entryList(QStringList() << "*.desktop");
    QStringList localEntries = QDir("~/.local/share/applications").entryList(QStringList() << "*.desktop");
    for (int i = 0; i < globalEntries.length(); i++) {
        entries << QString("/usr/local/share/applications/%1").arg(globalEntries.at(i));
    }
    for (int i = 0; i < anotherGlobalEntries.length(); i++) {
        entries << QString("/usr/share/applications/%1").arg(anotherGlobalEntries.at(i));
    }
    for (int i = 0; i < localEntries.length(); i++) {
        entries << QString("~/.local/share/applications/%1").arg(localEntries.at(i));
    }
    for (int i = 0; i < entries.length(); i++){
        desktopList << new DesktopFile(entries.at(i));
    }
    QFileSystemWatcher fileWatcher(entries);
    QFileSystemWatcher *dirWatcher = new QFileSystemWatcher(QStringList() << "/usr/local/share/applications" << "/usr/share/applications" << "~/.local/share/applications");
    connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &DesktopScrobbler::processFileModification);
    connect(dirWatcher, &QFileSystemWatcher::directoryChanged, this, &DesktopScrobbler::processDirChange);
}

void DesktopScrobbler::processFileModification(const QString &path) {
    if (QFile::exists(path)){
        for (int i = 0; i < desktopList.length(); i++){
            if (desktopList.at(i)->property("location").toString() == path){
                desktopList.replace(i, new DesktopFile(path));
                break;
            }
        }
    } else {
        for (int i = 0; i < desktopList.length(); i++){
            if (desktopList.at(i)->property("location").toString() == path){
                desktopList.removeAt(i);
                fileWatcher.removePath(path);
                break;
            }
        }
    }
    emit desktopFilesChanged(desktopFiles());
}

void DesktopScrobbler::processDirChange(const QString &path){
    QStringList entryList = QDir(path).entryList(QStringList() << "*.desktop");
    QStringList currentPath;
    for (int i = 0; i < desktopList.length(); i++){
        if (desktopList[i]->m_location.startsWith(path)){
            currentPath << desktopList[i]->m_location;
        }
    }
    if (currentPath.length() != entryList.length()) {
        for (int i = 0; i < entryList.length(); i++){
            if (!currentPath.contains(path + "/" + entryList[i])) {
                desktopList << new DesktopFile(path + "/" + entryList[i]);
                emit desktopFilesChanged(desktopFiles());
                fileWatcher.addPath(path + "/" + entryList[i]);
                connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &DesktopScrobbler::processFileModification);
            }
        }
    }
}
