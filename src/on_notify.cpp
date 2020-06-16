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
    check(memo == "convert" || memo == "fee" || memo == "fund",
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
    // add/remove liquidity depth
    set_balance(quantity.symbol.code());
    update_spot_prices(base_symbol.code());
    if (from == get_self())
      sub_depth(quantity);
    else if (to == get_self())
      add_depth(quantity);
    return;
  }

  // check if contract maintenance is ongoing
  swapSx::settings _settings(get_self(), get_self().value);
  check(_settings.exists(), "contract is currently disabled for maintenance");
  auto settings = _settings.get();
  check(settings.fee_account.value > 0, "fee_account not set in settings");

  // update balances (post convert transfer, triggered by `on_notify`)
  // just mirrors the token contracts accounts row
  // only update for outgoing tx (from == _self)
  if (from == get_self() && (memo == "convert" || memo == "fee")) {
    set_balance(quantity.symbol.code());
    update_spot_prices(base_symbol.code());
    check_price_within_feed();
  }

  // check incoming converts now
  if (to != get_self())
    return;

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
  self_transfer(settings.fee_account, fee, "fee");

  // post transfer
  update_volume(vector<asset>{quantity, rate}, fee);
  set_balance(quantity.symbol.code());
}