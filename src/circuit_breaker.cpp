TABLE priceaverage {
  double price = 0.0;
  eosio::time_point_sec updated_at = eosio::time_point_sec(0);
};
typedef eosio::singleton<"priceaverage"_n, priceaverage> priceaverage_t;

void swapSx::check_price_within_feed() {
  swapSx::tokens_table _tokens(get_self(), get_self().value);
  swapSx::spotprices_table _spotprices(get_self(), get_self().value);
  swapSx::settings _settings(get_self(), get_self().value);
  if (!_spotprices.exists())
    return;

  auto spotprices = _spotprices.get();
  // settings should always exist => otherwise contract cannot transfer
  auto settings = _settings.get();
  if (!settings.pricefeed_contract)
    return;

  // spot prices
  for (const auto token : _tokens) {
    const symbol_code quote = token.sym.code();
    if (quote == spotprices.base)
      continue;

    priceaverage_t priceavg_table(settings.pricefeed_contract, quote.raw());
    check(priceavg_table.exists(),
          "priceavg does not exist: " + quote.to_string());
    if (!priceavg_table.exists())
      continue;
    priceaverage priceavg = priceavg_table.get();

    double spot_price = spotprices.quotes[quote];
    double oracle_price = priceavg.price;

    check(fabs(spot_price - oracle_price) / oracle_price <=
              settings.max_price_divergence,
          "trade would move price outside of reasonable range (spot: " +
              std::to_string(spot_price) +
              ", oracle: " + std::to_string(oracle_price) + ")");
  }
}

void swapSx::checkbalanced(const symbol_code &base, const symbol_code &quote,
                           double spotprice, double threshold) {
  double current_spotprice = get_spot_price(base, quote);

  check(threshold <= 0.1, "threshold must be between 0 and 0.1 (10%)");
  check(
      fabs(spotprice - current_spotprice) / current_spotprice <= threshold,
      "spot price differed too much - expected: " + std::to_string(spotprice) +
          ", actual: " + std::to_string(current_spotprice) + ")");
}