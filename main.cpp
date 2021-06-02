#include <iostream>

#include <Lexer.hpp>
#include <IOUtils.hpp>

int main(int argc, char** argv) {

    if (argc > 1) {
        auto res = lab::Lexer{}.process(lab::read_all(argv[1]));
        std::cout << to_string(res);
    }
}