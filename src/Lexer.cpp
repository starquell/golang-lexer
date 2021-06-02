#include <Lexer.hpp>

#include <unordered_set>

namespace {

    auto is_punctuation(char ch) -> bool
    {
        static const std::unordered_set marks = {
                ',', ';', '(', ')', '[', ']', '{', '}'
        };
        return marks.contains(ch);
    }

    auto is_keyword(std::string_view str) -> bool
    {
        static const std::unordered_set<std::string_view> keywords = {
                "break", "case", "chan", "const", "continue",
                "default", "defer", "else", "fallthrough", "for",
                "func", "go", "goto", "if", "import",
                "interface", "map", "package", "range", "return",
                "select", "struct", "switch", "type", "var"
        };
        return keywords.contains(str);
    }

    auto is_operator(char ch) -> bool
    {
        static const std::unordered_set operators = {
                '=', '+', '-', '>', '<', '&', '|', '/', '*', '!', '%', '^'
        };
        return operators.contains(ch);
    }

    auto is_hex_digit(char ch) -> bool
    {
        const auto digit = std::tolower(ch);
        return std::isdigit(ch) || digit == 'a' || digit == 'b' || digit == 'c'
                                || digit == 'd' || digit == 'e' || digit == 'f';
    }

    auto ltrimed(std::string_view s) -> std::string_view
    {
        return {
            std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch);}),
            s.end()
        };
    }
}

class lab::Lexer::Impl {

    enum class State {
        Start,
        Rune,
        RuneEscaping,
        RuneProcessing,
        StringProcessing,
        StringEscaping,
        Punctiation,
        Zero,
        Binary,      // for ints only
        Hex,         // for ints only
        Octal,       // for ints only
        Decimal,
        Floating,
        Op,
        Plus,
        Minus,
        Slash,
        Ampersand,
        Or,
        Shift,
        Ls,
        Gt,
        Dot,
        Colon,
        Text,
        LineComment,
        BlockComment,
        ClosingBlockComment,
        Error
    };

public:
    auto process(std::string_view input) -> std::vector<Token>;

private:
    void add_token(Token::Type type, bool take_last = false);

    template<State state>
    void handle(char ch);

private:
    State _curr_state = State::Start;
    std::vector<Token> _tokens;
    std::string _curr_token;
};

template <lab::Lexer::Impl::State state>
void lab::Lexer::Impl::handle(char ch) {}

