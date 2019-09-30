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

#define AUTH_INIT    1U
#define AUTH_SUCCESS 2U
#define AUTH_FAILURE 3U

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int authorized READ getAuthorized NOTIFY authorizedChanged)
private:
    LogFileHandler *m_refToFileHandlerNM;
    QNetworkReply *m_reply;
    QNetworkRequest m_request;
    QNetworkAccessManager *m_accessManager;
    QString m_replyStr;
    QOAuth2AuthorizationCodeFlow *m_googleAuth;
    QOAuthHttpServerReplyHandler *m_replyHandler;
    QUrl m_authUri;
    QString m_clientId;
    QUrl m_tokenUri;
    QString m_clientSecret;
    uint16_t m_port;
    QUrl m_uploadUrl;
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
            "https://www.googleapis.com/auth/drive.readonly "
            "https://www.googleapis.com/auth/userinfo.email";

    uint8_t m_authorized = AUTH_INIT;
public:
    explicit NetworkManager(LogFileHandler *fh )
    {
        m_refToFileHandlerNM = fh;
        m_googleAuth = new QOAuth2AuthorizationCodeFlow;
        m_googleAuth->setScope("https://www.googleapis.com/auth/drive.file");

        m_replyStr.clear();
        m_accessManager = new QNetworkAccessManager();
        //googleAuth->setScope("https://www.googleapis.com/auth/drive");
        connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
                &QDesktopServices::openUrl);
        //connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::granted, this, &NetworkManager::onAuthGranted);
        connect(m_googleAuth, &QOAuth2AuthorizationCodeFlow::statusChanged, this, &NetworkManager::stateChanged);
        connect(m_accessManager, SIGNAL(finished(QNetworkReply *)),
                this, SLOT(uploadFinished(QNetworkReply *)));
    }

    uint8_t getAuthorized()
    {
        return m_authorized;
    }

    void downloadFromDrive()
    {

    }

    //void uploadToDrive(QNetworkReply *reply)
    void uploadToDrive()
    {
        //        QByteArray data = reply->readAll();
        //        qDebug() << data;
        //        QString x = getValue(data,"access_token");
        //        qDebug() << x;
        //        x = "Bearer " + x;
        //        qDebug() << x;
        QNetworkRequest request;
        // QUrl url("https://www.googleapis.com/upload/drive/v2/files?uploadType=media");
        QUrl url(QString(googleDriveFile).arg("devTest"));
        request.setUrl(url);
        request.setRawHeader("Content-Length","200000000");
        request.setRawHeader("Content-Type","image/jpeg");
        request.setRawHeader("title","sp");



        QFile uploader(m_refToFileHandlerNM->getHomeLocation()+"splash_small.jpg");
        qDebug()<<"NetworkManger: uploading file:"<<uploader.open(QIODevice::ReadOnly);
        QByteArray arr = uploader.readAll();
        uploader.close();

        m_accessManager->post(request,arr);

    }

    void authorize()
    {
        if (m_authorized == AUTH_SUCCESS)
            return;
        // parse json file
        QFile jsonReader;
        QString fn = "client_secret_914794792674-hkarun0t7ccuchh8om2n0a160rnsnbce.apps.googleusercontent.com.json";
        QByteArray val;
        jsonReader.setFileName(m_refToFileHandlerNM->getHomeLocation()+fn);
        qDebug()<<"NetworkManager: opening json file -"<<
                  jsonReader.open(QIODevice::ReadOnly | QIODevice::Text);
        val = jsonReader.readAll();
        jsonReader.close();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(val);
        QJsonObject jsonObj = jsonDoc.object();
        const QJsonValueRef jsonWebRef = jsonObj["web"];
        const QJsonObject webSettingsObj = jsonWebRef.toObject();
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
        m_replyHandler = new QOAuthHttpServerReplyHandler(m_port, this);
        m_googleAuth->setReplyHandler(m_replyHandler);

        // start authorization
        qDebug()<<"NetworkManager: starting authorization";
        m_googleAuth->grant();
    }





    void setRefToFileHandlerNM(LogFileHandler *reference)
    {
        m_refToFileHandlerNM = reference;
    }


    Q_INVOKABLE void synchronizeData()
    {
        authorize();

        if (m_authorized == AUTH_SUCCESS)
        {
            qDebug()<<"UPLOAD";
            //uploadToDrive();
        }



    }



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
            if (m_replyHandler->isListening())
            {
                qDebug()<<"NetworkManager: Reply handler listening to port"<<m_replyHandler->port();
                qDebug()<<"NetworkManager: callback path:"<<m_replyHandler->callbackPath();
                qDebug()<<"NetworkManager: callback text:"<<m_replyHandler->callbackText();
                qDebug()<<"NetworkManager: callback:"<<m_replyHandler->callback();
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
};

#endif // NETWORKMANAGER_H
