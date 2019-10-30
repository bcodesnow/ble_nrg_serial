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
    QNetworkReply *m_reply;
    QNetworkRequest m_request;
    uint8_t m_authorized = AUTH_UNAUTH;

    QOAuth2AuthorizationCodeFlow *m_googleAuth;
    QNetworkAccessManager *m_networkHandler;
    QOAuthHttpServerReplyHandler *m_oauthReplyHandler;

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
    explicit NetworkManager()
    {
        m_googleAuth = new QOAuth2AuthorizationCodeFlow();
        m_networkHandler = new QNetworkAccessManager();


        m_googleAuth->setScope(drive_file_scope);

        connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
                &QDesktopServices::openUrl);
        connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::granted,
                this, &NetworkManager::authorizationGranted);
        connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::statusChanged,
                this, &NetworkManager::stateChanged);

        connect( m_networkHandler, &QNetworkAccessManager::authenticationRequired,
                 this, &NetworkManager::authenticationReply);
        connect( m_networkHandler, SIGNAL(finished(QNetworkReply *)),
                 this, SLOT(replyFinished(QNetworkReply *)));

    }



    void authorize()
    {
        if (m_authorized == AUTH_SUCCESS)
            return;
        // parse json file
        QFile jsonReader;

        QString fn = ":/common/client_secret_914794792674-hkarun0t7ccuchh8om2n0a160rnsnbce.apps.googleusercontent.com.json";
        QByteArray val;
        // jsonReader.setFileName(m_refToFileHandlerNM->getHomeLocation()+fn);
        jsonReader.setFileName(fn);
        qDebug()<<"NetworkManager: json file location:"<<QStandardPaths::HomeLocation+fn;
        if ( !jsonReader.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            qDebug()<<"NetworkManager: can't read json file";
            jsonReader.close();
            return;
        }
        val = jsonReader.readAll();
        jsonReader.close();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(val);
        QJsonObject jsonObj = jsonDoc.object();
        const QJsonObject webSettingsObj = jsonObj["web"].toObject();
        m_authUri = QUrl(webSettingsObj["auth_uri"].toString());
        m_clientId = QString(webSettingsObj["client_id"].toString());
        m_tokenUri = QUrl(webSettingsObj["token_uri"].toString());
        m_clientSecret = QString(webSettingsObj["client_secret"].toString());

        const QJsonArray redirectUris = webSettingsObj["redirect_uris"].toArray();
        const QUrl redirectUri = QUrl(redirectUris[0].toString()); // Get the first URI
        m_port = static_cast<quint16>(redirectUri.port()); // Get the port

        // config google
        qDebug()<<"NetworkManager: authorization URI:"<<m_authUri;
        m_googleAuth->setAuthorizationUrl(m_authUri);
        qDebug()<<"NetworkManager: client ID:"<<m_clientId;
        m_googleAuth->setClientIdentifier(m_clientId);
        qDebug()<<"NetworkManager: token URI:"<<m_tokenUri;
        m_googleAuth->setAccessTokenUrl(m_tokenUri);
        qDebug()<<"NetworkManager: client secret:"<<m_clientSecret;
        m_googleAuth->setClientIdentifierSharedKey(m_clientSecret);
        qDebug()<<"NetworkManager: port:"<<m_port;

        m_oauthReplyHandler = new QOAuthHttpServerReplyHandler(m_port);

        connect(m_oauthReplyHandler, &QOAuthHttpServerReplyHandler::replyDataReceived,
                this, &NetworkManager::printNetworkReply);

        connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::granted,
                m_oauthReplyHandler, &QOAuthHttpServerReplyHandler::close);

        m_googleAuth->setReplyHandler(m_oauthReplyHandler);


        // start authorization
        qDebug()<<"NetworkManager: starting authorization";
        m_googleAuth->grant();
    }


    void createDriveFolder(QString name, QString uploadUrl)
    {
        uploadUrl.append(drive_type_multi);
        QNetworkRequest request(uploadUrl);


        const QString bearer = bearer_format.arg(QString(m_currentAccessToken));

        QHttpPart MetadataPart;
        MetadataPart.setRawHeader("Content-Type", "application/json; charset=UTF-8");
        QString body = "{\n" + tr("\"name\": \"%1\",\n").arg(name)
                + tr("\"mimeType\": \"%1\"\n").arg("application/vnd.google-apps.folder") + tr("}");
        MetadataPart.setBody(body.toUtf8());

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
        multiPart->setBoundary("bound2");
        multiPart->append(MetadataPart);

        request.setRawHeader("Content-Type","multipart/related; boundary=bound2");
        request.setRawHeader("Authorization", bearer.toUtf8());

        qDebug()<<"Sending request:"<<request.rawHeaderList();

        QNetworkReply *reply = m_networkHandler->post(request,multiPart);

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug()<<"Folder upload request. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });
    }


    void uploadFileHttpMulti(QString localPath, QString uploadUrl,
                             QByteArray contentType, QString folderID = "")
    {
        uploadUrl.append(drive_type_multi);
        QNetworkRequest request(uploadUrl);

        const QString bearer = bearer_format.arg(QString(m_currentAccessToken));

        QFile *file = new QFile(localPath); // ":/qml/images/splash.png"
        if ( !file->open(QIODevice::ReadOnly) )
        {
            qDebug()<<"cannot open file";
            return;
        }
        QByteArray rawData = file->readAll();
        file->close();

        QHttpPart MetadataPart;
        MetadataPart.setRawHeader("Content-Type", "application/json; charset=UTF-8");
        QString body;
        if (folderID.isEmpty()) {
            body = "{\n" + tr("\"name\": \"%1\"\n").arg("myUpload") + tr("}");
        }
        else {
            body = "{\n" + tr("\"name\": \"%1\",\n").arg("myUpload")
                    + tr("\"parents\": [\"%1\"]\n").arg(folderID)
                    + tr("}");
        }

        MetadataPart.setBody(body.toUtf8());

        QHttpPart MediaPart;
        MediaPart.setRawHeader("Content-Type", contentType);
        MediaPart.setBody(rawData);

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
        multiPart->setBoundary("bound1");
        multiPart->append(MetadataPart);
        multiPart->append(MediaPart);

        request.setRawHeader("Content-Type","multipart/related; boundary=bound1");
        request.setRawHeader("Authorization", bearer.toUtf8());

        qDebug()<<"Sending request:"<<request.rawHeaderList();

        QNetworkReply *reply = m_networkHandler->post(request,multiPart);

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug()<<"File upload request. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });
    }

    void readFilesHttp (QString downloadUrl, QString folderID = "")
    {
        QNetworkRequest request(downloadUrl);
        const QString bearer = bearer_format.arg(QString(m_currentAccessToken));
        QNetworkReply *reply = m_networkHandler->get(request);

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug()<<"Read filenames request. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });
    }




    uint8_t getAuthorized()
    {
        return m_authorized;
    }


    Q_INVOKABLE void synchronizeData()
    {
        authorize();

        if (m_authorized == AUTH_SUCCESS)
        {
            // todo: qsettings and synchronization with drive-appdata
            qDebug()<<"sending TEST request";
            // uploadFileHttpMulti(":/qml/images/splash.png", drive_file_upload_url, "image/png", development_drive_folder_id);
            //  uploadFileHttpMulti("/home/boergi/catch_balint/0_LEFT_AUDIO", drive_file_url, "text/plain");
            // createDriveFolder("Catch_Data_Wearable",drive_file_upload_url);
            readFilesHttp(file_metadata_url); // drives_url file_metadata_url
        }



    }

    //    void setRefToFileHandlerNM(LogFileHandler *reference)
    //    {
    //        m_refToFileHandlerNM = reference;
    //    }

