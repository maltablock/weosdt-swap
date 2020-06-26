static const std::vector<name> FUNDERS_WHITELIST{name("waxmeetup111")};

[[eosio::on_notify("*::transfer")]] void
swapSx::on_transfer(const name from, const name to, const asset quantity,
                    const string memo) {
  // authenticate incoming `from` account
  require_auth(from);

  const symbol_code in_symcode = quantity.symbol.code();
  check_token_exists(in_symcode, get_first_receiver());
  check(from == get_self() || to == get_self(),
        "not involved in this transfer");

  // prevent invalid transfers
  if (from == get_self())
    check(memo == "convert" || memo == "fee",
          "invalid outgoing memo");

  // ignore transfers
  const set<name> ignore = set<name>{
      // EOSIO system accounts
      "eosio.stake"_n, "eosio.names"_n, "eosio.ram"_n, "eosio.rex"_n, "eosio"_n,
  };
  if (ignore.find(from) != ignore.end())
    return;

  // handle liquidity providing transfers
  if (memo == "fund") {
    check(std::find(FUNDERS_WHITELIST.begin(), FUNDERS_WHITELIST.end(), from) != FUNDERS_WHITELIST.end(), "funds from this account are not allowed");
    // add/remove liquidity depth
    set_balance(quantity.symbol.code());
    update_spot_prices();
    if (from == get_self())
      sub_depth(quantity);
    else if (to == get_self())
      add_depth(quantity);
    return;
  }

  // check incoming converts now
  if (to != get_self())
    return;

  // check if contract maintenance is ongoing
  swapSx::settings _settings(get_self(), get_self().value);
  check(_settings.exists(), "contract is currently disabled for maintenance");
  auto settings = _settings.get();
  check(settings.fee_account.value > 0, "fee_account not set in settings");

  // validate input
  const symbol_code out_symcode = parse_memo_symcode(memo);
  check_token_exists(out_symcode, name{});
  check(in_symcode != out_symcode,
        in_symcode.to_string() + " symbol code cannot be the same as quantity");

  // calculate rates
  const asset fee = swapSx::get_fee(get_self(), quantity);
  const asset rate = swapSx::get_rate(get_self(), quantity, out_symcode);

  // validate output
  check(rate.amount > 0, "quantity would be zero");

  // send transfers
  self_transfer(from, rate, "convert");
  if ( fee.amount ) self_transfer(settings.fee_account, fee, "fee");

  update_volume(vector<asset>{quantity, rate}, fee);
  
  // update balances `on_notify` inline transaction
  // prevents re-entry exploit
  add_balance( quantity - fee );
  sub_balance( rate );
  update_spot_prices();

  check_price_within_feed();

  // trade log
  const double trade_price = asset_to_double( rate ) / asset_to_double( quantity );
  const double spot_price = get_spot_price( base_symbol.code(), rate.symbol.code() );
  const double value = spot_price * asset_to_double( rate );
  const double base_quote_spot_price = get_spot_price( base_symbol.code(), rate.symbol.code() == base_symbol.code() ? quantity.symbol.code() : rate.symbol.code());
  swapSx::log_action log( get_self(), { get_self(), "active"_n });
  log.send( from, quantity, rate, fee, trade_price, base_quote_spot_price, value );

  // enforce 1.00 trades (prevents spam)
  check( value >= 1.0, "minimum trade value must exceed 1.00 " + base_symbol.code().to_string() );
}