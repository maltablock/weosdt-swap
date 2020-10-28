const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `waxtest`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const RECEIVER = accounts[0];
const CONTRACT_ACCOUNT = accounts[1];
const TOKEN_ACCOUNT = accounts[2];

async function action() {
  try {
    await sendTransaction([
      {
      account: CONTRACT_ACCOUNT,
      name: `withdraw`,
      authorization: [
        {
          actor: RECEIVER,
          permission: `active`,
        },
      ],
      data: {
        to: RECEIVER,
        token0: {
          quantity: `0.123456789 WEOSDT`,
          contract: TOKEN_ACCOUNT,
        },
        token1: {
          quantity: `0.0000000 WAX`,
          contract: `eosio.token`,
        }
      },
    },
      {
      account: CONTRACT_ACCOUNT,
      name: `checkbalanced`,
      authorization: [
        {
          actor: RECEIVER,
          permission: `active`,
        },
      ],
      data: {
        spotprice: `0.084`,
        base: `WEOSDT`,
        quote: `WAX`,
        threshold: 0.01,
      },
    },
  
  ]);

    process.exit(0);
  } catch (error) {
    console.error(error)
    // ignore
    process.exit(1);
  }
}

action();
