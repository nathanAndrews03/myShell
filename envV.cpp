#include <iostream>
#include <string>
#include <regex>
#include <vector>

std::vector<std::string> find_env_variables(const std::string& text) {
    std::regex env_var_pattern(R"(\$[A-Za-z_]\w*)");
    std::sregex_iterator begin(text.begin(), text.end(), env_var_pattern);
    std::sregex_iterator end;
    
    std::vector<std::string> matches;
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        matches.push_back(match.str());
    }
    return matches;
}

int main() {
    std::string text = "echo I like ${C}p${D}";
    std::vector<std::string> env_variables = find_env_variables(text);
    
    std::cout << "Found environment variables:" << std::endl;
    for (const auto& var : env_variables) {
        std::cout << var << std::endl;
    }
    
    return 0;
}

