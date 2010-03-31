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

#include "ModifyPrinter.h"

#include "QCups.h"
#include <cups/cups.h>

#include <KDebug>

using namespace QCups;

ModifyPrinter::ModifyPrinter(const QString &destName, bool isClass, QWidget *parent)
 : PrinterPage(parent), m_destName(destName), m_isClass(isClass), m_changes(0)
{
    setupUi(this);

    connectionL->setVisible(!isClass);
    connectionLE->setVisible(!isClass);
    makeModelCB->setVisible(!isClass);

    membersL->setVisible(isClass);
    membersLV->setVisible(isClass);

    m_model = new QStandardItemModel(membersLV);
    membersLV->setModel(m_model);

    connect(nameLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(locationLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(connectionLE, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(modelChanged()));
}

ModifyPrinter::~ModifyPrinter()
{
}

void ModifyPrinter::setValues(const QHash<QString, QVariant> &values)
{
    if (m_isClass) {
        QList<Destination> dests;
        // Ask just these attributes
        QStringList requestAttr;
        requestAttr << "printer-uri-supported"
                    << "printer-name";
        // Get destinations with these masks
        dests = QCups::getDests(CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE |
                                CUPS_PRINTER_IMPLICIT, requestAttr);
        m_model->clear();
        QStringList memberNames = values["member-names"].toStringList();
        QStringList origMemberUris;
        foreach (const QString &memberUri, memberNames) {
            for (int i = 0; i < dests.size(); i++) {
                if (dests.at(i)["printer-name"].toString() == memberUri) {
                    origMemberUris << dests.at(i)["printer-uri-supported"].toString();
                    break;
                }
            }
        }
        m_model->setProperty("orig-member-uris", origMemberUris);

        for (int i = 0; i < dests.size(); i++) {
            QString destName = dests.at(i)["printer-name"].toString();
            if (destName != m_destName) {
                QStandardItem *item = new QStandardItem(destName);
                item->setCheckable(true);
                item->setEditable(false);
                if (memberNames.contains(destName)) {
                    item->setCheckState(Qt::Checked);
                }
                item->setData(dests.at(i)["printer-uri-supported"].toString());
                m_model->appendRow(item);
            }
        }
    }

    nameLE->setText(values["printer-info"].toString());
    nameLE->setProperty("orig_text", values["printer-info"].toString());

    locationLE->setText(values["printer-location"].toString());
    locationLE->setProperty("orig_text", values["printer-location"].toString());

    connectionLE->setText(values["device-uri"].toString());
    connectionLE->setProperty("orig_text", values["device-uri"].toString());

    makeCB->addItem(values["printer-make-and-model"].toString());

    // clear old values
    m_changes = 0;
    m_changedValues.clear();
    nameLE->setProperty("different", false);
    locationLE->setProperty("different", false);
    connectionLE->setProperty("different", false);
    m_model->setProperty("different", false);
    emit changed(0);
}

void ModifyPrinter::modelChanged()
{
    QStringList currentMembers;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QStandardItem *item = m_model->item(i);
        if (item && item->checkState() == Qt::Checked) {
            currentMembers << item->data().toString();
        }
    }
    currentMembers.sort();

    bool isDifferent = m_model->property("orig-member-uris").toStringList() != currentMembers;
    if (isDifferent != m_model->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        m_model->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    // store the new values
    if (isDifferent) {
        m_changedValues["member-uris"] = currentMembers;
    } else {
        m_changedValues.remove("member-uris");
    }
}

void ModifyPrinter::textChanged(const QString &text)
{
    KLineEdit *le = qobject_cast<KLineEdit *>(sender());

    bool isDifferent = le->property("orig_text") != text;
    if (isDifferent != le->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        le->setProperty("different", isDifferent);
        emit changed(m_changes);
    }

    // store the new values
    QString attribute = le->property("AttributeName").toString();
    if (isDifferent) {
        m_changedValues[attribute] = text;
    } else {
        m_changedValues.remove(attribute);
    }
}

void ModifyPrinter::save()
{
    if (m_changes) {
        if (Dest::setAttributes(m_destName, m_isClass, m_changedValues)) {
            setValues(Dest::getAttributes(m_destName, m_isClass, neededValues()));
        }
    }
}

QHash<QString, QVariant> ModifyPrinter::modifiedValues() const
{
    return m_changedValues;
}

bool ModifyPrinter::hasChanges()
{
    return m_changes;
}

void ModifyPrinter::setRemote(bool remote)
{
    nameLE->setReadOnly(remote);
    locationLE->setReadOnly(remote);
    connectionLE->setReadOnly(remote);
}


QStringList ModifyPrinter::neededValues() const
{
    QStringList values;
    values << "printer-info"
           << "printer-location";
    if (m_isClass) {
        values << "member-names";
    } else {
        values << "device-uri"
               << "printer-make-and-model";
    }
    return values;
}

#include "ModifyPrinter.moc"
