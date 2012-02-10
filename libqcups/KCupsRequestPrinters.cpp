#include "KCupsRequestPrinters.h"

#include <KLocale>
#include <KDebug>

#define CUPS_DATADIR    "/usr/share/cups"

KCupsRequestPrinters::KCupsRequestPrinters()
{
}

void KCupsRequestPrinters::setAttributes(const QString &destName, bool isClass, const Arguments &values, const char *filename)
{
    if (values.isEmpty() && !filename) {
        setFinished();
        return;
    }

    if (KCupsConnection::readyToStart()) {
        Arguments request = values;
        request["printer-name"] = destName;
        request["printer-is-class"] = isClass;
        if (filename) {
            request["filename"] = filename;
        }

        ipp_op_e op;
        // TODO this seems weird now.. review this code..
        if (isClass && values.contains("member-uris")) {
            op = CUPS_ADD_CLASS;
        } else {
            op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;
        }


        m_retArguments = KCupsConnection::request(op,
                                                  "/admin/",
                                                  request,
                                                  false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setAttributes", destName, isClass, values, filename);
    }
}

void KCupsRequestPrinters::setShared(const QString &destName, bool isClass, bool shared)
{
    if (KCupsConnection::readyToStart()) {
        Arguments request;
        request["printer-name"] = destName;
        request["printer-is-class"] = isClass;
        request["printer-is-shared"] = shared;
        request["need-dest-name"] = true;

        ipp_op_e op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;

        m_retArguments = KCupsConnection::request(op,
                                                  "/admin/",
                                                  request,
                                                  false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setShared", destName, isClass, shared);
    }
}

void KCupsRequestPrinters::getAttributes(const QString &destName, bool isClass, const QStringList &requestedAttr)
{
    if (KCupsConnection::readyToStart()) {
        Arguments request;
        request["printer-name"] = destName;
        request["printer-is-class"] = isClass;
        request["need-dest-name"] = false; // we don't need a dest name since it's a single list
        request["requested-attributes"] = requestedAttr;

        m_retArguments = KCupsConnection::request(IPP_GET_PRINTER_ATTRIBUTES,
                                                  "/admin/",
                                                  request,
                                                  true);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getAttributes", destName, isClass, requestedAttr);
    }
}

void KCupsRequestPrinters::printTestPage(const QString &destName, bool isClass)
{
    if (KCupsConnection::readyToStart()) {
        Arguments request;
        request["printer-name"] = destName;
        request["printer-is-class"] = isClass;
        request["job-name"] = i18n("Test Page");
        char          resource[1024], /* POST resource path */
                      filename[1024]; /* Test page filename */
        const char    *datadir;       /* CUPS_DATADIR env var */

        /*
         * Locate the test page file...
         */
        datadir = qgetenv("CUPS_DATADIR").isEmpty() ? CUPS_DATADIR : qgetenv("CUPS_DATADIR") ;
        snprintf(filename, sizeof(filename), "%s/data/testprint", datadir);
        request["filename"] = filename;

        /*
         * Point to the printer/class...
         */
        snprintf(resource, sizeof(resource),
                 isClass ? "/classes/%s" : "/printers/%s", destName.toUtf8().data());

        m_retArguments = KCupsConnection::request(IPP_PRINT_JOB,
                                                  resource,
                                                  request,
                                                  false);
        // TODO old code..
//        result->setProperty("methodName", "printTestPage");

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printTestPage", destName, isClass);
    }
}

void KCupsRequestPrinters::printCommand(const QString &destName, const QString &command, const QString &title)
{
    if (KCupsConnection::readyToStart()) {
        do {
            int           job_id;                 /* Command file job */
            char          command_file[1024];     /* Command "file" */
            http_status_t status;                 /* Document status */
            cups_option_t hold_option;            /* job-hold-until option */

            /*
             * Create the CUPS command file...
             */
            snprintf(command_file, sizeof(command_file), "#CUPS-COMMAND\n%s\n", command.toUtf8().data());

            /*
             * Send the command file job...
             */
            hold_option.name  = const_cast<char*>("job-hold-until");
            hold_option.value = const_cast<char*>("no-hold");

            if ((job_id = cupsCreateJob(CUPS_HTTP_DEFAULT,
                                        destName.toUtf8(),
                                        title.toUtf8(),
                                        1,
                                        &hold_option)) < 1) {
                qWarning() << "Unable to send command to printer driver!";

                setError(IPP_NOT_POSSIBLE, i18n("Unable to send command to printer driver!"));
                setFinished();
                return;
            }

            status = cupsStartDocument(CUPS_HTTP_DEFAULT,
                                       destName.toUtf8(),
                                       job_id,
                                       NULL,
                                       CUPS_FORMAT_COMMAND,
                                       1);
            if (status == HTTP_CONTINUE) {
                status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file,
                                            strlen(command_file));
            }

            if (status == HTTP_CONTINUE) {
                cupsFinishDocument(CUPS_HTTP_DEFAULT, destName.toUtf8());
            }

            setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
            if (cupsLastError() >= IPP_REDIRECTION_OTHER_SITE) {
                qWarning() << "Unable to send command to printer driver!";

                cupsCancelJob(destName.toUtf8(), job_id);
                setFinished();
                return; // Return to avoid a new try
            }
        } while (KCupsConnection::retryIfForbidden());
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printCommand", destName, command, title);
    }
}

KIcon KCupsRequestPrinters::icon(const QString &destName, int printerType)
{
    // TODO get the ppd or something to get the real printer icon
    Q_UNUSED(destName)

    if (!(printerType & CUPS_PRINTER_COLOR)) {
        // If the printer is not color it is probably a laser one
        return KIcon("printer-laser");
    } else if (printerType & CUPS_PRINTER_SCANNER) {
        return KIcon("scanner");
    } else {
        return KIcon("printer");
    }
}
