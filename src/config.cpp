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
 ***************************************************************************/


//QSvn
#include "config.h"
#include "qsvn.h"

//Qt
#include <QtCore>


//make Config a singleton
Config* Config::m_instance = 0;

Config* Config::instance()
{
    if ( m_instance == 0 )
    {
        m_instance = new Config;
    }
    return m_instance;
}

//Config implementation
Config::Config( QObject *parent )
        : QObject( parent )
{
    QCoreApplication::setOrganizationName( "QSvn" );
    QCoreApplication::setOrganizationDomain( "" );
    QCoreApplication::setApplicationName( "QSvn" );

    QSettings settings;

    //read saved settings
    _diffViewer = settings.value( "configuration/diffViewer", "" ).toString();
}

void Config::saveChanges()
{
    //write the entire settings in config-file
    QSettings settings;
    settings.setValue( "configuration/diffViewer", _diffViewer );
}

void Config::setDiffViewer( QString aString )
{
    if ( aString != _diffViewer )
    {
        _diffViewer = aString;
        saveChanges();
    }
}

QString Config::getDiffViewer()
{
    return _diffViewer;
}

void Config::saveMainWindow( QSvn *aQSvn )
{
    if ( aQSvn )
    {
        QSettings settings;

        settings.setValue( "mainwindow/width", aQSvn->width() );
        settings.setValue( "mainwindow/height", aQSvn->height() );
        settings.setValue( "mainwindow/x", aQSvn->x() );
        settings.setValue( "mainwindow/y", aQSvn->y() );

        //save settings from splitterVertical
        int i = 0;
        QListIterator< int > it( aQSvn->splitterVertical->sizes() );
        while ( it.hasNext() )
        {
            settings.setValue( QString( "verticalSize%1" ).arg( i ), it.next() );
            i++;
        }

        //save settings from splitterHorizontal
        i = 0;
        it = ( aQSvn->splitterHorizontal->sizes() );
        while ( it.hasNext() )
        {
            settings.setValue( QString( "horizontalSize%1" ).arg( i ), it.next() );
            i++;
        }
    }
}

void Config::restoreMainWindow( QSvn *aQSvn )
{
    if ( aQSvn )
    {
        QSettings settings;

        aQSvn->resize( settings.value( "mainwindow/width", aQSvn->width() ).toInt(),
                       settings.value( "mainwindow/height", aQSvn->height() ).toInt() );

        aQSvn->move( QPoint( settings.value( "mainwindow/x", aQSvn->x() ).toInt(),
                             settings.value( "mainwindow/y", aQSvn->y() ).toInt() ) );

        //restore settings from splitterVertical
        int i = 0;
        QList< int > list = aQSvn->splitterVertical->sizes();
        QList< int >::Iterator it = list.begin();
        while( it != list.end() )
        {
            *it = settings.value( QString( "verticalSize%1" ).arg( i ), &it ).toInt();
            ++it;
            ++i;
        }
        aQSvn->splitterVertical->setSizes( list );

        //restore settings from splitterHorizontal
        i = 0;
        list = aQSvn->splitterVertical->sizes();
        it = list.begin();
        while( it != list.end() )
        {
            *it = settings.value( QString( "horizontalSize%1" ).arg( i ), &it ).toInt();
            ++it;
            ++i;
        }
        aQSvn->splitterHorizontal->setSizes( list );
    }
}

void Config::saveStringList( const QString &prefix, const QStringList &stringList )
{
    if ( prefix.isEmpty() || stringList.isEmpty() )
        return;

    QSettings settings;

    settings.remove( prefix );
    settings.beginWriteArray( prefix, stringList.count() );
    for ( int i = 0; i < stringList.count(); ++i )
    {
        settings.setArrayIndex( i );
        settings.setValue( "entry" , stringList.at( i ) );
    }
    settings.endArray();
}

QStringList Config::getStringList( const QString &prefix )
{
    QStringList stringList;
    QSettings settings;

    int size = settings.beginReadArray( prefix );

    for ( int i = 0; i < size; ++i )
    {
        settings.setArrayIndex( i );
        stringList.append( settings.value( "entry" ).toString() );
    }
    settings.endArray();

    return stringList;
}


/*todo:
void Config::saveListView( QListView *aListView )
{
    if ( aListView )
    {
        QSettings settings;
        settings.setPath( _SETTINGS_DOMAIN, _SETTINGS_PRODUCT, QSettings::User );
        settings.beginGroup( "listViews/" + QString( aListView->name() ) );
        for ( int i = 0; i < aListView->columns(); i++ )
        {
            settings.writeEntry( QString( "Column%1" ).arg( i ), aListView->columnWidth( i ) );
        }
        settings.endGroup();
    }
}

void Config::restoreListView( QListView *aListView )
{
    if ( aListView )
    {
        QSettings settings;
        settings.setPath( _SETTINGS_DOMAIN, _SETTINGS_PRODUCT, QSettings::User );
        settings.beginGroup( "listViews/" + QString( aListView->name() ) );
        for ( int i = 0; i < aListView->columns(); i++ )
        {
            aListView->setColumnWidth( i,
                                       settings.readNumEntry( QString( "Column%1" ).arg( i ),
                                       aListView->columnWidth( i ) ) );
        }
        settings.endGroup();
    }
}
*/
