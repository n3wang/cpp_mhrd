#include "simulator.h"
#include <sstream>
#include <regex>
#include <stdexcept>
#include <cctype>

static inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static inline void replaceAll(std::string& s, const std::string& a, const std::string& b) {
    size_t pos = 0;
    while ((pos = s.find(a, pos)) != std::string::npos) {
        s.replace(pos, a.size(), b);
        pos += b.size();
    }
}

static std::string blockOf(const std::string& text, const std::string& key) {
    std::regex re(key + R"(:\s*([^;]+);)", std::regex::icase);
    std::smatch m;
    if (std::regex_search(text, m, re)) return trim(m[1]);
    return "";
}

static std::vector<std::string> parseList(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ',') {
            if (!trim(cur).empty()) out.push_back(trim(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!trim(cur).empty()) out.push_back(trim(cur));
    return out;
}

AST parseHDL(const std::string& src) {
    std::string line, out;
    std::stringstream ss(src);
    while (std::getline(ss, line)) {
        auto p = line.find("//");
        if (p != std::string::npos) line = line.substr(0, p);
        out += line + " ";
    }
    replaceAll(out, "->", "->");
    
    AST ast;
    for (auto& n : parseList(blockOf(out, "Inputs"))) ast.inputs.push_back(n);
    for (auto& n : parseList(blockOf(out, "Outputs"))) ast.outputs.push_back(n);
    
    std::string parts = blockOf(out, "Parts");
    if (!parts.empty()) {
        for (auto& p : parseList(parts)) {
            auto c = p.find(':');
            if (c == std::string::npos) throw std::runtime_error("Bad part: " + p);
            ast.parts.push_back({trim(p.substr(0, c)), trim(p.substr(c + 1))});
        }
    }
    
    std::string wires = blockOf(out, "Wires");
    if (!wires.empty()) {
        for (auto& w : parseList(wires)) {
            auto a = w.find("->");
            if (a == std::string::npos) throw std::runtime_error("Bad wire: " + w);
            ast.wires.push_back({trim(w.substr(0, a)), trim(w.substr(a + 2))});
        }
    }
    return ast;
}

static GateDef gateOf(const std::string& kind) {
    std::string k;
    for (char c : kind) k += std::tolower(c);
    if (k == "not") return {{"in"}, {"out"}, [](auto const& p) { return std::unordered_map<std::string, int>{{"out", p.at("in") ^ 1}}; }};
    if (k == "and") return {{"in1", "in2"}, {"out"}, [](auto const& p) { return std::unordered_map<std::string, int>{{"out", (p.at("in1") & p.at("in2"))}}; }};
    if (k == "or") return {{"in1", "in2"}, {"out"}, [](auto const& p) { return std::unordered_map<std::string, int>{{"out", (p.at("in1") | p.at("in2"))}}; }};
    if (k == "xor") return {{"in1", "in2"}, {"out"}, [](auto const& p) { return std::unordered_map<std::string, int>{{"out", (p.at("in1") ^ p.at("in2"))}}; }};
    if (k == "nand") return {{"in1", "in2"}, {"out"}, [](auto const& p) { return std::unordered_map<std::string, int>{{"out", ((p.at("in1") & p.at("in2")) ^ 1)}}; }};
    if (k == "nor") return {{"in1", "in2"}, {"out"}, [](auto const& p) { return std::unordered_map<std::string, int>{{"out", ((p.at("in1") | p.at("in2")) ^ 1)}}; }};
    throw std::runtime_error("Unknown gate kind: " + kind);
}

static std::string pinKey(const std::string& part, const std::string& pin) {
    return "part:" + part + "." + pin;
}

static std::string resolveSrc(const AST& ast, const std::unordered_map<std::string, int>& have, const std::string& ep) {
    if (ep.find('.') != std::string::npos) {
        std::string part = ep.substr(0, ep.find('.'));
        std::string pin = ep.substr(ep.find('.') + 1);
        std::string key = pinKey(part, pin);
        if (!have.count(key)) throw std::runtime_error("Unknown src pin: " + ep);
        return key;
    }
    std::string inKey = "inp:" + ep;
    if (have.count(inKey)) return inKey;
    throw std::runtime_error("Ambiguous/unknown src: " + ep);
}

static std::string resolveDst(const AST& ast, const std::unordered_map<std::string, int>& have, const std::string& ep) {
    if (ep.find('.') != std::string::npos) {
        std::string part = ep.substr(0, ep.find('.'));
        std::string pin = ep.substr(ep.find('.') + 1);
        std::string key = pinKey(part, pin);
        if (!have.count(key)) throw std::runtime_error("Unknown dst pin: " + ep);
        return key;
    }
    std::string outKey = "out:" + ep;
    if (have.count(outKey)) return outKey;
    throw std::runtime_error("Ambiguous/unknown dst: " + ep);
}

Net buildNet(const AST& ast) {
    Net net;
    net.ast = ast;
    for (auto& i : ast.inputs) net.val["inp:" + i] = 0;
    for (auto& o : ast.outputs) net.val["out:" + o] = 0;
    for (auto& p : ast.parts) {
        auto g = gateOf(p.kind);
        net.partDef[p.name] = g;
        for (auto& ip : g.inPins) net.val[pinKey(p.name, ip)] = 0;
        for (auto& op : g.outPins) net.val[pinKey(p.name, op)] = 0;
    }
    for (auto& w : ast.wires) {
        std::string s = resolveSrc(ast, net.val, w.src);
        std::string d = resolveDst(ast, net.val, w.dst);
        net.fan[s].push_back(d);
    }
    return net;
}

std::unordered_map<std::string, int> simulate(Net& net, const std::unordered_map<std::string, int>& inVec) {
    for (auto& [k, v] : inVec) net.val["inp:" + k] = v & 1;
    bool changed = true;
    int guard = 0;
    while (changed && guard++ < 64) {
        changed = false;
        for (auto& kv : net.partDef) {
            const std::string& name = kv.first;
            const GateDef& g = kv.second;
            std::unordered_map<std::string, int> pins;
            for (auto& p : g.inPins) pins[p] = net.val[pinKey(name, p)] & 1;
            auto outs = g.eval(pins);
            for (auto& ov : outs) {
                std::string key = pinKey(name, ov.first);
                int nv = ov.second & 1;
                if (net.val[key] != nv) {
                    net.val[key] = nv;
                    changed = true;
                }
            }
        }
        for (auto& e : net.fan) {
            int sv = net.val[e.first] & 1;
            for (auto& d : e.second) {
                if (net.val[d] != sv) {
                    net.val[d] = sv;
                    changed = true;
                }
            }
        }
    }
    std::unordered_map<std::string, int> out;
    for (auto& o : net.ast.outputs) out[o] = net.val["out:" + o] & 1;
    return out;
}

std::vector<std::unordered_map<std::string, int>> allCombos(const std::vector<std::string>& names) {
    std::vector<std::unordered_map<std::string, int>> v;
    int n = static_cast<int>(names.size());
    for (int m = 0; m < (1 << n); ++m) {
        std::unordered_map<std::string, int> t;
        for (int i = 0; i < n; ++i) t[names[i]] = (m >> i) & 1;
        v.push_back(std::move(t));
    }
    return v;
}

