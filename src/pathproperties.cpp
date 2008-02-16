/***************************************************************************
 *   This file is part of QSvn Project http://www.anrichter.net/projects/qsvn   *
 *   Copyright (c) 2004-2007 Andreas Richter <ar@anrichter.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License Version 2        *
 *   as published by the Free Software Foundation.                         *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/

//QSvn
#include "config.h"
#include "pathproperties.h"
#include "pathpropertiesdelegate.h"
#include "pathpropertiesmodel.h"
#include "statustext.h"
#include "svnclient.h"

#include "svnqt/status.hpp"

//Qt
#include <QtGui>


PathProperties::PathProperties(QObject *parent, const QString path)
    : QDialog(0)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setupUi(this);

    addButton = new QPushButton(this);
    addButton->setText(tr("Add"));
    this->buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addButtonClicked()));

    deleteButton = new QPushButton(this);
    deleteButton->setText(tr("Delete"));
    this->buttonBox->addButton(deleteButton, QDialogButtonBox::ActionRole);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteButtonClicked()));

    this->setWindowTitle(QString(tr("Edit Properties for %1")).arg(path));

    m_model = new PathPropertiesModel(path);
    viewPathProperties->setModel(m_model);
    viewPathProperties->setItemDelegateForColumn(0, &delegate);

    Config::instance()->restoreWidget(this);
    Config::instance()->restoreHeaderView(this, viewPathProperties->header());
}


PathProperties::~PathProperties()
{
    Config::instance()->saveWidget(this);
    Config::instance()->saveHeaderView(this, viewPathProperties->header());
    delete(m_model);
}

void PathProperties::doPathProperties(QWidget *parent, const QString path)
{
    svn::StatusPtr status = SvnClient::instance()->singleStatus(path);
    if (status->isVersioned())
    {
        PathProperties *pathProperties = new PathProperties(parent, path);
        pathProperties->show();
        pathProperties->raise();
        pathProperties->activateWindow();
    }
    else
    {
        StatusText::out(QString("Path %1 is not versioned").arg(path));
    }
}

void PathProperties::addButtonClicked()
{
    m_model->addProperty();
}

void PathProperties::accept()
{
    m_model->writeProperties();
    QDialog::accept();
}

void PathProperties::deleteButtonClicked()
{
    QItemSelectionModel *selection = viewPathProperties->selectionModel();
    m_model->deleteProperty(*selection);
}

#include "pathproperties.moc"
