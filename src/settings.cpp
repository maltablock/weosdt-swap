[[eosio::action]]
void swapSx::setparams( const optional<swapSx::params> params )
{
    require_auth( get_self() );
    swapSx::settings _settings( get_self(), get_self().value );

    // clear table if params is `null`
    if ( !params ) {
        _settings.remove();
        return;
    }

    check( params->fee <= 50, "fee cannot be greater than 0.5%");
    check( params->fee >= 0, "fee must be positive");
    check( params->max_price_divergence >= 0, "max_price_divergence must be positive");

    _settings.set( *params, get_self() );
}

[[eosio::action]]
void swapSx::token( const symbol_code symcode, const optional<name> contract )
{
    require_auth( get_self() );

    swapSx::tokens_table _tokens( get_self(), get_self().value );
    auto itr = _tokens.find( symcode.raw() );

    // delete if contract is null
    if ( itr != _tokens.end() && contract->value == 0 ) {
        _tokens.erase( itr );
        return;
    }

    check( itr == _tokens.end(), "token already exists");

    // validate symcode & contract
    check( contract->value, "contract is required");
    const asset supply = token::get_supply( *contract, symcode );
    const asset balance = token::get_balance( *contract, get_self(), symcode );
    if ( contract ) check( supply.amount > 0, "invalid supply");

    _tokens.emplace( get_self(), [&]( auto& row ){
        row.sym = supply.symbol;
        row.contract = *contract;
        row.balance = balance;
        row.depth = balance;
    });
}
