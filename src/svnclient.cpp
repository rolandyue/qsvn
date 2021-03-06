/********************************************************************************
 *   This file is part of QSvn Project http://www.anrichter.net/projects/qsvn   *
 *   Copyright (c) 2004-2010 Andreas Richter <ar@anrichter.net>                 *
 *                                                                              *
 *   This program is free software; you can redistribute it and/or modify       *
 *   it under the terms of the GNU General Public License Version 2             *
 *   as published by the Free Software Foundation.                              *
 *                                                                              *
 *   This program is distributed in the hope that it will be useful,            *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 *   GNU General Public License for more details.                               *
 *                                                                              *
 *   You should have received a copy of the GNU General Public License          *
 *   along with this program; if not, write to the                              *
 *   Free Software Foundation, Inc.,                                            *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
 *                                                                              *
 *******************************************************************************/

//QSvn
#include "config.h"
#include "listener.h"
#include "showlog.h"
#include "statustext.h"
#include "svnclient.h"
#include "svnclient.moc"

//SvnCpp
#include "svnqt/client.hpp"
#include "svnqt/client_commit_parameter.hpp"
#include "svnqt/revision.hpp"
#include "svnqt/status.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/url.hpp"
#include "svnqt/wc.hpp"
#include "svnqt/client_parameter.hpp"
#include "svnqt/client_update_parameter.h"

//Qt
#include <QtCore>
#include <QtGui>


//make SvnClient a singleton
SvnClient* SvnClient::m_instance = 0;

SvnClient* SvnClient::instance()
{
    if (m_instance == 0)
        m_instance = new SvnClient;

    return m_instance;
}

//SvnClient implementation
SvnClient::SvnClient()
{
    svnContext = new svn::Context();
    svnClient = svn::Client::getobject(svnContext, 0);
    listener = new Listener(this);

    svnContext->setListener(listener);
    m_InProgress = false;
}

SvnClient::~SvnClient()
{
    delete listener;
    delete svnClient;
    delete svnContext;
}

bool SvnClient::isInProgress()
{
    return m_InProgress;
}

svn::StatusEntries SvnClient::status(const QString& path,
                                     svn::Depth depth,
                                     const bool get_all,
                                     const bool update,
                                     const bool no_ignore,
                                     const svn::Revision revision,
                                     bool detailed_remote,
                                     const bool hide_externals)
{
    if (!svn::Wc::checkWc(path))
        return svn::StatusEntries();

    QDir dir(path);
    listener->setVerbose(false);
    svn::StatusParameter params(dir.canonicalPath());
    try
    {
        return svnClient->status(params.depth(depth).all(get_all).update(update).noIgnore(no_ignore).revision(revision).detailedRemote(detailed_remote).ignoreExternals(hide_externals));
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return svn::StatusEntries();
    }
}

svn::StatusPtr SvnClient::singleStatus(const QString &path)
{
    listener->setVerbose(false);
    try
    {
        return svnClient->singleStatus(path, false, svn::Revision::HEAD);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return svn::StatusPtr(new svn::Status());
    }
}

bool SvnClient::update(QStringList updateList, const bool isFileList)
{
    if (updateList.isEmpty())
        return true;

    svn::Revision fromRevision;
    svn::Revisions toRevisions;
    bool result = true;

    foreach (QString path, updateList)
    {
        fromRevision = singleStatus(path)->entry().cmtRev();
        try
        {
            StatusText::out(tr("\nUpdate '%1'").arg(path));
            listener->setVerbose(true);
            m_InProgress = true;
            svn::UpdateParameter updateParameter;
            updateParameter.targets(path)
                           .revision(svn::Revision::HEAD)
                           .depth(svn::DepthInfinity)
                           .ignore_externals(false)
                           .allow_unversioned(false)
                           .sticky_depth(true);
            toRevisions = svnClient->update(updateParameter);
            m_InProgress = false;
            if (Config::instance()->value(KEY_SHOWLOGAFTERUPDATE).toBool() &&
                !toRevisions.isEmpty() )
            {
                ShowLog::doShowLog(0, path,
                                   toRevisions.at(0).revision(),
                                           fromRevision);
            }
        }
        catch (const svn::ClientException& e)
        {
            StatusText::out(e.msg());
            result = false;
        }
    }
    return result;
}

