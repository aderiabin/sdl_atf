#ifndef SDLDATACONTROLLER_H
#define SDLDATACONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

using namespace stefanfrings;

class SdlDataController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(SdlDataController)
public:

    /** Constructor */
    SdlDataController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // SDLDATACONTROLLER_H
