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

#include <QCups.h>

#include <KDebug>

using namespace QCups;

ModifyPrinter::ModifyPrinter(const QString &destName, QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    m_printer = new Printer(destName, this);
    kDebug();
    nameLE->setText(m_printer->description());
    locationLE->setText(m_printer->location());
    connectionLE->setText(m_printer->connection());

//     cups_dest_t *dests;
//     const char *value;
//     int num_dests = cupsGetDests(&dests);
//     cups_dest_t *dest = cupsGetDest(destName.toLocal8Bit(), NULL, num_dests, dests);
//     cupsFreeDests(num_dests, dests);
}

ModifyPrinter::~ModifyPrinter()
{
}

void ModifyPrinter::save()
{
    kDebug();
    m_printer->setDescription(nameLE->text());
    m_printer->setLocation(locationLE->text());
}

#include "ModifyPrinter.moc"
