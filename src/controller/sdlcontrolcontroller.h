#ifndef SDLCONTROLCONTROLLER_H
#define SDLCONTROLCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

using namespace stefanfrings;

class SdlControlController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(SdlControlController)
public:

    /** Constructor */
    SdlControlController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // SDLCONTROLCONTROLLER_H
