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

#ifndef CHOOSE_URI_H
#define CHOOSE_URI_H

#include "ui_ChooseUri.h"

#include "GenericPage.h"
#include <QRegExp>

class ChooseUri : public GenericPage, Ui::ChooseUri
{
    Q_OBJECT
public:
    ChooseUri(QWidget *parent = 0);
    ~ChooseUri();

    void setValues(const QVariantHash &args);
    QVariantHash values() const;
    bool isValid() const;
    bool canProceed() const;

public slots:
    void load();

private slots:
    void checkSelected();

private:
    bool m_isValid;
};

#endif
