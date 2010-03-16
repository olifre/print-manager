/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef PRINT_QUEUE_TRAY_H
#define PRINT_QUEUE_TRAY_H

#include <KStatusNotifierItem>

class KMenu;

class PrintQueueTray : public KStatusNotifierItem
{
Q_OBJECT
public:
    PrintQueueTray(QObject *parent = 0);
    ~PrintQueueTray();

    void connectToLauncher(const QString &destName);
    void connectToMenu(const QList<QString> &printerList);

private slots:
    void openQueue();
    void openQueue(const QString &destName);

private:
    KMenu *m_printerMenu;
    QList<QString> m_printerList;
    QString m_destName;
};

#endif
