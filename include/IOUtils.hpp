#pragma once

#include <Lexer.hpp>

#include <string_view>
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

    auto to_string(std::span<Token> tokens) -> std::string;

    auto read_all(std::string_view filename) -> std::string;
}