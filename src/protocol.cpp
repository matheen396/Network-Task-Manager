#include "netask/protocol.hpp"
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace netask {
namespace {
std::uint64_t number(const std::string& s) {
    std::uint64_t n{}; auto [p, ec] = std::from_chars(s.data(), s.data()+s.size(), n);
    if (ec != std::errc{} || p != s.data()+s.size()) throw std::invalid_argument("invalid task id");
    return n;
}
std::string task_line(const Task& t) {
    return "TASK|" + std::to_string(t.id) + "|" + encode_field(t.title) + "|" + encode_field(t.description) + "|" + encode_field(t.assignee) + "|" + to_string(t.status);
}
}
std::string encode_field(const std::string& s) {
    static constexpr char hex[] = "0123456789ABCDEF"; std::string out;
    for (unsigned char c : s) {
        if (c == '%' || c == '|' || c == '\n' || c == '\r') { out += '%'; out += hex[c >> 4]; out += hex[c & 15]; }
        else out += static_cast<char>(c);
    }
    return out;
}
std::string decode_field(const std::string& s) {
    auto val=[](char c)->int { if(c>='0'&&c<='9')return c-'0'; if(c>='A'&&c<='F')return c-'A'+10; if(c>='a'&&c<='f')return c-'a'+10; return -1; };
    std::string out;
    for (std::size_t i=0;i<s.size();++i) {
        if (s[i]!='%') { out+=s[i]; continue; }
        if (i+2>=s.size() || val(s[i+1])<0 || val(s[i+2])<0) throw std::invalid_argument("invalid escaped field");
        out+=static_cast<char>((val(s[i+1])<<4)|val(s[i+2])); i+=2;
    }
    return out;
}
std::vector<std::string> split_fields(const std::string& line) {
    std::vector<std::string> fields; std::stringstream ss(line); std::string part;
    while (std::getline(ss, part, '|')) fields.push_back(part);
    if (!line.empty() && line.back()=='|') fields.emplace_back();
    return fields;
}
std::string handle_request(TaskManager& m, const std::string& request) {
    try {
        const auto f=split_fields(request);
        if (f.empty()) throw std::invalid_argument("empty request");
        if (f[0]=="PING" && f.size()==1) return "OK|PONG";
        if (f[0]=="LIST" && f.size()==1) {
            std::string out="OK|"; auto tasks=m.list();
            for (std::size_t i=0;i<tasks.size();++i) { if(i)out+='\n'; out+=task_line(tasks[i]); }
            return out;
        }
        if (f[0]=="CREATE" && f.size()==4) return "OK|"+task_line(m.create(decode_field(f[1]),decode_field(f[2]),decode_field(f[3])));
        if (f[0]=="UPDATE" && f.size()==4) return "OK|"+task_line(m.update(number(f[1]),decode_field(f[2]),decode_field(f[3])));
        if (f[0]=="ASSIGN" && f.size()==3) return "OK|"+task_line(m.assign(number(f[1]),decode_field(f[2])));
        if (f[0]=="STATUS" && f.size()==3) return "OK|"+task_line(m.set_status(number(f[1]),status_from_string(f[2])));
        if (f[0]=="DELETE" && f.size()==2) { m.remove(number(f[1])); return "OK|DELETED"; }
        throw std::invalid_argument("unknown command or wrong number of fields");
    } catch (const std::exception& e) { return std::string("ERR|")+encode_field(e.what()); }
}
}
