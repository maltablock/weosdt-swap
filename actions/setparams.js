const initEnvironment = require(`eosiac`);

const envName = process.env.EOSIAC_ENV || `waxtest`
const { sendTransaction, env } = initEnvironment(
  envName,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[1];
const TOKEN_ACCOUNT = accounts[2];
const FEES_ACCOUNT = envName === `wax` ? `waxmeetup111` : `cmichelonwax`

async function action() {
  try {
    await sendTransaction({
      account: CONTRACT_ACCOUNT,
      name: `setparams`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `admin`,
        },
      ],
      data: {
        params: {
          fee: 30,
          fee_account: FEES_ACCOUNT,
          pricefeed_contract: `globaloracle`,
          max_price_divergence: 0.25,
        },
        // params: undefined
      },
    });
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
