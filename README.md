# WAX <> WEOSDT Swap Pool with circuit breaker

Based on [SX Swap Pools](https://github.com/stableex/sx.swap).

> Automated Liquidity Pool with circuit breaker

Many new pools face issues of liquidity which can lead to drastic price differences compared to other exchanges.
This contract protects consumers by stopping trades for big orders that would lead to a price difference of a certain % compared to the asset's real traded price.
The prices of the underlying assets are taken from medians of 24-hour spot prices provided by an oracle.


## Quickstart

### Setup

Run the [`setparams` action](./actions/setparams.js) to set up the swap contract:

```js
params: {
    fee: 30,
    amplifier: 1,
    pricefeed_contract: `globaloracle`,
},
```

Open account balances for all tokens for the contract.
Then run the [`token` actions](./actions/token.js) to set up the tokens:

```js
 {
    account: CONTRACT_ACCOUNT,
    name: `token`,
    data: {
        symcode: "WEOSDT",
        contract: TOKEN_ACCOUNT,
    },
},
{
    account: CONTRACT_ACCOUNT,
    name: `token`,
    data: {
        symcode: "WAX",
        contract: `eosio.token`,
    },
},
```

### Set up liquidity

To deposit initial liquidity send a transfer with the `fund` memo.

