const initEnvironment = require(`eosiac`);

const { sendTransaction, env } = initEnvironment(
  process.env.EOSIAC_ENV || `waxtest`,
  { verbose: true }
);

const accounts = Object.keys(env.accounts);

const CONTRACT_ACCOUNT = accounts[1];
const TOKEN_ACCOUNT = accounts[2];

async function action() {
  try {
    await sendTransaction([
      {
        account: `eosio.token`,
        name: `open`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          owner: CONTRACT_ACCOUNT,
          symbol: "8,WAX",
          ram_payer: CONTRACT_ACCOUNT,
        },
      },
      {
        account: TOKEN_ACCOUNT,
        name: `open`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          owner: CONTRACT_ACCOUNT,
          symbol: "9,WEOSDT",
          ram_payer: CONTRACT_ACCOUNT,
        },
      },

      {
        account: CONTRACT_ACCOUNT,
        name: `token`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          symcode: "WEOSDT",
          contract: TOKEN_ACCOUNT,
        },
      },
      {
        account: CONTRACT_ACCOUNT,
        name: `token`,
        authorization: [
          {
            actor: CONTRACT_ACCOUNT,
            permission: `active`,
          },
        ],
        data: {
          symcode: "WAX",
          contract: `eosio.token`,
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
