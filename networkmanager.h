#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "logfilehandler.h"

#include <QObject>
#include <QtNetwork>
#include <QAbstractOAuth>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>

#include <QNetworkAccessManager>

#define AUTH_UNAUTH  1U
#define AUTH_SUCCESS 2U
#define AUTH_FAILURE 3U

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int authorized READ getAuthorized NOTIFY authorizedChanged)
private:
    LogFileHandler* m_logfile_handler_ptr;

    QNetworkReply *m_reply;
    QNetworkRequest m_request;
    uint8_t m_authorized = AUTH_UNAUTH;
    bool m_uploadEnabled = false;

    QOAuth2AuthorizationCodeFlow *m_googleAuth;
    QNetworkAccessManager *m_networkHandler;
    QOAuthHttpServerReplyHandler *m_oauthReplyHandler;

    QString m_curr_folder_id;
    QUrl m_authUri;
    QString m_clientId;
    QUrl m_tokenUri;
    QString m_clientSecret;
    uint16_t m_port;
    QUrl m_uploadUrl;
    QByteArray m_currentAccessToken;
    const QString bearer_format = QStringLiteral("Bearer %1");
    int m_accessExpirationIn; // todo

    // access_token expires_in scope token_type

    const QString drive_scope = "https://www.googleapis.com/auth/drive";
    const QString drive_file_scope = "https://www.googleapis.com/auth/drive.file";
    const QString drive_appdata_scope = "https://www.googleapis.com/auth/drive.appdata";

    const QString file_upload_url = "https://www.googleapis.com/upload/drive/v3/files";
    const QString file_metadata_url = "https://www.googleapis.com/drive/v3/files";
    const QString drives_url = "https://www.googleapis.com/drive/v3/drives";


    const QByteArray content_type_folder = "application/vnd.google-apps.folder";
    const QByteArray content_type_image = "image/png";
    const QByteArray content_type_txt = "text/plain";

    const QString drive_type_multi = "?uploadType=multipart";

    // note: get folder id by using createDriveFolder(name, uploadUrl)
    const QString development_drive_folder_id = "1eQo3DNOG7VmZtEwDnoX9migoQitqyIXL";

    const char* googleDriveFile =
            "https://www.googleapis.com/drive/v2/files/%1";
    const char* googleDriveChanges =
            "https://www.googleapis.com/drive/v2/changes";
    const char* googleOAuthUserInfoEndpoint =
            "https://www.googleapis.com/oauth2/v1/userinfo";
    const char* googlekOAuthEndpoint = "https://accounts.google.com/o/oauth2/auth";
    const char* googlekOAuthTokenEndpoint =
            "https://accounts.google.com/o/oauth2/token";
    const char* googlekOAuthScope =
            "https://www.googleapis.com/auth/drive.readonly";

public:
    NetworkManager(LogFileHandler* logfile_handler, QObject *parent = nullptr);

    Q_INVOKABLE void authorize();


    void createDriveFolder(QString name, QString uploadUrl);


//    void uploadLocalFileHttpMulti(QString localPath, QString uploadUrl,
//                             QByteArray contentType, QString folderID);

    void uploadDataHttpMulti(QByteArray data, QString name, QString uploadUrl,
                             QByteArray contentType, QString folderID = "");


    void readFilesHttp (QString downloadUrl, QString folderID = "");




    uint8_t getAuthorized();


    Q_INVOKABLE void synchronizeData();

    //    void setRefToFileHandlerNM(LogFileHandler *reference)
    //    {
    //        m_refToFileHandlerNM = reference;
    //    }

signals:
    void authorizedChanged();

public slots:

    void uploadFinished(QNetworkReply *reply);

    void stateChanged(QAbstractOAuth::Status state);

    void authorizationGranted();

    void replyFinished(QNetworkReply *reply);

    void printNetworkReply (const QByteArray reply);

    void authenticationReply(QNetworkReply *reply, QAuthenticator *authenticator);

    void uploadCatchData(QString name, QByteArray data);


};


//    void graveyard()
//    {
//        // QUrl url("https://www.googleapis.com/upload/drive/v2/files?uploadType=media");
//        // QUrl url(QString(googleDriveFile).arg("devTest"));
//        // QUrl url("https://www.googleapis.com/upload/drive/v2/files?uploadType=media"); // working
//        // QUrl url("https://www.googleapis.com/upload/drive/v2/files"); // working
//        QUrl url("https://www.googleapis.com/upload/drive/v3/files"); // working
//        // QUrl url("https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");
//        // QUrl url("https://www.googleapis.com/upload/drive/v3/files?uploadType=media"); // working

