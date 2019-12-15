#include "sdldatacontroller.h"
#include "global.h"

SdlDataController::SdlDataController() {}

void SdlDataController::service(HttpRequest& request, HttpResponse& response) {

    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");
    if (!lua_interpreter) {
        response.write("{\"check\":\"Existing of LUA interpreter\",\"result\":false}", true);
    } else {
        response.write("{\"check\":\"Existing of LUA interpreter\",\"result\":true}", true);
    }
}
