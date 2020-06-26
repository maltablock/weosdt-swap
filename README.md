# WAX <> WEOSDT Swap Pool with circuit breaker

Based on [SX Amplified Liquidity Pools](https://github.com/stableex/sx.swap).

> Automated Liquidity Pool with circuit breaker


Many new pools face issues of liquidity which can lead to drastic price differences compared to other exchanges.
This contract protects consumers by stopping trades for big orders that would lead to a price difference of a certain % compared to the asset's real traded price.
The prices of the underlying assets are taken from medians of 24-hour spot prices provided by an oracle.


### Liquidity Pools on WAX


| **Pool**       | **Tokens**      |
|----------------|-----------------|
| [`weosdtwaxswp`](https://wax.bloks.io/account/weosdtwaxswp?loadContract=true&tab=Tables&account=weosdtwaxswp&scope=weosdtwaxswp&limit=100)      | WAX, [WEOSDT](https://wax.bloks.io/account/weosdttokens?loadContract=true&tab=Tables&account=weosdttokens&scope=weosdttokens&limit=100)


To convert between tokens simply send a transfer of the token you want to convert from to the liquidity pool with the memo of the token you want to convert to.

[Example transaction](https://greymass.github.io/eosio-uri-builder/gmPgYmhY3mTyvGXlIwYGhnBdm7NnGRkZIIAJSqvABAIs37WBVU4xLQfxOcNd_YNdQhiYwx0jmBkYAA) for converting WEOSDT to WAX:

```json
{
    "account": "weosdttokens",
    "name": "transfer",
    "data": {
        "from":"Your Account",
        "to":"weosdtwaxswp",
        "quantity":"2.000000000 WEOSDT",
        "memo":"WAX"
    }
}
```


[Example transaction](https://greymass.github.io/eosio-uri-builder/gmPgYmBY1mTC_MoglIGBIVzX5uxZRkYGCGCC0uowgQDLd23PW1Y-Yjg1yxrE5wh3jADRbOGu_sEuIUBlAA) for converting WAX to WEOSDT:

## Quickstart

### Setup Tokens


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
}
```


### Set up liquidity

To deposit initial liquidity send a transfer with the `fund` memo.

```js
 {
    account: `eosio.token`,
    name: `transfer`,
    data: {
        from: ACCOUNT,
        to: CONTRACT_ACCOUNT,
        quantity: `10.00000000 WAX`,
        memo: `fund`,
    },
}
```

### Enable trading

Run the [`setparams` action](./actions/setparams.js) to set up the swap contract and enable conversions:

```js
params: {
    fee: 30,
    fee_account: `waxmeetup111`,
    pricefeed_contract: `globaloracle`,
    max_price_divergence: 0.25,
}
```

