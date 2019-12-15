#include "sdlcontrolcontroller.h"
#include "global.h"
extern "C" {
#include <lua5.2/lua.h>
#include <lua5.2/lualib.h>
#include <lua5.2/lauxlib.h>
}

SdlControlController::SdlControlController() {}

void SdlControlController::service(HttpRequest& request, HttpResponse& response) {
    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

    if (request.getParameter("action")=="start") {
        response.write("<html><body>");
        // connect to SDL
        lua_State* L = lua_interpreter->getLuaState();
        lua_getglobal(L, "start");
        if (lua_pcall(L, 0, 0, 0) != 0) {
            response.write("SDL service has not started, vehicle data is not started to update");
        } else {
            response.write("SDL service has started, vehicle data is started to update from now");
        }
        response.write("<p> <a href=\"/sdl_control\">Return to SDL control page</a>");
        response.write("</body></html>",true);
     }
     else if (request.getParameter("action")=="stop") {
        response.write("<html><body>");
        lua_State* L = lua_interpreter->getLuaState();
        lua_getglobal(L, "stop");
        if (lua_pcall(L, 0, 0, 0) != 0) {
            response.write("SDL service has not stoped, vehicle data is still updating now");
        } else {
            response.write("SDL service has stoped, vehicle data is stopped to update from now");
        }
        response.write("<p> <a href=\"/sdl_control\">Return to SDL control page</a>");
        response.write("</body></html>",true);
        // disconnect from SDL
     }
     else {
        response.write("<html><body>");
        response.write("SDL service control page");
        response.write("<p><ul>");
        response.write("<li><a href=\"/sdl_control?action=start\">Start SDL service</a>");
        response.write("<li><a href=\"/sdl_control?action=stop\">Stop SDL service</a>");
        response.write("</ul></body></html>",true);
     }
}
