#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <csignal>
#include <csetjmp>
#include "global.h"
#include "httplistener.h"
#include "requestmapper.h"

using namespace stefanfrings;

namespace {
void abrt_handler(int signal) {
  _exit(1);
}
}

/** Search the configuration file */
QString searchConfigFile() {
    QString binDir=QCoreApplication::applicationDirPath();
    QString appName=QCoreApplication::applicationName();
    QString fileName(appName+".ini");

    QStringList searchList;
    searchList.append(binDir);
    searchList.append(binDir+"/etc");
    // searchList.append(binDir+"/../etc");
    // searchList.append(binDir+"/../../etc"); // for development without shadow build
    // searchList.append(binDir+"/../"+appName+"/etc"); // for development with shadow build
    // searchList.append(binDir+"/../../"+appName+"/etc"); // for development with shadow build
    // searchList.append(binDir+"/../../../"+appName+"/etc"); // for development with shadow build
    // searchList.append(binDir+"/../../../../"+appName+"/etc"); // for development with shadow build
    // searchList.append(binDir+"/../../../../../"+appName+"/etc"); // for development with shadow build
    // searchList.append(QDir::rootPath()+"etc/opt");
    // searchList.append(QDir::rootPath()+"etc");

    foreach (QString dir, searchList)
    {
        QFile file(dir+"/"+fileName);
        if (file.exists())
        {
            // found
            fileName=QDir(file.fileName()).canonicalPath();
            qDebug("Using config file %s",qPrintable(fileName));
            return fileName;
        }
    }

    // not found
    foreach (QString dir, searchList)
    {
        qWarning("%s/%s not found",qPrintable(dir),qPrintable(fileName));
    }
    qFatal("Cannot find config file %s",qPrintable(fileName));
}


/**
  Entry point of the program.
*/
int main(int argc, char *argv[])
{
    struct sigaction sa, oldsa;
    sa.sa_handler = abrt_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGABRT, &sa, &oldsa);
    sa.sa_flags = 0;

    QCoreApplication app(argc, argv);

    app.setApplicationName("SdlApp");
    app.setOrganizationName("Luxoft");

    // Find the configuration file
    QString configFileName=searchConfigFile();

    // Configure logging into a file
    QSettings* logSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    logSettings->beginGroup("logging");
    FileLogger* logger=new FileLogger(logSettings,10000,&app);
    logger->installMsgHandler();

    // Configure template loader and cache
    QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    templateSettings->beginGroup("templates");
    templateCache=new TemplateCache(templateSettings,&app);

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    sessionSettings->beginGroup("sessions");
    sessionStore=new HttpSessionStore(sessionSettings,&app);

    // Configure static file controller
    QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    fileSettings->beginGroup("docroot");
    staticFileController=new StaticFileController(fileSettings,&app);

    // Configure Lua interpreter
    QString fileName = "modules/app/main.lua";
    QStringList arguments(fileName);
    auto arg = arguments.begin();
    lua_interpreter = new LuaInterpreter(&app, arg, arguments.end());

    int res = lua_interpreter->load(fileName.toUtf8().constData());
    if (res) {
      return res;
    }

    // Configure and start the TCP listener
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,&app);
    listenerSettings->beginGroup("listener");
    new HttpListener(listenerSettings,new RequestMapper(&app),&app);

    qInfo("Application has started\nQtWebAppLib version: %s", getQtWebAppLibVersion());
    return app.exec();
    qInfo("Application has stopped");
}
