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

#include "PrinterModel.h"

#include <QDateTime>
#include <QMimeData>
#include <KUser>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <QCupsStrings.h>

#include <KCupsRequestServer.h>

#include <cups/cups.h>

PrinterModel::PrinterModel(WId parentId, QObject *parent)
 : QStandardItemModel(parent),
   m_parentId(parentId)
{
    // setup the timer that updates the UIs
    m_updateT = new QTimer(this);
    m_updateT->setInterval(1000);
    connect(m_updateT, SIGNAL(timeout()), this, SLOT(update()));
    m_updateT->start();
}

void PrinterModel::getDestsFinished()
{
    KCupsRequestServer *request = qobject_cast<KCupsRequestServer *>(sender());
    if (request) {
        if (request->hasError()) {
            emit error(request->error(), QCups::serverError(request->error()), request->errorMsg());
            // clear the model after so that the proper widget can be shown
            clear();
        } else {
            // remove old printers
            // The above code starts from 0 and make sure
            // dest == modelIndex(x) and if it's not the
            // case it either inserts or moves it.
            // so any item > num_jobs can be safely deleted
            while (rowCount() > request->result().size()) {
                removeRow(rowCount() - 1);
            }

            request->deleteLater();
            emit error(IPP_OK, QString(), QString());
        }
    } else {
        kWarning() << "Should not be called from a non KCupsRequestServer class" << sender();
    }
}

QVariant PrinterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return i18n("Printers");
    }
    return QVariant();
}

void PrinterModel::update()
{
    QStringList requestAttr;
    requestAttr << "printer-name"
                << "printer-state"
                << "printer-state-message"
                << "printer-is-shared"
                << "printer-type"
                << "printer-location"
                << "printer-info"
                << "printer-make-and-model"
                << "printer-commands"
                << "marker-change-time"
                << "marker-colors"
                << "marker-levels"
                << "marker-names"
                << "marker-types";

//                 kcmshell(6331) PrinterModel::update: (QHash(("printer-type", QVariant(int, 75534348) ) ( "marker-names" ,  QVariant(QStringList, ("Cyan", "Yellow", "Magenta", "Black") ) ) ( "printer-name" ,  QVariant(QString, "EPSON_Stylus_TX105") ) ( "marker-colors" ,  QVariant(QStringList, ("#00ffff", "#ffff00", "#ff00ff", "#000000") ) ) ( "printer-location" ,  QVariant(QString, "Luiz Vitor’s MacBook Pro") ) ( "marker-levels" ,  QVariant(QList<int>, ) ) ( "marker-types" ,  QVariant(QStringList, ("inkCartridge", "inkCartridge", "inkCartridge", "inkCartridge") ) ) ( "printer-is-shared" ,  QVariant(bool, true) ) ( "printer-state-message" ,  QVariant(QString, "") ) ( "printer-commands" ,  QVariant(QStringList, ("Clean", "PrintSelfTestPage", "ReportLevels") ) ) ( "marker-change-time" ,  QVariant(int, 1267903160) ) ( "printer-state" ,  QVariant(int, 3) ) ( "printer-info" ,  QVariant(QString, "EPSON Stylus TX105") ) ( "printer-make-and-model" ,  QVariant(QString, "EPSON TX105 Series") ) )  )
    // Get destinations with these attributes
    KCupsRequestServer *request = new KCupsRequestServer;
    connect(request, SIGNAL(printer(int,KCupsPrinter)), this, SLOT(printer(int,KCupsPrinter)));
    connect(request, SIGNAL(finished()), this, SLOT(getDestsFinished()));
    request->getPrinters(requestAttr);
}

void PrinterModel::printer(int pos, const KCupsPrinter &printer)
{
    // If there is a printer and it's not the current one add it
    // as a new destination
    int dest_row = destRow(printer.name());
    if (dest_row == -1) {
        // not found, insert new one
        insertDest(pos, printer);
    } else if (dest_row == pos) {
        // update the printer
        updateDest(item(pos), printer);
    } else {
        // found at wrong position
        // take it and insert on the right position
        QList<QStandardItem *> row = takeRow(dest_row);
        insertRow(pos, row);
        updateDest(item(pos), printer);
    }
}

