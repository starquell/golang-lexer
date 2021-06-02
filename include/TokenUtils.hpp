#pragma once

#include <fmt/format.h>
#include <Lexer.hpp>

#include <fstream>
#include <array>
#include <span>

namespace lab {

    constexpr auto to_string(Token::Type type) -> std::string_view
    {
        constexpr std::array names = {
            "identifier",
            "bool",
            "int",
            "float",
            "imaginary",
            "rune",
            "string",
            "nil",
            "keyword",
            "operator",
            "punctuation",
            "comment",
            "error"
        };
        return names[static_cast<std::size_t>(type)];
    }

    auto to_string(std::span<Token> tokens) -> std::string
    {
        std::string res;
        const auto index_max_width = static_cast<std::size_t>(std::log10(tokens.size())) + 1;

        for (std::size_t i = 0; i < tokens.size(); ++i) {
            fmt::format_to(std::back_inserter(res), "{:>{}}  {:>12}: {}\n", i, index_max_width, to_string(tokens[i].type), tokens[i].token);
        }
        return res;
    }

    auto read_all(std::string_view filename) -> std::string
    {
        std::ifstream stream(filename.data());
        return {
            std::istreambuf_iterator<char>(stream),
            std::istreambuf_iterator<char>()
        };
    }
}