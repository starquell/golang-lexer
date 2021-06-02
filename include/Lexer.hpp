#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <memory>

namespace lab {

     struct Token {

        enum class Type {
            Identifier,
            BoolLiteral,
            IntLiteral,
            FloatLiteral,
            ImagLiteral,
            RuneLiteral,
            StringLiteral,
            Nil,
            Keyword,
            Operator,
            PunctuationMark,
            Comment,
            Error
        };

        Type type;
        std::string token;
    };

    class Lexer {

    public:
        Lexer();
        ~Lexer();

        auto process(std::string_view input) -> std::vector<Token>;

    private:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };
}