#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <string>
#include <sstream>
#include <map>
#include <vector>

// 简单的 JSON 帮助类（不依赖第三方库）
class JsonHelper {
public:
    // 解析简单的 JSON 对象 {"key": "value", ...}
    static std::map<std::string, std::string> parseSimpleObject(const std::string& json);
    
    // 创建 JSON 对象字符串
    static std::string createObject(const std::map<std::string, std::string>& data);
    
    // 转义 JSON 字符串
    static std::string escape(const std::string& str);
    
    // 提取值（简单实现）
    static std::string getValue(const std::string& json, const std::string& key);
};

#endif // JSON_HELPER_H
