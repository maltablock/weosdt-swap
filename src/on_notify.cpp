#ifdef __TEST__
static const std::vector<name> FUNDERS_WHITELIST{name("cmichelonwax")};
#else
static const std::vector<name> FUNDERS_WHITELIST{name("waxmeetup111"),
                                                 name("chongqingbnk")};
#endif

[[eosio::on_notify("*::transfer")]] void
swapSx::on_transfer(const name from, const name to, const asset quantity,
                    const string memo) {
  // authenticate incoming `from` account
  require_auth(from);

  const symbol_code in_symcode = quantity.symbol.code();
  check_token_exists(in_symcode, get_first_receiver());
  check(from == get_self() || to == get_self(),
        "not involved in this transfer");

  // ignore transfers
  const set<name> ignore = set<name>{
      // EOSIO system accounts
      "eosio.stake"_n, "eosio.names"_n, "eosio.ram"_n, "eosio.rex"_n, "eosio"_n,
  };
  if (ignore.find(from) != ignore.end())
    return;

  // prevent invalid outgoing transfers
  if (from == get_self())
    check(memo == "convert" || memo == "fee" || memo == "fund",
          "invalid outgoing memo");

  // handle liquidity providing transfers
  if (memo == "fund") {

    // add/remove liquidity depth + balances
    if (from == get_self()) {
      check(std::find(FUNDERS_WHITELIST.begin(), FUNDERS_WHITELIST.end(), to) !=
                FUNDERS_WHITELIST.end(),
            "withdrawals to this account are not allowed");
      sub_depth(quantity);
      sub_balance(quantity);
    } else if (to == get_self()) {
      check(std::find(FUNDERS_WHITELIST.begin(), FUNDERS_WHITELIST.end(),
                      from) != FUNDERS_WHITELIST.end(),
            "deposits from this account are not allowed");
      add_depth(quantity);
      add_balance(quantity);
    }

    update_spot_prices();
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
  const auto memo_info = parse_memo(memo);
  check_token_exists(memo_info.out_symcode, name{});
  check(in_symcode != memo_info.out_symcode,
        in_symcode.to_string() + " symbol code cannot be the same as quantity");

  // calculate rates
  const asset fee = swapSx::get_fee(get_self(), quantity);
  const asset rate =
      swapSx::get_rate(get_self(), quantity, memo_info.out_symcode);

  // validate output
  check(rate.amount > 0, "quantity would be zero");

  // send transfers
  self_transfer(from, rate, "convert");
  if (fee.amount)
    self_transfer(settings.fee_account, fee, "fee");

  update_volume(vector<asset>{quantity, rate}, fee);

  // update balances `on_notify` inline transaction
  // prevents re-entry exploit
  add_balance(quantity - fee);
  sub_balance(rate);
  update_spot_prices();

  check(rate.amount > memo_info.expected_return,
        "converted amount was lower than desired (slippage?) - converted to " +
            rate.to_string());
  check_price_within_feed();

  // trade log
  const double trade_price = asset_to_double(rate) / asset_to_double(quantity);
  const double spot_price =
      get_spot_price(base_symbol.code(), rate.symbol.code());
  const double value = spot_price * asset_to_double(rate);
  const double base_quote_spot_price = get_spot_price(
      base_symbol.code(), rate.symbol.code() == base_symbol.code()
                              ? quantity.symbol.code()
                              : rate.symbol.code());
  swapSx::log_action log(get_self(), {get_self(), "active"_n});
  log.send(from, quantity, rate, fee, trade_price, base_quote_spot_price,
           value);

  // enforce 1.00 trades (prevents spam)
  check(value >= 1.0, "minimum trade value must exceed 1.00 " +
                          base_symbol.code().to_string());
}

void swapSx::withdraw(name to, const extended_asset &token0,
                      const extended_asset &token1) {
  require_auth(to);

  check(std::find(FUNDERS_WHITELIST.begin(), FUNDERS_WHITELIST.end(), to) !=
            FUNDERS_WHITELIST.end(),
        "withdrawals to this account are not allowed");

  check(token0.get_extended_symbol() != token1.get_extended_symbol(),
        "tokens must be different");
  check_token_exists(token0.get_extended_symbol().get_symbol().code(),
                     token0.get_extended_symbol().get_contract());
  check_token_exists(token1.get_extended_symbol().get_symbol().code(),
                     token1.get_extended_symbol().get_contract());

  // fund memo is important to update balances in on_receive
  if (token0.quantity.amount > 0)
    self_transfer(to, token0.quantity, "fund");
  if (token1.quantity.amount > 0)
    self_transfer(to, token1.quantity, "fund");
}