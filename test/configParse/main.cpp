#include <iostream>
#include <cassert>
#include <fstream>
#include "../../inc/configParse/configParse.hpp"

using namespace std;

// Helper to write temporary config file
void writeConfig(const string& path, const string& content) {
    ofstream file(path.c_str());
    file << content;
    file.close();
}

// Test parsing a minimal valid server block
void test_valid_server() {
    string cfg = 
        "server {\n"
        "listen 8080\n"
        "host 127.0.0.1\n"
        "server_name myserver\n"
        "root /var/www\n"
        "client_max_body_size 42069\n"
        "}\n";
    string path = "test/configParse/test_valid_server.conf";
    writeConfig(path, cfg);

    map<int, ServerConfig> conf = parseConfig(path);
    assert(conf.size() == 1);
    ServerConfig sc = conf[8080];
    assert(sc.port == 8080);
    assert(sc.host == "127.0.0.1");
    assert(sc.server_name == "myserver");
    assert(sc.root == "/var/www");
    assert(sc.clientMaxBodySize == 42069);
    cout << "test_valid_server passed\n";
}

// Test parsing a location block with CGI
void test_location_cgi() {
    string cfg =
        "server {\n"
        "listen 9090\n"
        "host 0.0.0.0\n"
        "server_name cgisrv\n"
        "root /srv/www\n"
        "client_max_body_size 42069\n"
        "location /cgi {\n"
        "cgi_ext .py .php\n"
        "cgi_path /usr/bin/python3 /usr/bin/php\n"
        "}\n"
        "}\n";
    string path = "test/configParse/test_location_cgi.conf";
    writeConfig(path, cfg);

    map<int, ServerConfig> conf = parseConfig(path);
    ServerConfig sc = conf[9090];
    assert(sc.pathToLoc.size() == 1);
    Location loc = sc.pathToLoc["/cgi"];
    assert(loc.cgiHandler[".py"] == "/usr/bin/python3");
    assert(loc.cgiHandler[".php"] == "/usr/bin/php");
    cout << "test_location_cgi passed\n";
}

// Test invalid directive throws exception
void test_invalid_directive() {
    string cfg =
        "server {\n"
        "listen 8081\n"
        "foo_bar something\n"
        "}\n";
    string path = "test/configParse/test_invalid_directive.conf";
    writeConfig(path, cfg);

    try {
        map<int, ServerConfig> conf = parseConfig(path);
        assert(false); // should not reach here
    } catch (...) {
        cout << "test_invalid_directive passed\n";
    }
}

// Test missing closing braces
void test_missing_brace() {
    string cfg =
        "server {\n"
        "listen 8082\n"
        "host 127.0.0.1\n";
    string path = "test/configParse/test_missing_brace.conf";
    writeConfig(path, cfg);

    try {
        map<int, ServerConfig> conf = parseConfig(path);
        assert(false);
    } catch (...) {
        cout << "test_missing_brace passed\n";
    }
}

// Test allow_methods parsing
void test_allow_methods() {
    string cfg =
        "server {\n"
        "listen 8083\n"
        "host 127.0.0.1\n"
        "location / {\n"
        "allow_methods GET POST DELETE\n"
        "}\n"
        "}\n";
    string path = "test/configParse/test_allow_methods.conf";
    writeConfig(path, cfg);

    map<int, ServerConfig> conf = parseConfig(path);
    Location loc = conf[8083].pathToLoc["/"];
    assert(loc.allowedMethods.size() == 3);
    assert(loc.allowedMethods[0] == "GET");
    assert(loc.allowedMethods[1] == "POST");
    assert(loc.allowedMethods[2] == "DELETE");
    cout << "test_allow_methods passed\n";
}

// Test return redirect parsing
void test_return_redirect() {
    string cfg =
        "server {\n"
        "listen 8084\n"
        "host 127.0.0.1\n"
        "location /old {\n"
        "return 301 /new\n"
        "}\n"
        "}\n";
    string path = "test/configParse/test_return_redirect.conf";
    writeConfig(path, cfg);

    map<int, ServerConfig> conf = parseConfig(path);
    Location loc = conf[8084].pathToLoc["/old"];
    assert(loc.hasRedir);
    assert(loc.redirCode == (StatusCode)301);
    assert(loc.redirURL == "/new");
    cout << "test_return_redirect passed\n";
}

int main() {
    test_valid_server();
    test_location_cgi();
    test_invalid_directive();
    test_missing_brace();
    test_allow_methods();
    test_return_redirect();

	cerr << "\e[0;32m CONF OK\n\e[0m";
    return 0;
}
