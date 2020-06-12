#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>

#include <math.h>
#include <string>

using namespace eosio;
using namespace std;

constexpr name fee_account = name("cmichelonwax");
constexpr symbol base_symbol = symbol("WEOSDT", 9);

class [[eosio::contract("swap.sx")]] swapSx : public contract {
public:
    using contract::contract;

    /**
     * ## TABLE `settings`
     *
     * - `{int64_t} fee` - trading fee (pips 1/100 of 1%)
     * - `{int64_t} amplifier` - liquidity pool amplifier
     *
     * ### example
     *
     * ```json
     * {
     *   "fee": 20,
     *   "amplifier": 100
     * }
     * ```
     */
    struct [[eosio::table("settings")]] params {
        int64_t             fee;
        int64_t             amplifier;
        name                pricefeed_contract;
    };
    typedef eosio::singleton< "settings"_n, params > settings;


    /**
     * ## TABLE `tokens`
     *
     * - `{symbol} sym` -  token symbol
     * - `{contract} contract` - token contract account name
     * - `{asset} balance` - current balance
     * - `{asset} depth` - liquidity depth
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,USDT",
     *     "contract": "tethertether",
     *     "balance": "10000.0000 USDT",
     *     "depth": "10000.0000 USDT"
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol                      sym;
        name                        contract;
        asset                       balance;
        asset                       depth;

        uint64_t primary_key() const { return sym.code().raw(); }
    };
    typedef eosio::multi_index< "tokens"_n, tokens_row > tokens_table;

    /**
     * ## TABLE `volume`
     *
     * - `{time_point_sec} timestamp` - daily periods (86400 seconds)
     * - `{map<symbol_code, asset>} fees` - total fees collected
     * - `{map<symbol_code, asset>} volume` - total trading volume of assets
     *
     * ### example
     *
     * ```json
     * {
     *   "timestamp": "2020-06-03T00:00:00",
     *   "volume": [
     *     {"key": "EOSDT", "value": "25.000000000 EOSDT"},
     *     {"key": "USDT", "value": "100.0000 USDT"}
     *   ],
     *   "fees": [
     *     {"key": "EOSDT", "value": "0.100000000 EOSDT"},
     *     {"key": "USDT", "value": "0.4000 USDT"}
     *   ]
     * }
     * ```
     */
    struct [[eosio::table("volume")]] volume_params {
        time_point_sec             timestamp;
        map<symbol_code, asset>    volume;
        map<symbol_code, asset>    fees;
    };
    typedef eosio::singleton< "volume"_n, volume_params > volume_table;

    /**
     * ## TABLE `spotprices`
     *
     * - `{time_point_sec} last_modified` - last modified timestamp
     * - `{symbol_code} fees` - base symbol code
     * - `{map<symbol_code, double>} quotes` - quotes prices calculated relative to base
     *
     * ### example
     *
     * ```json
     * {
     *   "last_modified": "2020-06-03T12:30:00",
     *   "base": "EOS",
     *   "quotes": [
     *     {"key": "EOSDT", "value": 0.3636},
     *     {"key": "USDT", "value": 0.3636}
     *   ]
     * }
     * ```
     */
    struct [[eosio::table("spotprices")]] spotprices_params {
        time_point_sec              last_modified;
        symbol_code                 base;
        map<symbol_code, double>    quotes;
    };
    typedef eosio::singleton< "spotprices"_n, spotprices_params > spotprices_table;

    /**
     * ## ACTION `setparams`
     *
     * Update contract parameters
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{settings_params} settings` - settings parameters
     *
     * ### example
     *
     * ```bash
     * cleos push action swap.sx setparams '[{"fee": 10, "amplifier": 20}]' -p stable.sx
     * ```
     */
    [[eosio::action]]
    void setparams( const optional<swapSx::params> params );

    /**
     * ## ACTION `token`
     *
     * Add/removes token
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{symbol_code} symcode` - token symbol code
     * - `{name} [contract=null]` - token contract account name (if `null` delete symbol)
     *
     * ### example
     *
     * ```bash
     * cleos push action swap.sx token '["USDT", "tethertether"]' -p swap.sx
     * ```
     */
    [[eosio::action]]
    void token( const symbol_code symcode, const optional<name> contract );

    /**
     * Notify contract when any token transfer notifiers relay contract
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name       from,
                      const name       to,
                      const asset      quantity,
                      const string     memo );

    /**
     * ## STATIC `get_rate`
     *
     * Get calculated rate (includes fee)
     *
     * ### params
     *
     * - `{name} contract` - contract account
     * - `{asset} quantity` - input quantity
     * - `{symbol_code} symcode` - out symbol code
     *
     * ### example
     *
     * ```c++
     * const asset quantity = asset{10000, symbol{"USDT", 4}};
     * const symbol_code symcode = symbol_code{"EOSDT"};
     * const asset rate = get_rate( "stable.sx"_n, quantity, symcode );
     * //=> "1.002990000 EOSDT"
     * ```
     */
    static asset get_rate( const name contract, const asset quantity, const symbol_code symcode )
    {
        const asset fee = get_fee( contract, quantity );
        return get_price( contract, quantity - fee, symcode );
    }

    // convert
    static vector<double> get_uppers( const name contract, const symbol_code base, const symbol_code quote );
    static asset get_price( const name contract, const asset quantity, const symbol_code symcode );
    static asset get_fee( const name contract, const asset quantity );

    // utils
    static double asset_to_double( const asset quantity );
    static asset double_to_asset( const double amount, const symbol sym );

    // bancor
    static double get_bancor_output( const double base_reserve, const double quote_reserve, const double quantity );
    static double get_bancor_input( const double quote_reserve, const double base_reserve, const double out );

    // tokens
    static name get_contract( const name contract, const symbol_code symcode );
    static symbol get_symbol( const name contract, const symbol_code symcode );
    static extended_symbol get_extended_symbol( const name contract, const symbol_code symcode );

    // action wrappers
    using setparams_action = eosio::action_wrapper<"setparams"_n, &swapSx::setparams>;
    using token_action = eosio::action_wrapper<"token"_n, &swapSx::token>;

private:
    // utils
    symbol_code parse_memo_symcode( const string memo );
    void self_transfer( const name to, const asset quantity, const string memo );

    // tokens
    void set_balance( const symbol_code symcode );
    void add_depth( const asset quantity );
    void sub_depth( const asset quantity );

    void check_is_active( const symbol_code symcode, const name contract );
    void check_max_ratio( const symbol_code symcode );
    void check_min_ratio( const asset out );

    void check_price_within_feed();

    double get_ratio( const symbol_code symcode );
    asset get_balance( const symbol_code symcode );
    asset get_depth( const symbol_code symcode );

    // volume
    void update_volume( const vector<asset> volumes, const asset fee );

    // spot prices
    void update_spot_prices( const symbol_code base );
    double get_spot_price( const symbol_code base, const symbol_code quote );
};
