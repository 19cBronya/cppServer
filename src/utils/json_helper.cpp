#include "utils/json_helper.h"
#include <algorithm>
#include <cctype>

std::map<std::string, std::string> JsonHelper::parseSimpleObject(const std::string& json) {
    std::map<std::string, std::string> result;
    
    // 简单解析：查找 "key": "value" 模式
    size_t pos = 0;
    while (pos < json.length()) {
        // 查找键的开始
        size_t keyStart = json.find('"', pos);
        if (keyStart == std::string::npos) break;
        keyStart++;
        
        size_t keyEnd = json.find('"', keyStart);
        if (keyEnd == std::string::npos) break;
        
        std::string key = json.substr(keyStart, keyEnd - keyStart);
        
        // 查找冒号
        size_t colon = json.find(':', keyEnd);
        if (colon == std::string::npos) break;
        
        // 查找值的开始
        size_t valueStart = json.find('"', colon);
        if (valueStart == std::string::npos) break;
        valueStart++;
        
        size_t valueEnd = json.find('"', valueStart);
        if (valueEnd == std::string::npos) break;
        
        std::string value = json.substr(valueStart, valueEnd - valueStart);
        
        result[key] = value;
        pos = valueEnd + 1;
    }
    
    return result;
}

std::string JsonHelper::createObject(const std::map<std::string, std::string>& data) {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& pair : data) {
        if (!first) {
            oss << ",";
        }
        oss << "\"" << escape(pair.first) << "\":\"" << escape(pair.second) << "\"";
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

std::string JsonHelper::escape(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:   oss << c; break;
        }
    }
    return oss.str();
}

std::string JsonHelper::getValue(const std::string& json, const std::string& key) {
    auto data = parseSimpleObject(json);
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}
