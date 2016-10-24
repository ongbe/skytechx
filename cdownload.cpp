#include "cdownload.h"
#include "mainwindow.h"
#include "cbkimages.h"
#include "cstatusbar.h"

extern MainWindow *pcMainWnd;
extern CStatusBar *g_statusBar;

///////////////////////////////////////
CDownload::CDownload(QObject *parent) :
///////////////////////////////////////
  QObject(parent)
{
}

///////////////////////////////////////////////////////////
void CDownload::beginBkImage(QString url, QString fileName)
///////////////////////////////////////////////////////////
{
  QUrl qurl(url);

  qDebug("url '%s'", qPrintable(url));

  QNetworkRequest request(qurl);
  QNetworkReply *reply = manager.get(request);

  m_fileName = fileName;

  connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(slotProgress(qint64,qint64)));
  connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotDownloadFinished(QNetworkReply*)));
  connect(this, SIGNAL(sigProgress(qint64,int)), pcMainWnd->m_pcDSSProg, SLOT(setProgressValue(qint64,int)));
  connect(this, SIGNAL(sigError(QString)), pcMainWnd, SLOT(slotDownloadError(QString)));

  pcMainWnd->setToolBoxPage(1);
  g_statusBar->setDownloadStatus(true);
}

void CDownload::beginFile(QString url, QString fileName)
{
  QUrl qurl(url);

  qDebug("url '%s'", qPrintable(url));

  QNetworkRequest request(qurl);
  QNetworkReply *reply = manager.get(request);

  m_fileName = fileName;

  connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotDownloadFileFinished(QNetworkReply*)));
}

///////////////////////////////////////////////////////
void CDownload::slotProgress(qint64 recv ,qint64 total)
///////////////////////////////////////////////////////
{
  //qDebug() << "recv : " << recv << " / " << total;
  if (recv == 0 || total == 0)
  {
    emit sigProgress((qint64)this, 0);
    return;
  }

  int p = 100 - (recv * 100 / (float)total);

  if (p == 0)
    p = 1;
  if (recv == total)
    p = 0;

  emit sigProgress((qint64)this, p);
}

//////////////////////////////////////////////////////////
void CDownload::slotDownloadFinished(QNetworkReply *reply)
//////////////////////////////////////////////////////////
{
  //qDebug("done %s", qPrintable(reply->errorString()));
  if (reply->error() == QNetworkReply::NoError)
  {    
    SkFile f(m_fileName);
    QFileInfo fi(m_fileName);

    checkAndCreateFolder(fi.path());

    if (f.open(SkFile::WriteOnly))
    {
      f.write(reply->readAll());
      f.close();
    }

    bkImg.load(m_fileName);
    pcMainWnd->repaintMap();
  }
  else
  { // error
    emit sigError(reply->errorString());
  }

  reply->deleteLater();
  deleteLater();
  g_statusBar->setDownloadStatus(false);
}

void CDownload::slotDownloadFileFinished(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError)
  {
    SkFile f(m_fileName);

    if (f.open(SkFile::WriteOnly))
    {
      f.write(reply->readAll());
      f.close();
    }
    emit sigFileDone(true);
  }
  else
  { // error
    emit sigFileDone(false);
  }

  reply->deleteLater();
  deleteLater();
}

