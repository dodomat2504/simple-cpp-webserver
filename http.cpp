#include "h/http.h"
#include <iostream>
#include <sstream>
#include <json/json.h>
#include "h/string_trim.h"


using namespace http;

static int indexOf(const std::string& str, const char c) {
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == c)
            return i;
    }
    return -1;
}

static void removeCarrierReturn(std::string& str) {
    for (int i = str.size()-1; i >= 0; i--) {
        if (str[i] == '\r') {
            str.erase(i, 1);
        }
    }
}

Request http::parseHTTPRequest(const std::string& msg) {
    std::string message(msg);
    removeCarrierReturn(message);

    std::stringstream ss(message);
    std::string method, path, version;

    std::getline(ss, method, ' ');
    std::getline(ss, path, ' ');
    std::getline(ss, version, '\n');

    Request req;
    req.header.Method = HTTP_METHOD_fromString(method);
    req.header.Path = path;
    req.header.Version = version;

    std::string line;
    std::getline(ss, line, '\n');

    while (! line.empty()) {
        const int indexOfKeyEnd = indexOf(line, ':');

        if (indexOfKeyEnd == -2)
            throw std::runtime_error("Invalid HTTP header");

        const std::string key = line.substr(0, indexOfKeyEnd);
        std::string value = line.substr(indexOfKeyEnd+1, line.size()-indexOfKeyEnd);
        trim(value);

        if (key == "Host") {
            req.header.Host = value;
        } else if (key == "Connection") {
            req.header.Connection = value;
        } else if (key == "User-Agent") {
            req.header.UserAgent = value;
        } else if (key == "Accept") {
            req.header.Accept = value;
        } else if (key == "Content-Type") {
            req.header.ContentType = CONTENT_TYPE_fromString(value);
        }

        std::getline(ss, line, '\n');
    }

    std::string body;
    std::getline(ss, body, '\0');
    req.body.data = body;

    // try to validate JSON
    if (req.header.ContentType == CONTENT_TYPE::JSON) {
        Json::Value root;
        Json::Reader reader;
        const bool correctFormat = reader.parse(body, root, false);

        if (! correctFormat)
            throw std::runtime_error("Invalid JSON format");
    }
    
    return req;
}

std::string http::serializeHTTPResponse(const Response& res) {
    std::stringstream ss;
    ss << res.header.Version << " " << res.header.StatusCode << " " << res.header.StatusMessage << "\r\n";
    ss << "Connection: " << res.header.Connection << "\r\n";
    ss << "Content-Type: " << CONTENT_TYPE_toString(res.header.ContentType) << "\r\n";
    ss << "Access-Control-Allow-Origin: *\r\n";
    ss << "Content-Length: " << res.body.data.size() << "\r\n";
    ss << "\r\n";
    ss << res.body.data;
    ss << "\r\n\r\n";

    return ss.str();
}

std::string HTTP_METHOD_toString(HTTP_METHOD method) {
    switch (method) {
    case HTTP_METHOD::GET:
        return "GET";
    case HTTP_METHOD::POST:
        return "POST";
    case HTTP_METHOD::PUT:
        return "PUT";
    case HTTP_METHOD::DELETE:
        return "DELETE";
    default:
        return "UNSUPPORTED";
    }
}

HTTP_METHOD HTTP_METHOD_fromString(const std::string& method) {
    if (method == "GET")
        return HTTP_METHOD::GET;
    else if (method == "POST")
        return HTTP_METHOD::POST;
    else if (method == "PUT")
        return HTTP_METHOD::PUT;
    else if (method == "DELETE")
        return HTTP_METHOD::DELETE;
    else
        return HTTP_METHOD::UNSUPPORTED;
}

std::string CONTENT_TYPE_toString(CONTENT_TYPE type) {
    switch (type) {
    case CONTENT_TYPE::TEXT:
        return "text/plain";
    case CONTENT_TYPE::JSON:
        return "application/json";
    case CONTENT_TYPE::HTML:
        return "text/html";
    default:
        return "UNSUPPORTED";
    }
}

CONTENT_TYPE CONTENT_TYPE_fromString(const std::string& type) {
    if (type == "text/plain")
        return CONTENT_TYPE::TEXT;
    else if (type == "application/json")
        return CONTENT_TYPE::JSON;
    else if (type == "text/html")
        return CONTENT_TYPE::HTML;
    else
        return CONTENT_TYPE::UNSUPPORTED;
}
