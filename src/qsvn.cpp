/***************************************************************************
 *   This file is part of QSvn Project http://qsvn.berlios.de              *
 *   Copyright (c) 2004-2005 Andreas Richter <ar@oszine.de>                *
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
 *   As a special exception, permission is given to link this program      *
 *   with any edition of Qt, and distribute the resulting executable,      *
 *   without including the source code for Qt in the source distribution.  *
 ***************************************************************************/


//QSvn
#include "aboutdlg.h"
#include "config.h"
#include "configure.h"
#include "filelist.h"
#include "filelistitem.h"
#include "fileselector.h"
#include "qsvn.h"
#include "statustext.h"
#include "svnclient.h"
#include "workingcopy.h"
#include "workingcopyitem.h"

//Qt
//#include <qaction.h>
#include <qapplication.h>
#include <qmessagebox.h>
//#include <qtextedit.h>
//#include <qurl.h>
#include <qwidgetstack.h>


QSvn::QSvn( QWidget *parent, const char *name )
        : QSvnDlg( parent, name )
{
    //insert WorkingCopy widget
    widgetStackWorkingCopy->addWidget( WorkingCopy::Exemplar()->getWidget() );
    widgetStackWorkingCopy->raiseWidget( WorkingCopy::Exemplar()->getWidget() );
    //insert FileList widget
    widgetStackFileList->addWidget( FileList::Exemplar()->getWidget() );
    widgetStackFileList->raiseWidget( FileList::Exemplar()->getWidget() );
    //insert StatusText widget
    widgetStackStatusText->addWidget( StatusText::Exemplar()->getWidget() );
    widgetStackStatusText->raiseWidget( StatusText::Exemplar()->getWidget() );

    Config::Exemplar()->restoreWorkingCopyEntries();

    //connect
    connect( WorkingCopy::Exemplar(), SIGNAL( directoryChanged( QString ) ), FileList::Exemplar(), SLOT( updateListSlot( QString ) ) );

    Config::Exemplar()->restoreMainWindow( this );
}

QSvn::~QSvn()
{}

void QSvn::closeEvent( QCloseEvent * )
{
    exitSlot();
}

//protected slots
void QSvn::exitSlot()
{
    Config::Exemplar()->saveChanges();
    Config::Exemplar()->saveMainWindow( this );
    FileList::releaseExemplar();
    WorkingCopy::releaseExemplar();
    qApp->exit( 0 );
}

void QSvn::addWorkingCopySlot()
{
    WorkingCopy::Exemplar()->addExistingWorkingCopySlot();
}

void QSvn::removeWorkingCopySlot()
{
    WorkingCopy::Exemplar()->removeCurrentWorkingCopySlot();
}

void QSvn::checkoutSlot()
{
    WorkingCopy::Exemplar()->checkoutSlot();
}

void QSvn::svnCommand( int svnCommandType, bool withFileSelector )
{
    //already implemented: commit, revert
    if ( !WorkingCopy::Exemplar()->selectedWorkingCopyItem() )
        return;

    bool runCommand = FALSE;

    WorkingCopyItem *item = WorkingCopy::Exemplar()->selectedWorkingCopyItem();
    QStringList *fileList = new QStringList; // QStringList with filenames for svn command
    QString commitMessage;

    if ( withFileSelector )
        FileSelector::Exemplar()->initFileSelector( svnCommandType, item->fullPath() );

    //get the fileList depend on what widget has the focus
    if ( WorkingCopy::Exemplar()->getWidget()->hasFocus() )
    {
        //WorkingCopy has focus
        SvnClient::Exemplar()->filesToList( svnCommandType, fileList, item->fullPath() );
        if ( withFileSelector )
            FileSelector::Exemplar()->setSelectedFiles( fileList );
    }
    else if ( FileList::Exemplar()->getWidget()->hasFocus() )
    {
        //FileList has focus
        fileList = FileList::Exemplar()->selectedFileListItems();
        if ( withFileSelector )
            FileSelector::Exemplar()->setSelectedFiles( fileList );
    }
    else
    {
        QMessageBox::information( this, "QSvn", tr( "Please select a Working Copy or some Files for SVN Command" ) );
        return;
    }

    if ( withFileSelector )
    {
        if ( FileSelector::Exemplar()->exec() )
        {
            fileList = FileSelector::Exemplar()->selectedFiles();
            commitMessage = FileSelector::Exemplar()->messageString();
            runCommand = TRUE;
        }
    }
    else
        runCommand = TRUE;

    if ( runCommand )
    {
        //call svnCommand
        SvnClient::Exemplar()->doSvnCommand( svnCommandType, item->fullPath(), fileList, commitMessage );
        //Updates
        WorkingCopy::Exemplar()->updateElement( item );
        FileList::Exemplar()->updateListSlot( item->fullPath() );
    }
}


void QSvn::updateSlot()
{
    svnCommand( SvnClient::Update );
}

void QSvn::commitSlot()
{
    svnCommand( SvnClient::Commit, TRUE );
}

void QSvn::addSlot()
{
    svnCommand( SvnClient::Add, TRUE );
}

void QSvn::removeSlot()
{
    svnCommand( SvnClient::Remove, TRUE );
}

void QSvn::revertSlot()
{
    svnCommand( SvnClient::Revert, TRUE );
}

void QSvn::diffSlot()
{
    SvnClient::Exemplar()->diff( WorkingCopy::Exemplar()->selectedWorkingCopyItem()->fullPath(), FileList::Exemplar()->selectedFileListItems(), FALSE );
}

void QSvn::configureQSvnSlot()
{
    Configure configure( this );
    configure.exec();
}

void QSvn::helpContentsSlot()
{
    //todo: implement
    StatusText::Exemplar()->outputMessage( QString( "Help Content - not implemented yet" ) );
}

void QSvn::helpIndexSlot()
{
    //todo: implement
    StatusText::Exemplar()->outputMessage( QString( "Help Index - not implemented yet" ) );
}

void QSvn::aboutSlot()
{
    AboutDlg about( this );
    about.exec();
}

void QSvn::aboutQtSlot()
{
    QMessageBox::aboutQt( this );
}
