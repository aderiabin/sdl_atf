// #line 16 "main.nw"
#include <getopt.h>
#include <iostream>
// #line 5 "main.nw"
extern "C" {
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
}
// #line 19 "main.nw"
#include <QObject>
#include <QStringList>
#include <QCoreApplication>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <csignal>
#include <csetjmp>
#include "lua_interpreter.h"

// #line 32 "main.nw"
namespace {
void abrt_handler(int signal) {
  _exit(1);
}
}

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);

  QString fileName = "modules/app/main.lua";
  QStringList arguments(fileName);
  auto arg = arguments.begin();

  LuaInterpreter lua_interpreter(&app, arg, arguments.end());

  struct sigaction sa, oldsa;
  sa.sa_handler = abrt_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGABRT, &sa, &oldsa);
  sa.sa_flags = 0;

  int res = lua_interpreter.load(fileName.toUtf8().constData());
  if (res) {
    return res;
  }
  if (lua_interpreter.quitCalled) {
    return lua_interpreter.retCode;
  }
  return app.exec();
}