// one state - one specialization
template <>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Start>(char ch)
{
    if (std::isspace(ch))        _curr_state = State::Start;
    else if (ch == '\'')         _curr_state = State::Rune;
    else if (ch == '\"')         _curr_state = State::StringProcessing;
    else if (ch == '0')          _curr_state = State::Zero;
    else if (std::isdigit(ch))   _curr_state = State::Decimal;
    else if (ch == '.')          _curr_state = State::Dot;
    else if (is_punctuation(ch)) _curr_state = State::Punctiation;
    else if (std::isalpha(ch)
            || ch == '_')        _curr_state = State::Text;
    else if (ch == '+')          _curr_state = State::Plus;
    else if (ch == '-')          _curr_state = State::Minus;
    else if (ch == '>')          _curr_state = State::Gt;
    else if (ch == '<')          _curr_state = State::Ls;
    else if (ch == '&')          _curr_state = State::Ampersand;
    else if (ch == '^'
            || ch == '!'
            || ch == '*'
            || ch == '='
            || ch == '%')        _curr_state = State::Op;
    else if (ch == ':')          _curr_state = State::Colon;
    else if (ch == '|')          _curr_state = State::Or;
    else if (ch == '/')          _curr_state = State::Slash;
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Error>(char ch)
{
    add_token(Token::Type::Error);
    handle<State::Start>(ch);
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Punctiation>(char ch)
{
    add_token(Token::Type::PunctuationMark);
    handle<State::Start>(ch);
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Rune>(char ch)
{
    if (ch != ' ' && std::isspace(ch)) {
        handle<State::Error>(ch);
    } else if (ch == '\\') {
        _curr_state = State::RuneEscaping;
    } else {
        _curr_state = State::RuneProcessing;
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::RuneProcessing>(char ch)
{
    if (ch == '\'') {
        add_token(Token::Type::RuneLiteral, true);
    } else {
        _curr_state = State::Error;
    }
}
template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::RuneEscaping>(char ch)
{
    _curr_state = State::RuneProcessing;
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::StringProcessing>(char ch)
{
    if (ch == '\"') {
        add_token(Token::Type::StringLiteral, true);
    } else if (ch == '\\') {
        _curr_state = State::StringEscaping;
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::StringEscaping>(char ch)
{
    _curr_state = State::StringProcessing;
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Zero>(char ch)
{
    if (std::tolower(ch) == 'b') {
        _curr_state = State::Binary;
    } else if (std::tolower(ch) == 'o') {
        _curr_state = State::Octal;
    } else if (std::tolower(ch) == 'x') {
        _curr_state = State::Hex;
    } else if (std::isdigit(ch)) {
        _curr_state = State::Decimal;
    } else {
        add_token(Token::Type::IntLiteral);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Decimal>(char ch)
{
    if (std::isspace(ch)) {
        add_token(Token::Type::IntLiteral);
    } else if (ch == '.') {
        _curr_state = State::Floating;
    } else if (ch == 'i') {
        add_token(Token::Type::ImagLiteral, true);
    } else if (!std::isdigit(ch)) {
        add_token(Token::Type::IntLiteral);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Binary>(char ch)
{
    if (std::isspace(ch)) {
        add_token(Token::Type::IntLiteral);
    } else if (std::isdigit(ch) && ch != '0' && ch != '1') {
        handle<State::Error>(ch);
    } else if (!(ch == '0' || ch == '1')) {
        add_token(Token::Type::IntLiteral);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Octal>(char ch)
{
    if (std::isspace(ch)) {
        add_token(Token::Type::IntLiteral);
    } else if (ch == '8' || ch == '9') {
        handle<State::Error>(ch);
    } else if (!std::isdigit(ch)) {
        add_token(Token::Type::IntLiteral);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Hex>(char ch)
{
    if (std::isspace(ch)) {
        add_token(Token::Type::IntLiteral);
    } else if (!is_hex_digit(ch)) {
        add_token(Token::Type::IntLiteral);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Dot>(char ch)
{
    if (std::isdigit(ch)) {
        _curr_state = State::Floating;
    } else if (std::isspace(ch)) {
        add_token(Token::Type::FloatLiteral);
    } else {
        add_token(Token::Type::Operator);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Floating>(char ch)
{
     if (std::isspace(ch)) {
         add_token(Token::Type::FloatLiteral);
     } else if (ch == 'i') {
         add_token(Token::Type::ImagLiteral, true);
     } else if (!std::isdigit(ch)) {
         add_token(Token::Type::FloatLiteral);
         handle<State::Start>(ch);
     }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Gt>(char ch)
{
    if (ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (ch == '>') {
        _curr_state = State::Shift;
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Ls>(char ch)
{
    if (ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (ch == '<') {
        _curr_state = State::Shift;
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Shift>(char ch)
{
    if (ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}


template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Ampersand>(char ch)
{
    if (ch == '&' || ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Op>(char ch)
{
    if (ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Colon>(char ch)
{
     if (ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Plus>(char ch)
{
    if (ch == '+' || ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Minus>(char ch)
{
    if (ch == '-' || ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Or>(char ch)
{
    if (ch == '|' || ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Text>(char ch)
{
    if (!(std::isalnum(ch) || ch == '_')) {
        auto current_text = ltrimed(std::string_view{_curr_token.data(), _curr_token.size() - 1});
        if (is_keyword(current_text)) {
            add_token(Token::Type::Keyword);
        }
        else if (current_text == "true" || current_text == "false") {
            add_token(Token::Type::BoolLiteral);
        }
        else if (current_text == "nil") {
            add_token(Token::Type::Nil);
        }
        else {
            add_token(Token::Type::Identifier);
        }
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::Slash>(char ch)
{
    if (ch == '/') {
        _curr_state = State::LineComment;
    } else if (ch == '*') {
        _curr_state = State::BlockComment;
    } else if (ch == '=') {
        add_token(Token::Type::Operator, true);
    } else if (is_operator(ch)) {
        _curr_state = State::Error;
    } else {
        add_token(Token::Type::Operator);
        handle<State::Start>(ch);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::LineComment>(char ch)
{
    if (ch == '\n') {
        add_token(Token::Type::Comment);
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::BlockComment>(char ch)
{
    if (ch == '*') {
        _curr_state = State::ClosingBlockComment;
    }
}

template<>
void lab::Lexer::Impl::handle<lab::Lexer::Impl::State::ClosingBlockComment>(char ch)
{
    if (ch == '/') {
        add_token(Token::Type::Comment, true);
    } else {
        _curr_state = State::BlockComment;
    }
}

auto lab::Lexer::Impl::process(std::string_view input) -> std::vector<Token>
{
    _curr_state = State::Start;
    _tokens.clear();

    for (auto ch: input) {
        _curr_token.push_back(ch);
        switch (_curr_state) {
            case State::Start:                handle<State::Start>(ch); break;
            case State::Rune:                 handle<State::Rune>(ch); break;
            case State::RuneEscaping:         handle<State::RuneEscaping>(ch); break;
            case State::RuneProcessing:       handle<State::RuneProcessing>(ch); break;
            case State::StringProcessing:     handle<State::StringProcessing>(ch); break;
            case State::StringEscaping:       handle<State::StringEscaping>(ch); break;
            case State::Punctiation:          handle<State::Punctiation>(ch); break;
            case State::Decimal:                handle<State::Decimal>(ch); break;
            case State::Floating:             handle<State::Floating>(ch); break;
            case State::Op:                   handle<State::Op>(ch); break;
            case State::Plus:                 handle<State::Plus>(ch); break;
            case State::Minus:                handle<State::Minus>(ch); break;
            case State::Slash:                handle<State::Slash>(ch); break;
            case State::Ampersand:            handle<State::Ampersand>(ch); break;
            case State::Or:                   handle<State::Or>(ch); break;
            case State::Shift:                handle<State::Shift>(ch); break;
            case State::Ls:                   handle<State::Ls>(ch); break;
            case State::Gt:                   handle<State::Gt>(ch); break;
            case State::Dot:                  handle<State::Dot>(ch); break;
            case State::Colon:                handle<State::Colon>(ch); break;
            case State::Text:                 handle<State::Text>(ch); break;
            case State::LineComment:          handle<State::LineComment>(ch); break;
            case State::BlockComment:         handle<State::BlockComment>(ch); break;
            case State::ClosingBlockComment:  handle<State::ClosingBlockComment>(ch); break;
            case State::Error:                handle<State::Error>(ch); break;
            case State::Zero:                 handle<State::Zero>(ch); break;
            case State::Binary:               handle<State::Binary>(ch); break;
            case State::Hex:                  handle<State::Hex>(ch); break;
            case State::Octal:                handle<State::Octal>(ch); break;
        }
    }
    return std::move(_tokens);
}

void lab::Lexer::Impl::add_token(lab::Token::Type type, bool take_last)
{
    auto res_token = ltrimed(take_last
                             ? _curr_token
                             : std::string_view{_curr_token.data(), _curr_token.size() - 1});
    _tokens.push_back(Token{type, std::string{res_token.data(), res_token.size()}});

    if (take_last) {
        _curr_token.clear();
    }
    else {
        _curr_token.erase(_curr_token.begin(), _curr_token.end() - 1);
    }
    _curr_state = State::Start;
}

lab::Lexer::Lexer()
    : _impl{std::make_unique<Impl>()}
{}

auto lab::Lexer::process(std::string_view input) -> std::vector<Token>
{
    return _impl->process(input);
}

lab::Lexer::~Lexer() = default;