bool SvnClient::svnexport(const QString &url, const QString &path,
                          const svn::Revision &revision, const bool verbose)
{
    if (url.isEmpty() || path.isEmpty())
        return false;

    if (verbose)
        StatusText::out(tr("\nExport '%1' to '%2'").arg(url).arg(path));

    svn::CheckoutParameter checkoutParameter;
    checkoutParameter
            .moduleName(url)
            .destination(path)
            .revision(revision);

    listener->setVerbose(verbose);
    try
    {
        svnClient->doExport(checkoutParameter);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    listener->setVerbose(true);
    return true;
}

bool SvnClient::add(const QString &path, svn::Depth depth)
{
    if (path.isEmpty())
        return true;

    listener->setVerbose(true);
    try
    {
        svnClient->add(path, depth);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::add(const QStringList &addList, svn::Depth depth)
{
    if (addList.isEmpty())
        return true;

    listener->setVerbose(true);
    try
    {
        QString file;
        foreach (file, addList)
        {
            svnClient->add(file, depth);
        }
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::revert(const QStringList &revertList, const bool verbose)
{
    if (revertList.isEmpty())
        return true;

    listener->setVerbose(verbose);
    try
    {
        svn::Targets targets(revertList);
        svnClient->revert(targets, svn::DepthEmpty);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::revert(const QString fileName, const bool verbose)
{
    QStringList fileList;
    fileList << fileName;
    return revert(fileList, verbose);
}

bool SvnClient::commit(const QStringList &commitList, const QString &logMessage)
{
    if (commitList.isEmpty())
        return true;

    StatusText::out(tr("\nStart Commit"));
    listener->setVerbose(true);
    try
    {
        svn::Targets targets(commitList);
        svn::Depth _depth = svn::DepthFiles;
        foreach(QString _path, commitList)
        {
            QFileInfo _fileInfo(_path);
            if (_fileInfo.isDir())
            {
                svn::StatusPtr _status = singleStatus(_path);
                if (_status->textStatus() == svn_wc_status_deleted)
                {
                    _depth = svn::DepthInfinity;
                    break;
                }
            }
        }
        svn::CommitParameter commitParameter;
        commitParameter
                .targets(targets)
                .depth(_depth)
                .message(logMessage);
        svnClient->commit(commitParameter);
        completedMessage(commitList.at(0));
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::remove(const QStringList &removeList)
{
    if (removeList.isEmpty())
        return true;

    listener->setVerbose(true);
    try
    {
        svn::Targets targets(removeList);
        svnClient->remove(targets, false, false);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

QString SvnClient::getFileRevisionPath(const QString &file, const svn::Revision &revision)
{
    QString _result, _file, _base, _ext, _dir;
    svn::Path _path(file);
    switch (svn::Revision(revision).kind())
    {
        case svn_opt_revision_base:
            _path.split(_dir, _file);
            _result = _dir +
                    QDir::separator() +
                    QString(".svn/text-base/%1.svn-base").arg(_file);
            _result = QDir::toNativeSeparators(_result);
            break;
        case svn_opt_revision_working:
            _result = _path.native();
            break;
        default:
            if (svn::Url::isValid(file))
            {
                _path.split(_dir, _file, _ext);
                _result = Config::instance()->tempDir() +
                        _file + QString(".%1.%2")
                        .arg(int(revision.revnum()))
                        .arg(_ext);
                if (!svnexport(file, _result, revision, false))
                    _result.clear();
            }
            break;
    }
    return _result;
}

bool SvnClient::diff(const QString &fileFrom, const QString &fileTo, const svn::Revision &revisionFrom, const svn::Revision &revisionTo)
{
    if (Config::instance()->value(KEY_DIFFVIEWER).toString().isEmpty())
    {
        //diff output to StatusText
        listener->setVerbose(true);
        try
        {
            svn::DiffParameter params;
            QString fileRelative(fileFrom==fileTo?fileFrom:"");

            QString delta = svnClient->diff(params.tmpPath(svn::Path(Config::instance()->tempDir())).
                                            path1(svn::Path(fileFrom)).path2(svn::Path(fileTo)).relativeTo(svn::Path(fileRelative)).
                                            rev1(revisionFrom).rev2(revisionTo).
                                            depth(svn::DepthInfinity).ignoreAncestry(false).noDiffDeleted(false).ignoreContentType(true));
            if (delta.isEmpty())
                StatusText::out(tr("There are no differences."));
            else
                StatusText::out(delta);
        }
        catch (svn::ClientException e)
        {
            StatusText::out(e.msg());
            return false;
        }
    }
    else
    {
        QString _fileFrom, _fileTo;
        _fileFrom = getFileRevisionPath(fileFrom, revisionFrom);
        _fileTo = getFileRevisionPath(fileTo, revisionTo);
        if (!QProcess::startDetached(Config::instance()->value(KEY_DIFFVIEWER).toString(),
             QStringList() << _fileFrom << _fileTo))
        {
            QMessageBox::critical(0, tr("Error"),
                                  QString(tr("Can't start %1"))
                                          .arg(Config::instance()->value(KEY_DIFFVIEWER).toString()));
        }
    }
    return true;
}

bool SvnClient::diffBASEvsWORKING(const QString &file)
{
    return diff(file, file, svn::Revision::BASE, svn::Revision::WORKING);
}

bool SvnClient::diffBASEvsWORKING(const QStringList &fileList)
{
    bool result = true;
    QString file;
    foreach (file, fileList)
    result = result && diff(file, file, svn::Revision::BASE, svn::Revision::WORKING);

    return result;
}

bool SvnClient::log(const QString &path,
                                        const svn::Revision &revisionStart,
                                        const svn::Revision &revisionEnd,
                                        const svn::Revision &revisionPeg,
                                        bool discoverChangedPaths,
                                        bool strictNodeHistory,
                                        int limit,svn::LogEntriesMap&targetmap )
{
    listener->setVerbose(true);
    try
    {
        svn::LogParameter params;

        return svnClient->log(params.targets(path).revisionRange(revisionStart,revisionEnd).
                                peg(revisionPeg).discoverChangedPathes(discoverChangedPaths).
                                strictNodeHistory(strictNodeHistory).limit(limit),
                                targetmap);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
}

bool SvnClient::cleanup(const QString &path)
{
    listener->setVerbose(true);
    try
    {
        StatusText::out(tr("\nCleanup '%1'").arg(path));
        svnClient->cleanup(path);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    completedMessage("");
    return true;
}

void SvnClient::completedMessage(const QString &path)
{
    if (!path.isEmpty())
    {
        QString _path = path;
        svn::StatusPtr status = singleStatus(_path);
        if (status->isVersioned())
            StatusText::out(QString(tr("Completed at Revision %1\n"))
                    .arg(status->entry().revision()));
        else
            StatusText::out(tr("Completed\n"));
    }
    else
        StatusText::out(tr("Completed\n"));
}

void SvnClient::setCancel()
{
    listener->setCancel(true);
}

bool SvnClient::resolved(const QString &path)
{
    listener->setVerbose(true);
    try
    {
        svnClient->resolve(path, svn::DepthEmpty);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::move(const QString &srcPath, const QString &destPath,
                     bool force)
{
    listener->setVerbose(true);
    try
    {
        svn::CopyParameter params(srcPath,destPath);
        svnClient->move(params.force(force));
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::copy(const QString &srcPath, const QString &destPath)
{
    listener->setVerbose(true);
    try
    {
        svnClient->copy(srcPath, svn::Revision::WORKING, destPath);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

bool SvnClient::mkdir(const QString & destPath)
{
    const QString message = "";
    listener->setVerbose(true);
    try
    {
        svnClient->mkdir(svn::Path(destPath), message);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
    return true;
}

svn::PathPropertiesMapListPtr SvnClient::propList(const QString &path,
        const svn::Revision &revision, const svn::Revision &peg, svn::Depth depth)
{
    listener->setVerbose(true);
    try
    {
        return svnClient->proplist(path, revision, peg, depth);
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return svn::PathPropertiesMapListPtr();
    }
}

bool SvnClient::propSet(const svn::PropertiesMap propMap,
                        const QString &path,
                        svn::Depth depth,
                        const svn::Revision &revision)
{
    bool result = true;
    //remove all exist properties
    svn::PathPropertiesMapListPtr _propList;
    _propList = propList(path, revision, revision);
    if(!_propList->isEmpty())
    {
        svn::PathPropertiesMapEntry entry = _propList->at(0);
        svn::PropertiesMap _oldPropMap = entry.second;
        QMapIterator<QString, QString> _oldIter(_oldPropMap);
        while (_oldIter.hasNext())
        {
            _oldIter.next();
            result = result && propDel(_oldIter.key(), path, svn::DepthEmpty, revision);
        }
    }

    //write new properties
    QMapIterator<QString, QString> _iter(propMap);
    while (_iter.hasNext())
    {
        _iter.next();
        result = result && propSet(_iter.key(), _iter.value(), path, depth, revision);
    }
    return result;
}

bool SvnClient::propSet(const QString &propName,
                        const QString &propValue,
                        const svn::Path &path,
                        svn::Depth depth,
                        const svn::Revision &revision,
                        bool skip_checks)
{
    listener->setVerbose(true);
    try
    {
        svn::PropertiesParameter params;
        svnClient->propset(params.propertyName(propName).propertyValue(propValue).path(path).depth(depth).skipCheck(skip_checks).revision(revision));
        return true;
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
}

bool SvnClient::propDel(const QString &propName,
                        const svn::Path &path,
                        svn::Depth depth,
                        const svn::Revision &revision)
{
    listener->setVerbose(true);
    try
    {
        //todo: adjust function-header to propset-header
        svn::PropertiesParameter params;
        svnClient->propset(params.propertyName(propName).path(path).depth(depth).revision(revision).propertyValue(QString()));
        return true;
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
}

bool SvnClient::revPropSet(const QString & propName, const QString & propValue,
                           const svn::Path & path, const svn::Revision & revision,
                           bool force)
{
    listener->setVerbose(true);
    try
    {
        svn::PropertiesParameter params;
        svnClient->revpropset(params.propertyName(propName).propertyValue(propValue).path(path).revision(revision).force(force));
        return true;
    }
    catch (svn::ClientException e)
    {
        m_lastErrorMessage = e.msg();
        return false;
    }
}

bool SvnClient::removeFromDisk(const QStringList &pathList)
{
    QFileInfo fileInfo;
    QDir dir;
    bool success = false, result = true;
    foreach(QString path, pathList)
    {
        fileInfo = QFileInfo(path);
        if (fileInfo.isDir())
        {
            success = dir.rmdir(fileInfo.absolutePath());
        } else if (fileInfo.isFile()) {
            success = QFile::remove(fileInfo.absoluteFilePath());
        }
        if (success)
            StatusText::out(QString(tr("removed from disk: %1")).arg(fileInfo.absoluteFilePath()));
        else
            StatusText::out(QString(tr("Error while remove from disk: %1")).arg(fileInfo.absoluteFilePath()));
        result = result && success;
    }
    return result;
}

bool SvnClient::merge(const QString &fromUrl, const svn::Revision &fromRevision,
                      const QString &toUrl, const svn::Revision &toRevision,
                      const QString & wcPath, bool force, bool recurse,
                      bool notice_ancestry, bool dry_run)
{
    listener->setVerbose(true);

    svn::RevisionRanges revisionRanges;
    revisionRanges.append(svn::RevisionRange(fromRevision, toRevision));

    svn::MergeParameter mergeParameter;
    mergeParameter
            .revisions(revisionRanges)
            .path1(fromUrl)
            .path2(toUrl)
            .depth(recurse?svn::DepthInfinity:svn::DepthFiles)
            .notice_ancestry(notice_ancestry)
            .force(force)
            .dry_run(dry_run);
    try
    {
        svnClient->merge(mergeParameter);
        return true;
    }
    catch (svn::ClientException e)
    {
        StatusText::out(e.msg());
        return false;
    }
}

const QString SvnClient::getUUID(const QString &path)
{
    svn::InfoEntries infoEntries = svnClient->info(path, svn::DepthEmpty, svn::Revision::UNDEFINED, svn::Revision::UNDEFINED);
    if (!infoEntries.isEmpty())
        return infoEntries.at(0).uuid();
    else
        return QString();
}

const QString SvnClient::getSvnActionName(const SvnAction action)
{
    switch (action)
    {
        case SvnNone:
            return tr("");
            break;
        case SvnAdd:
            return tr("Add");
            break;
        case SvnCommit:
            return tr("Commit");
            break;
        case SvnDelete:
            return tr("Delete");
            break;
        case SvnRevert:
            return tr("Revert");
            break;
        case SvnRename:
            return tr("Rename");
            break;
        case SvnMove:
            return tr("Move");
            break;
        case SvnCopy:
            return tr("Copy");
            break;
        case SvnMkDir:
            return tr("MkDir");
            break;
        case RemoveFromDisk:
            return tr("RemoveFromDisk");
            break;
        default:
            return tr("unknown Action");
            break;
    }
}

QString SvnClient::getLastErrorMessage() const
{
    return m_lastErrorMessage;
}
