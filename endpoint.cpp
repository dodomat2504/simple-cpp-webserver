#include "h/endpoint.h"


std::ostream& Endpoint::operator<<(std::ostream &os) const {
    os << _route;
    return os;
}

Endpoint::~Endpoint() {
    for (Endpoint* child : _children)
        delete child;
}

Endpoint::operator std::string() const {
    return _route;
}

const std::vector<Endpoint*>& Endpoint::children() const {
    return _children;
}

bool Endpoint::hasChildRoute(const std::string& route) const {
    for (const Endpoint* child : _children)
        if (child->_route == "*" || child->_route == route)
            return true;

    return false;
}

std::vector<std::string> Endpoint::split(const std::string& route) {
    std::vector<std::string> result;
    std::string current = "";

    for (char c : route) {
        if (c == '/') {
            if (current != "")
                result.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }

    if (current != "")
        result.push_back(current);

    return result;
}

bool Endpoint::hasCallbackFor(const HTTP_METHOD method) const {
    return _callbacks.find(method) != _callbacks.end();
}

Endpoint* Endpoint::operator[](const std::string& route) const {
    Endpoint* wildcard = nullptr;

    for (Endpoint* child : _children) {
        if (child->_route == "*")
            wildcard = child;
        
        if (child->_route == route)
            return child;
    }

    return wildcard;
}

void Endpoint::addCallback(const HTTP_METHOD method, const std::function<http::Response(const http::Request&)>& callback) {
    if (hasCallbackFor(method))
        throw std::runtime_error("Callback for '" + HTTP_METHOD_toString(method) + " " + _parent + "/" + _route + "' already exists");

    _callbacks[method] = callback;
}

void Endpoint::addChild(Endpoint* child) {
    if (hasChildRoute(child->_route))
        throw std::runtime_error("Child route '" + child->_route + "' already exists");

    _children.push_back(child);
}

const std::function<http::Response(const http::Request&)>& Endpoint::getCallback(const HTTP_METHOD method) const {
    if (!hasCallbackFor(method))
        throw std::runtime_error("No callback for '" + HTTP_METHOD_toString(method) + " " + _parent + "/" + _route + "'");

    return _callbacks.find(method)->second;
}

std::string Endpoint::fullPath() const {
    return _parent == "" ? _route : _parent + "/" + _route;
}