void PrinterModel::insertDest(int pos, const KCupsPrinter &printer)
{
    // Create the printer item
    QStandardItem *stdItem = new QStandardItem(printer.name());
    stdItem->setData(printer.name(), DestName);
    stdItem->setIcon(printer.icon());
    // update the item
    updateDest(stdItem, printer);

    // insert the printer Item
    insertRow(pos, stdItem);
}

void PrinterModel::updateDest(QStandardItem *destItem, const KCupsPrinter &printer)
{
    // store if the printer is the network default
    bool isDefault = printer.isDefault();
    if (isDefault != destItem->data(DestIsDefault).toBool()) {
        destItem->setData(isDefault, DestIsDefault);
    }

    // store the printer state
    QString status = destStatus(printer.state(), printer.stateMsg());
    if (status != destItem->data(DestStatus)) {
        destItem->setData(status, DestStatus);
    }

    // store if the printer is shared
    bool shared = printer.isShared();
    if (shared != destItem->data(DestIsShared)) {
        destItem->setData(shared, DestIsShared);
    }

    // store if the printer is a class
    // the printer-type param is a flag
    bool isClass = printer.isClass();
    if (isClass != destItem->data(DestIsClass)) {
        destItem->setData(isClass, DestIsClass);
    }

    // store if the printer type
    // the printer-type param is a flag
    uint printerType = printer.type();
    if (printerType != destItem->data(DestType)) {
        destItem->setData(printerType, DestType);
    }

    // store the printer location
    QString location = printer.location();
    if (location != destItem->data(DestLocation).toString()) {
        destItem->setData(location, DestLocation);
    }

    if (destItem->data(DestName).toString() != destItem->text()){
        if (destItem->text() != destItem->data(DestName).toString()){
            destItem->setText(destItem->data(DestName).toString());
        }
    }

    // store the printer description
    QString description = printer.description();
    if (description != destItem->data(DestDescription).toString()){
        destItem->setData(description, DestDescription);
    }

    // store the printer kind
    QString kind = printer.makeAndModel();
    if (kind != destItem->data(DestKind)) {
        destItem->setData(kind, DestKind);
    }

    // store the printer commands
    QStringList commands = printer.commands();
    if (commands != destItem->data(DestCommands)) {
        destItem->setData(commands, DestCommands);
    }

    int markerChangeTime = printer.markerChangeTime();
    if (markerChangeTime != destItem->data(DestMarkerChangeTime)) {
        destItem->setData(markerChangeTime, DestMarkerChangeTime);
        destItem->setData(printer.argument("marker-colors"), DestMarkerColors);
        destItem->setData(printer.argument("marker-levels"), DestMarkerLevels);
        destItem->setData(printer.argument("marker-names"),  DestMarkerNames);
        destItem->setData(printer.argument("marker-types"),  DestMarkerTypes);
    }
}

int PrinterModel::destRow(const QString &destName)
{
    // find the position of the jobId inside the model
    for (int i = 0; i < rowCount(); i++) {
        if (destName == item(i)->data(DestName).toString())
        {
            return i;
        }
    }
    // -1 if not found
    return -1;
}

QString PrinterModel::destStatus(int state, const QString &message) const
{
    switch (state) {
    case DEST_IDLE :
        if (message.isEmpty()){
            return i18n("Idle");
        } else {
            return i18n("Idle - '%1'", message);
        }
    case DEST_PRINTING :
        if (message.isEmpty()){
            return i18n("In use");
        } else {
            return i18n("In use - '%1'", message);
        }
    case DEST_STOPED :
        if (message.isEmpty()){
            return i18n("Paused");
        } else {
            return i18n("Paused - '%1'", message);
        }
    default :
        if (message.isEmpty()){
            return i18n("Unknown");
        } else {
            return i18n("Unknown - '%1'", message);
        }
    }
}

Qt::ItemFlags PrinterModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

#include "PrinterModel.moc"