//        int switcher = 3;

//        if (switcher == 0)
//        {
//            QFile uploader(":/qml/images/splash.png");
//            qDebug()<<"NetworkManger: uploading file:"<<uploader.open(QIODevice::ReadOnly);
//            QByteArray arr = uploader.readAll();
//            uploader.close();



//            QString title = "myImage";
//            // QString protocol = QString("<title>%1</title>").arg(title);

//            //  QByteArray postData = protocol.toLatin1();
//            QNetworkRequest request;


//            //  request.setRawHeader("Authorization", QString("Bearer %1").arg(m_tokenUri.toString()).toLatin1());
//            request.setRawHeader("Content-Length", QString::number(arr.size()).toLatin1());
//            request.setRawHeader("Content-Type","image/png");
//            //  request.setRawHeader("name","myPngFile");


//            //  QNetworkAccessManager *mngr = m_googleAuth->networkAccessManager();
//            connect( m_googleAuth->networkAccessManager(), SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));


//            QNetworkReply *reply =  m_googleAuth->networkAccessManager()->post(request,arr);
//            connect(reply, &QNetworkReply::finished, [reply](){
//                qDebug()<<"NetworkManager: Reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
//                qDebug() << reply->readAll();
//            });



//        }
//        else if (switcher == 1)
//        {
//            // post using http multi part
//            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
//            // QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
//            //  QHttpPart imagePart;
//            //            imagePart.setHeader(QNetworkRequest::ContentTypeHeader,"image/png");
//            //            imagePart.setHeader(QNetworkRequest::ContentLengthHeader,"600000");
//            // imagePart.setRawHeader("Content-Length","600000"); // 300 kB needed
//            // imagePart.setRawHeader("Content-Type","image/png");
//            // imagePart.setRawHeader("name","myPngFile");
//            // imagePart.setRawHeader("title","myPngFile");

//            QHttpPart imagePart;
//            //imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
//            //imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"version.txt\""));
//            imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("\"name\": \"myFile\""));

//            //  QHttpPart textPart;
//            //   textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));
//            //textPart.setBody("toto");

//            QFile *file = new QFile(":/qml/images/splash.png");
//            file->open(QIODevice::ReadOnly);
//            imagePart.setBodyDevice(file);
//            file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

//            //  multiPart->append(textPart);
//            multiPart->append(imagePart);


//            QNetworkReply *reply = m_googleAuth->post(url,multiPart);
//            multiPart->setParent(reply); // delete the multiPart with the reply

//            connect(reply, &QNetworkReply::finished, [reply](){
//                qDebug()<<"NetworkManager: Reply arrived. Error? -"
//                       << (reply->error() != QNetworkReply::NoError);
//                qDebug() << reply->readAll();
//            });


//        }
//        else if (switcher == 2) {
//            // working, but filename is untitled
//            // post using raw data
//            QFile uploader("/home/boergi/catch_balint/0_LEFT_ACC.txt");
//            qDebug()<<"NetworkManger: uploading file:"<<uploader.open(QIODevice::ReadOnly);
//            QByteArray arr = uploader.readAll();
//            uploader.close();


//            QNetworkReply *reply = m_googleAuth->post(url,arr);
//            connect(reply, &QNetworkReply::finished, [reply](){
//                qDebug()<<"NetworkManager: Reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
//                qDebug() << reply->readAll();
//                qDebug()<<"TEST:ContentTypeHeader"<< reply->header(QNetworkRequest::ContentTypeHeader);
//                //                qDebug()<<"TEST:ContentDispositionHeader"<< reply->header(QNetworkRequest::ContentDispositionHeader);
//                qDebug()<<"TEST:ContentLengthHeader"<< reply->header(QNetworkRequest::ContentLengthHeader);
//                //                qDebug()<<"TEST:LocationHeader"<< reply->header(QNetworkRequest::LocationHeader);
//                //                qDebug()<<"TEST:LastModifiedHeader"<< reply->header(QNetworkRequest::LastModifiedHeader);
//                //                qDebug()<<"TEST:IfModifiedSinceHeader"<< reply->header(QNetworkRequest::IfModifiedSinceHeader);
//                //                qDebug()<<"TEST:ETagHeader"<< reply->header(QNetworkRequest::ETagHeader);
//                //                qDebug()<<"TEST:IfMatchHeader"<< reply->header(QNetworkRequest::IfMatchHeader);
//                //                qDebug()<<"TEST:IfNoneMatchHeader"<< reply->header(QNetworkRequest::IfNoneMatchHeader);
//                //                qDebug()<<"TEST:CookieHeader"<< reply->header(QNetworkRequest::CookieHeader);
//                //                qDebug()<<"TEST:SetCookieHeader"<< reply->header(QNetworkRequest::SetCookieHeader);
//                //                qDebug()<<"TEST:UserAgentHeader"<< reply->header(QNetworkRequest::UserAgentHeader);
//                qDebug()<<"TEST:ServerHeader"<< reply->header(QNetworkRequest::ServerHeader);
//            });

