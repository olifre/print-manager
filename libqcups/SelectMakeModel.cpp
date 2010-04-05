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

#include "SelectMakeModel.h"
#include "PrinterPage.h"

#include "ModifyPrinter.h"
#include "PrinterBehavior.h"
#include "PrinterOptions.h"

#include <cups/cups.h>

#include <KMessageBox>
#include <KDebug>

using namespace QCups;

SelectMakeModel::SelectMakeModel(const QString &destName, bool isClass, QWidget *parent)
 : KPageDialog(parent)
{
    setFaceType(List);
    setModal(true);
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    setWindowTitle(destName);
    enableButtonApply(false);
    KConfig config("print-manager");
    KConfigGroup configureDialog(&config, "SelectMakeModel");
    restoreDialogSize(configureDialog);

    QStringList attr;
    KPageWidgetItem *page;

    ModifyPrinter *widget = new ModifyPrinter(destName, isClass, this);
    PrinterBehavior *pBW = new PrinterBehavior(destName, isClass, this);
    attr << widget->neededValues();
    attr << pBW->neededValues();
    attr << "printer-type"; // needed to know if it's a remote printer
    attr.removeDuplicates();
    QHash<QString, QVariant> values = Dest::getAttributes(destName, isClass, attr);

    kDebug() << values;
 if (values["printer-type"].toUInt() & CUPS_PRINTER_LOCAL) {
     kDebug() << "CUPS_PRINTER_LOCAL";
 }
 if (values["printer-type"].toUInt() & CUPS_PRINTER_CLASS) {
     kDebug() << "CUPS_PRINTER_CLASS";
 }
 bool isRemote = false;
 if (values["printer-type"].toUInt() & CUPS_PRINTER_REMOTE) {
     kDebug() << "CUPS_PRINTER_REMOTE";
     isRemote = true;
 }
 if (values["printer-type"].toUInt() & CUPS_PRINTER_BW) {
     kDebug() << "CUPS_PRINTER_BW";
 }
  if (values["printer-type"].toUInt() & CUPS_PRINTER_COLOR) {
     kDebug() << "CUPS_PRINTER_COLOR";
 }
  if (values["printer-type"].toUInt() & CUPS_PRINTER_MFP) {
     kDebug() << "CUPS_PRINTER_MFP";
 }

    widget->setRemote(isRemote);
    widget->setValues(values);
    page = new KPageWidgetItem(widget, i18n("Modify Printer"));
    page->setHeader(i18n("Configure"));
    page->setIcon(KIcon("dialog-information"));
    // CONNECT this signal ONLY to the first Page
    connect(widget, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)));
    addPage(page);

    if (!isClass) {
        // At least on localhost:631 modify printer does not show printer options
        // for classes
        PrinterOptions *pOp = new PrinterOptions(destName, isClass, isRemote, this);
        page = new KPageWidgetItem(pOp, i18n("Printer Options"));
        page->setHeader(i18n("Set the Default Printer Options"));
        page->setIcon(KIcon("view-pim-tasks"));
        addPage(page);
    }

    pBW->setRemote(isRemote);
    pBW->setValues(values);
    page = new KPageWidgetItem(pBW, i18n("Banners, Policies and Allowed Users"));
    page->setHeader(i18n("Banners, Policies and Allowed Users"));
    page->setIcon(KIcon("feed-subscribe"));
    addPage(page);

    // connect this after ALL pages were added, otherwise the slot will be called
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)),
            SLOT(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)));
    restoreDialogSize(configureDialog);
}

SelectMakeModel::~SelectMakeModel()
{
    KConfig config("print-manager");
    KConfigGroup configureDialog(&config, "SelectMakeModel");
    saveDialogSize(configureDialog);
}

void SelectMakeModel::currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before)
{
    PrinterPage *currentPage = qobject_cast<PrinterPage*>(current->widget());
    PrinterPage *beforePage = qobject_cast<PrinterPage*>(before->widget());

    // Check if the before page has changes
    savePage(beforePage);
    if (beforePage) {
        disconnect(beforePage, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)));
    }

    // connect the changed signal to the new page and check if it has changes
    connect(currentPage, SIGNAL(changed(bool)), this, SLOT(enableButtonApply(bool)));
    enableButtonApply(currentPage->hasChanges());
}

void SelectMakeModel::slotButtonClicked(int button)
{
    PrinterPage *page = qobject_cast<PrinterPage *>(currentPage()->widget());
    if (button == KDialog::Ok) {
        page->save();
        accept();
    } else if (button == KDialog::Apply) {
        page->save();
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void SelectMakeModel::closeEvent(QCloseEvent *event)
{
    PrinterPage *page = qobject_cast<PrinterPage*>(currentPage()->widget());
    if (savePage(page)) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool SelectMakeModel::savePage(PrinterPage *page)
{
    if (page->hasChanges()) {
        int ret;
        ret = KMessageBox::warningYesNoCancel(this,
                                               i18n("The current page has changes.\n"
                                                    "Do you want to save them?"));
        if (ret == KMessageBox::Yes) {
            page->save();
        } else if (ret == KMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

#include "SelectMakeModel.moc"