signals:
    void authorizedChanged();

public slots:

    void uploadFinished(QNetworkReply *reply)
    {
        qDebug()<<"NetworkManager: Upload finished"<<reply->url();
    }

    void stateChanged(QAbstractOAuth::Status state)
    {
        if (state == QAbstractOAuth::Status::Granted) {
            qDebug()<<"State changed to: Granted";
            m_authorized = AUTH_SUCCESS;
            emit authorizedChanged();
            if (m_oauthReplyHandler->isListening())
            {
                qDebug()<<"NetworkManager: Reply handler listening to port"<<m_oauthReplyHandler->port();
                qDebug()<<"NetworkManager: callback path:"<<m_oauthReplyHandler->callbackPath();
                qDebug()<<"NetworkManager: callback text:"<<m_oauthReplyHandler->callbackText();
                qDebug()<<"NetworkManager: callback:"<<m_oauthReplyHandler->callback();
            }
            else {
                qDebug()<<"NetworkManager: Reply Handler not listening";
            }
        }
        else if (state == QAbstractOAuth::Status::NotAuthenticated) {
            qDebug()<<"State changed to: NotAuthenticated";
            m_authorized = AUTH_FAILURE;
            emit authorizedChanged();
        }
        else if (state == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
            qDebug()<<"State changed to: TemporaryCredentialsReceived";
        }
        else if (state == QAbstractOAuth::Status::RefreshingToken)
        {
            qDebug()<<"State changed to: RefreshingToken";
        }
    }

    void authorizationGranted() {
        qDebug() << "Authorization granted. Expires at:" << m_googleAuth->expirationAt().toString();
    }

    void replyFinished(QNetworkReply *reply)
    {
        qDebug()<<"Network reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
        qDebug() << reply->readAll();
    }

    void printNetworkReply (const QByteArray reply)
    {
        qDebug()<<"OAuth reply arrived.";
        qDebug()<<reply;

        const QByteArray accesstoken = "access_token";
        const QByteArray expiresin = "expires_in";

        QList<QByteArray> replyList = reply.split('\"');
        m_currentAccessToken = replyList.at(3);

        QByteArray tmpExpiration;
        tmpExpiration = replyList.at(6);
        tmpExpiration = tmpExpiration.simplified();
        tmpExpiration = tmpExpiration.left(tmpExpiration.size()-1);
        tmpExpiration = tmpExpiration.right(tmpExpiration.size()-1);

        m_accessExpirationIn = tmpExpiration.toInt();

    }

    void authenticationReply(QNetworkReply *reply, QAuthenticator *authenticator) {
        qDebug()<<"Authentication reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
        qDebug()<<"auth user:"<<authenticator->user();
        qDebug()<<"auth password:"<<authenticator->password();
        qDebug()<<"auth realm:"<<authenticator->realm();
        qDebug()<<reply->readAll();
    }


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
