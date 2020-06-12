const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `waxtest`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const USDWAX_PRICE = 0.07
const RECEIVER = accounts[0];
const CONTRACT_ACCOUNT = accounts[1];
const TOKEN_ACCOUNT = accounts[2];

async function action() {
  const eosdtQuantity = 100
  const waxQuantity = eosdtQuantity / USDWAX_PRICE
  try {
    await sendTransaction([
      {
        account: TOKEN_ACCOUNT,
        name: `create`,
        authorization: [
          {
            actor: TOKEN_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          issuer: TOKEN_ACCOUNT,
          // https://bloks.io/account/eosdtsttoken?loadContract=true&tab=Tables&table=stat&account=eosdtsttoken&scope=EOSDT&limit=100
          maximum_supply: `170000000.000000000 WEOSDT`,
        },
      },
      {
        account: TOKEN_ACCOUNT,
        name: `issue`,
        authorization: [
          {
            actor: TOKEN_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          to: TOKEN_ACCOUNT,
          quantity: `1000000.000000000 WEOSDT`,
          memo: `issue some of it`,
        },
      },
      {
        account: TOKEN_ACCOUNT,
        name: `transfer`,
        authorization: [
          {
            actor: TOKEN_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          from: TOKEN_ACCOUNT,
          to: RECEIVER,
          quantity: `1000000.000000000 WEOSDT`,
          memo: `transfer all of it`,
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
