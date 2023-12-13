#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>

#include "http.h"

class Endpoint {
private:
    const std::string _parent;
    const std::string _route;

    /// @brief The callback functions for the different routes
    std::unordered_map<HTTP_METHOD, std::function<http::Response(const http::Request&)>> _callbacks;

    /// @brief The children of this endpoint
    std::vector<Endpoint*> _children;
public:

    /// @brief Construct a new Endpoint object
    /// @param route The route
    Endpoint(const std::string& route, const std::string& parent = ""): _route(route), _parent(parent) {};

    /// @brief Destroy the Endpoint object and all children
    ~Endpoint();

    /// @brief Write the route to the output stream
    /// @param os Output stream
    /// @return Output stream
    std::ostream &operator<<(std::ostream &os) const;

    /// @brief Return the route as a string
    operator std::string() const;

    /// @brief Get the children of this endpoint
    /// @return Vector of children of this endpoint
    const std::vector<Endpoint*>& children() const;

    /// @brief Checks if this endpoint has the given child route
    /// @param route The route to check
    /// @return True if this endpoint has the given child route
    bool hasChildRoute(const std::string& route) const;

    /// @brief Checks if this endpoint has a callback function for the given HTTP method
    /// @param method The HTTP method to check
    /// @return True if this endpoint has a callback function for the given HTTP method
    bool hasCallbackFor(const HTTP_METHOD method) const;

    /// @brief Find the child endpoint with the given route
    /// @param route The route of the child endpoint
    /// @return The child endpoint with the given route
    Endpoint* operator[](const std::string& route) const;

    /// @brief Add a callback function for the given HTTP method
    /// @param method The HTTP method
    /// @param callback The callback function
    void addCallback(const HTTP_METHOD method, const std::function<http::Response(const http::Request&)>& callback);

    /// @brief Get the callback function for the given HTTP method
    /// @param method The HTTP method
    /// @return The callback function for the given HTTP method
    const std::function<http::Response(const http::Request&)>& getCallback(const HTTP_METHOD method) const;

    /// @brief Add a child endpoint
    /// @param child The child endpoint
    void addChild(Endpoint* child);

    /// @brief Get the route of this endpoint
    /// @return The route of this endpoint
    std::string fullPath() const;


    // STATIC FUNCTIONS

    /// @brief Split a route into its parts
    /// @param route The route to split
    /// @return Vector of route parts
    static std::vector<std::string> split(const std::string& route);
};
