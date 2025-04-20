#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

using json = nlohmann::json;
using namespace httplib;

const std::string client_id = "YOUR_CLIENT_ID";
const std::string client_secret = "YOUR_CLIENT_SECRET";
const std::string redirect_uri = "YOUR_REDIRECT_URI";

std::string base64_encode(const std::string &in) {
    static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

int main() {
    Server svr;
    svr.Post("/auth/token", [](const Request &req, Response &res) {
        auto body = json::parse(req.body);
        std::string code = body["code"];
        std::string auth_str = client_id + ":" + client_secret;
        std::string auth_header = "Basic " + base64_encode(auth_str);
        Client cli("https://accounts.spotify.com");
        Headers headers = {
            {"Authorization", auth_header},
            {"Content-Type", "application/x-www-form-urlencoded"}
        };
        std::stringstream ss;
        ss << "grant_type=authorization_code"
           << "&code=" << code
           << "&redirect_uri=" << redirect_uri;
        auto token_res = cli.Post("/api/token", headers, ss.str(), "application/x-www-form-urlencoded");
        res.set_header("Access-Control-Allow-Origin", "*");
        if (token_res && token_res->status == 200) {
            res.set_content(token_res->body, "application/json");
        } else {
            res.status = 400;
            res.set_content(R"({"error": "Token request failed"})", "application/json");
        }
    });
    svr.listen("0.0.0.0", 8080);
}
