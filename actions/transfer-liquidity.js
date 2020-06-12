const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `waxtest`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const USDWAX_PRICE = 0.07;
const RECEIVER = accounts[0];
const CONTRACT_ACCOUNT = accounts[1];
const TOKEN_ACCOUNT = accounts[2];

async function action() {
  const eosdtQuantity = 10;
  const waxQuantity = eosdtQuantity / USDWAX_PRICE;
  try {
    await sendTransaction([
      {
        account: TOKEN_ACCOUNT,
        name: `transfer`,
        authorization: [
          {
            actor: RECEIVER,
            permission: `active`,
          },
        ],
        data: {
          from: RECEIVER,
          to: CONTRACT_ACCOUNT,
          quantity: `${eosdtQuantity.toFixed(9)} WEOSDT`,
          // memo == get_self() means add liquidity
          memo: `fund`,
        },
      },
      {
        account: `eosio.token`,
        name: `transfer`,
        authorization: [
          {
            actor: RECEIVER,
            permission: `active`,
          },
        ],
        data: {
          from: RECEIVER,
          to: CONTRACT_ACCOUNT,
          quantity: `${waxQuantity.toFixed(8)} WAX`,
          // memo == get_self() means add liquidity
          memo: `fund`,
        },
      },
    ]);
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