//        }
//        else if (switcher == 3) {
//            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
//            //  QHttpPart imagePart;

//            // imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"version.txt\""));/* version.tkt is the name on my Disk of the file that I want to upload */

//            QHttpPart textPart;
//            textPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
//            textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; title=\"file\"; filename=\"version.txt\""));
//            //  textPart.setBody("toto");

//            QFile *file = new QFile("/home/boergi/catch_balint/0_LEFT_ACC.txt");
//            file->open(QIODevice::ReadOnly);
//            textPart.setBodyDevice(file);
//            file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

//            multiPart->append(textPart);
//            //  multiPart->append(imagePart);

//            //   QNetworkRequest request(url);

//            //   QNetworkAccessManager *networkManager= new QNetworkAccessManager;
//            QNetworkReply *reply = m_googleAuth->post(url, multiPart);
//            multiPart->setParent(reply); // delete the multiPart with the reply

//            //            connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
//            //                    this, SLOT  (uploadProgress(qint64, qint64)));

//            connect(reply, &QNetworkReply::finished, [reply](){
//                qDebug()<<"NetworkManager: Reply arrived. Error? -"
//                       << (reply->error() != QNetworkReply::NoError);
//                qDebug() << reply->readAll();
//            });
//        }
//        else if (switcher ==4)
//        {
//            //  QHttpMultiPart *multiPart = new QHttpMultiPart();
//            //  QHttpPart imagePart;


//            QNetworkAccessManager *mngr = m_googleAuth->networkAccessManager();
//            connect(mngr, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));


//            QNetworkRequest request(url);
//            qDebug()<<"token uri:"<<m_googleAuth->accessTokenUrl();
//            qDebug()<<"auth uri:"<<m_googleAuth->authorizationUrl();


//            //  request.setRawHeader("Authorization", QString("Bearer %1").arg(m_tokenUri.toString()).toLatin1());
//            // request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authUri.toString()).toLatin1());
//            //  request.setRawHeader("Authorization", QString("Bearer %1").arg(m_googleAuth->accessTokenUrl().toString()).toLatin1());
//            request.setRawHeader("Authorization", m_googleAuth->createAuthenticatedUrl(m_authUri).toString().toLatin1());


//            request.setRawHeader("Content-Type","image/png");
//            request.setRawHeader("Content-Length","600000");
//            request.setRawHeader("name","myPngFile");


//            QFile *file = new QFile(":/qml/images/splash.png");
//            file->open(QIODevice::ReadOnly);
//            QByteArray arr = file->readAll();
//            file->close();

//            QNetworkReply *reply = mngr->post(request,arr);

//            connect(reply, &QNetworkReply::finished, [reply](){
//                qDebug()<<"NetworkManager: Reply arrived. Error? -"
//                       << (reply->error() != QNetworkReply::NoError);
//                qDebug() << reply->readAll();
//            });
//            connect(mngr, &QNetworkAccessManager::finished, this, &NetworkManager::replyFinished);
//        }

//        else if (switcher == 5)
//        {
//            QFile uploader(":/qml/images/splash.png");
//            qDebug()<<"NetworkManger: uploading file:"<<uploader.open(QIODevice::ReadOnly);
//            QByteArray arr = uploader.readAll();
//            uploader.close();

//            QHttpMultiPart *multipart = new QHttpMultiPart();
//            QHttpPart *imagepart = new QHttpPart();
//            // QNetworkAccessManager *mngr = m_googleAuth->networkAccessManager();


//            // imagepart->setRawHeader("Content-Type", "image/png");
//            imagepart->setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
//            imagepart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"image\""));
//            imagepart->setBody(arr);

//            QHttpPart *textpart = new QHttpPart();
//            textpart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
//            textpart->setBody("my text");

//            multipart->append(*textpart);
//            multipart->append(*imagepart);






//            QNetworkReply *reply = m_googleAuth->post(url,multipart);


//            connect(reply, &QNetworkReply::finished, [reply](){
//                qDebug()<<"NetworkManager: Reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
//                qDebug() << reply->readAll();
//            });

//        }


//    }


#endif // NETWORKMANAGER_H
