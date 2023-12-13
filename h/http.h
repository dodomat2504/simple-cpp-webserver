#pragma once


#include <string>

enum class HTTP_METHOD {
    GET,
    POST,
    PUT,
    DELETE,
    UNSUPPORTED
};

enum class CONTENT_TYPE {
    TEXT,
    JSON,
    HTML,
    UNSUPPORTED
};

std::string HTTP_METHOD_toString(HTTP_METHOD method);

HTTP_METHOD HTTP_METHOD_fromString(const std::string& method);

std::string CONTENT_TYPE_toString(CONTENT_TYPE type);

CONTENT_TYPE CONTENT_TYPE_fromString(const std::string& type);

namespace http {
    struct BaseHeader {
        std::string Version = "";
        std::string Connection = "";
        
    };

    struct Body {
        std::string data = "";
    };

    namespace Req {
        struct Header : public BaseHeader {
            HTTP_METHOD Method = HTTP_METHOD::UNSUPPORTED;
            std::string Path = "";
            CONTENT_TYPE ContentType = CONTENT_TYPE::UNSUPPORTED;

            std::string Host = "";
            std::string UserAgent = "";
            std::string Accept = "";
        };
    }

    namespace Res {
        struct Header : public BaseHeader {
            unsigned int StatusCode = -1;
            std::string StatusMessage = "";
            CONTENT_TYPE ContentType = CONTENT_TYPE::TEXT;
            std::string AccessControlAllowOrigin = "*";
        };
    }

    struct Request {
        Req::Header header;
        Body body;
    };

    /**
     * @brief Parses a HTTP message
     * @param msg The message to parse
    */
    Request parseHTTPRequest(const std::string& msg);

    struct Response {
        Res::Header header;
        Body body;
    };

    /**
     * @brief Creates a HTTP response from a Response object
     * @param res The response object
    */
    std::string serializeHTTPResponse(const Response& res);

}
