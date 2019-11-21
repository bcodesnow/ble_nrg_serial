#include "networkmanager.h"

//NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
//{

//}

NetworkManager::NetworkManager(LogFileHandler *logfile_handler, QObject *parent) :
    QObject(parent),
    m_logfile_handler_ptr(logfile_handler)
{
    m_googleAuth = new QOAuth2AuthorizationCodeFlow();
    m_googleAuth->setScope(drive_file_scope);
    m_networkHandler = new QNetworkAccessManager();

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

    connect(m_logfile_handler_ptr, &LogFileHandler::invokeGoogleUpload, this, &NetworkManager::uploadCatchData);
    connect(m_logfile_handler_ptr, &LogFileHandler::invokeCreateGoogleFolder, this, &NetworkManager::createNewFolderWithId);

    // connect(m_control, &QLowEnergyController::connected, this, &DeviceController::onConnected);
}

void NetworkManager::authorize()
{
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

void NetworkManager::createDriveFolder(QString name, QString uploadUrl)
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

#if (VERBOSITY_LEVEL >= 2)
    qDebug()<<"NetworkManager::createDriveFolder"<<"Sending request:"<<request.rawHeaderList();
#endif

    QNetworkReply *reply = m_networkHandler->post(request,multiPart);

    connect(reply, &QNetworkReply::finished, [reply](){
#if (VERBOSITY_LEVEL >= 2)
        qDebug()<<"Folder upload request. Error? " << (reply->error() != QNetworkReply::NoError);
#else
        Q_UNUSED(reply)
#endif
    });

}

void NetworkManager::uploadDataHttpMulti(QByteArray data, QString name, QString uploadUrl, QByteArray contentType, QString folderID)
{
    uploadUrl.append(drive_type_multi);
    QNetworkRequest request(uploadUrl);

    const QString bearer = bearer_format.arg(QString(m_currentAccessToken));

    QHttpPart MetadataPart;
    MetadataPart.setRawHeader("Content-Type", "application/json; charset=UTF-8");
    QString body;
    if (folderID.isEmpty()) {
        body = "{\n" + tr("\"name\": \"%1\"\n").arg(name) + tr("}");
    }
    else {
        body = "{\n" + tr("\"name\": \"%1\",\n").arg(name)
                + tr("\"parents\": [\"%1\"]\n").arg(folderID)
                + tr("}");
    }

    MetadataPart.setBody(body.toUtf8());

    QHttpPart MediaPart;
    MediaPart.setRawHeader("Content-Type", contentType);
    MediaPart.setBody(data);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::RelatedType);
    multiPart->setBoundary("bound1");
    multiPart->append(MetadataPart);
    multiPart->append(MediaPart);

    request.setRawHeader("Content-Type","multipart/related; boundary=bound1");
    request.setRawHeader("Authorization", bearer.toUtf8());

#if (VERBOSITY_LEVEL >= 2)
    qDebug()<<"NetworkManager::uploadDataHttpMulti"<<"Sending request:"<<request.rawHeaderList();
#endif

    QNetworkReply *reply = m_networkHandler->post(request,multiPart);

    connect(reply, &QNetworkReply::finished, [reply](){
#if (VERBOSITY_LEVEL >= 2)
        qDebug()<<"File upload request. Error? " << (reply->error() != QNetworkReply::NoError);
        qDebug() << reply->readAll();
#else
        Q_UNUSED(reply)
#endif
    });
}

void NetworkManager::readFilesHttp(QString downloadUrl, QString folderID)
{
    if (folderID != "")
    {
        // read from folder
    }
    else
    {
        QNetworkRequest request(downloadUrl);
        const QString bearer = bearer_format.arg(QString(m_currentAccessToken));
        QNetworkReply *reply = m_networkHandler->get(request);

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug()<<"Read filenames request. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });
    }
}

uint8_t NetworkManager::getAuthorized()
{
    return m_authorized;
}

void NetworkManager::uploadFinished(QNetworkReply *reply)
{
    qDebug()<<"NetworkManager: Upload finished"<<reply->url();
}

void NetworkManager::stateChanged(QAbstractOAuth::Status state)
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

void NetworkManager::authorizationGranted() {
#if (VERBOSITY_LEVEL >= 1)
    qDebug() << "Google authorization granted. Expires at:" << m_googleAuth->expirationAt().toString();
#endif
}

void NetworkManager::replyFinished(QNetworkReply *reply)
{
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"Network reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
#endif
    QByteArray replyData = reply->readAll();
    QList<QByteArray> replyList = replyData.split('\"');

    if ( replyList.at(15).contains("folder"))
    {
        qDebug()<<"FOLDER ID"<<replyList.at(7);
        m_curr_folder_id = replyList.at(7);
    }
}

void NetworkManager::printNetworkReply(const QByteArray reply)
{
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

#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"OAuth reply arrived. Access Token:"<<m_currentAccessToken<<"Expiration:"<<m_accessExpirationIn;
    qDebug()<<"Full reply:"<<endl<<reply;
#endif
}

void NetworkManager::authenticationReply(QNetworkReply *reply, QAuthenticator *authenticator) {
    // unused?
#if (VERBOSITY_LEVEL >= 2)
    qDebug()<<"Authentication reply arrived. Error? " << (reply->error() != QNetworkReply::NoError);
    qDebug()<<"auth user:"<<authenticator->user();
    qDebug()<<"auth password:"<<authenticator->password();
    qDebug()<<"auth realm:"<<authenticator->realm();
    qDebug()<<reply->readAll();
#endif
}

void NetworkManager::uploadCatchData(QString filename, QByteArray data) {
    if (m_authorized == AUTH_SUCCESS && m_uploadEnabled == true)
    {
#if (VERBOSITY_LEVEL >= 1)
        qDebug()<<"Uploading data:"<<filename;
#endif
        //  uploadDataHttpMulti(data, m_logfile_handler_ptr->getHomeLocation()+"/"+path, file_upload_url, "text/plain");
        uploadDataHttpMulti(data, filename, file_upload_url, "text/plain", m_curr_folder_id);
    }
    else {
#if (VERBOSITY_LEVEL >= 1)
        qDebug()<<"Uploading not enabled";
#endif
    }
}

void NetworkManager::createNewFolderWithId(QString name)
{
    if (m_authorized == AUTH_SUCCESS)
    {
#if (VERBOSITY_LEVEL >= 1)
        qDebug()<<"Creating new drive folder:"<<name;
#endif
        m_curr_folder_id.clear();
        createDriveFolder(name,file_upload_url);
    }
    else {
#if (VERBOSITY_LEVEL >= 1)
        qDebug()<<"Uploading not enabled";
#endif
    }
}
