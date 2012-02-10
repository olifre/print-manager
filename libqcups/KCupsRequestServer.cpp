#include "KCupsRequestServer.h"

KCupsRequestServer::KCupsRequestServer(QObject *parent) :
    KCupsRequestInterface(parent)
{
}

void KCupsRequestServer::adminSetServerSettings(const HashStrStr &userValues);
void KCupsRequestServer::getPPDS(const QString &make = QString());

void KCupsRequestServer::getDevices();
// THIS function can get the default server dest through the
// "printer-is-default" attribute BUT it does not get user
// defined default printer, see cupsGetDefault() on www.cups.org for details
void KCupsRequestServer::getDests(int mask, const QStringList &requestedAttr = QStringList());
void KCupsRequestServer::getJobs(const QString &destName, bool myJobs, int whichJobs, const QStringList &requestedAttr = QStringList());

void KCupsRequestServer::addClass(const QHash<QString, QVariant> &values)
{
    if (values.isEmpty()) {
        return 0;
    }

    QHash<QString, QVariant> request(values);
    request["printer-is-class"] = true;
    request["printer-is-accepting-jobs"] = true;
    request["printer-state"] = IPP_PRINTER_IDLE;

    ipp_op_e op = CUPS_ADD_CLASS;;
//     if (isClass && values.contains("member-uris")) {
//         op = CUPS_ADD_CLASS;
//     } else {
//         op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;
//     }

    Result *result = new Result(QCupsConnection::instance());
    QMetaObject::invokeMethod(QCupsConnection::instance()->request(),
                              "request",
                              Qt::QueuedConnection,
                              Q_ARG(Result*, result),
                              Q_ARG(ipp_op_e, op),
                              Q_ARG(QString, "/admin/"),
                              Q_ARG(Arguments, request),
                              Q_ARG(bool, false));

    return result;
}

void KCupsRequestServer::adminGetServerSettings()
{
    if (KCupsConnection::readyToStart()) {
        int num_settings;
        cups_option_t *settings;
        QHash<QString, QString> ret;
        cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
        for (int i = 0; i < num_settings; i++) {
            QString name = QString::fromUtf8(settings[i].name);
            QString value = QString::fromUtf8(settings[i].value);
            ret[name] = value;
        }
        cupsFreeOptions(num_settings, settings);

        setHashStrStr(ret);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("adminGetServerSettings");
    }
}
