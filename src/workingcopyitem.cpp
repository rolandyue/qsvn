/***************************************************************************
 *   Copyright (C) 2004 by Andreas Richter                                 *
 *   ar@oszine.de                                                          *
 *   http://www.oszine.de                                                  *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 *   As a special exception, permission is given to link this program      *
 *   with any edition of Qt, and distribute the resulting executable,      *
 *   without including the source code for Qt in the source distribution.  *
 ***************************************************************************/

//QSvn
#include "workingcopyitem.h"
#include "svnclient.h"

//Qt
#include <qlistview.h>
#include <qstring.h>
#include <qurl.h>


WorkingCopyItem::WorkingCopyItem( QListViewItem* parent )
    : QListViewItem( parent )
{
    parentItem = 0;
    svnDirectory = FALSE;
}

WorkingCopyItem::WorkingCopyItem( QListView* parent, QString directory )
    : QListViewItem( parent )
{
    parentItem = 0;
    stringFullPath = directory;
    setText( 0, directory );  
    svnDirectory = SvnClient::Exemplar()->isWorkingCopy( directory );
}

WorkingCopyItem::WorkingCopyItem( WorkingCopyItem* parent, QString directory )
    : QListViewItem( parent )
{
    parentItem = parent;
    stringFullPath = directory;
    QUrl url( directory );
    setText( 0, url.fileName() );
    svnDirectory = SvnClient::Exemplar()->isWorkingCopy( directory );
}

WorkingCopyItem::~WorkingCopyItem()
{}

WorkingCopyItem* WorkingCopyItem::parent() const
{
    return parentItem;
}

QString WorkingCopyItem::fullPath()
{
    return stringFullPath;
}

bool WorkingCopyItem::isSvnDirectory()
{
    return svnDirectory;
}