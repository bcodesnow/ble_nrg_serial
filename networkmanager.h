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
#define AUTH_INACTIVE 4U

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int authorized READ getAuthorized NOTIFY authorizedChanged)
    Q_PROPERTY(bool enabled MEMBER m_uploadEnabled NOTIFY uploadEnabledChanged)
private:
    LogFileHandler* m_logfile_handler_ptr;

    QNetworkReply *m_reply;
    QNetworkRequest m_request;
    uint8_t m_authorized = AUTH_UNAUTH;
    bool m_uploadEnabled;

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

    void createDriveFolder(QString name, QString uploadUrl);
    void uploadDataHttpMulti(QByteArray data, QString name, QString uploadUrl,
                             QByteArray contentType, QString folderID = "");
    void readFilesHttp (QString downloadUrl, QString folderID = "");

    uint8_t getAuthorized();

    Q_INVOKABLE void authorize();


signals:
    void authorizedChanged();
    void uploadEnabledChanged();

public slots:

    void uploadFinished(QNetworkReply *reply);
    void stateChanged(QAbstractOAuth::Status state);
    void authorizationGranted();
    void replyFinished(QNetworkReply *reply);
    void printNetworkReply (const QByteArray reply);
    void authenticationReply(QNetworkReply *reply, QAuthenticator *authenticator);
    void uploadCatchData(QString name, QByteArray data);
    void createNewFolderWithId(QString name);

};


#endif // NETWORKMANAGER_H
