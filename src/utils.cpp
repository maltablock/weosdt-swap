#include <math.h>

vector<string> split(const string& str, const string& delim) {
    vector<string> tokens;
    size_t start = 0, found = 0;
    do {
        found = str.find(delim, start);
        if (found == string::npos) found = str.length();
        string token = str.substr(start, found-start);
        tokens.push_back(token);
        start = found + delim.length();
    } while (found < str.length() && start < str.length());

    return tokens;
}

swapSx::parsed_memo swapSx::parse_memo( const string memo )
{
    check( memo.length() > 0 && memo.length() <= 50 , "memo is too long" );
    vector<string> parts = split(memo, ",");

    check(parts.size() <= 2, "memo has too many parts (only one `,` expected)");

    // must be all uppercase alpha characters
    for (char const c: parts[0] ) {
        check( isalpha(c) && isupper(c), "memo contains invalid symbol code" );
    }
    const symbol_code symcode = symbol_code{ parts[0] };
    check( symcode.is_valid(), "memo contains invalid symbol code"  );

    int64_t expected_return = 0;
    if(parts.size() == 2) {
        for (char const c: parts[1] ) {
            check( isdigit(c), "memo contains invalid expected quantity" );
        }
        expected_return = stoull(parts[1]);
        check(expected_return >= 0, "memo contains invalid expected quantity (not non-negative)");
    }

    return {symcode, expected_return};
}

void swapSx::self_transfer( const name to, const asset quantity, const string memo )
{
    token::transfer_action transfer( get_contract( get_self(), quantity.symbol.code() ), { get_self(), "active"_n });
    transfer.send( get_self(), to, quantity, memo );
}
