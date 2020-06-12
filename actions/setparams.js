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
    await sendTransaction({
      account: CONTRACT_ACCOUNT,
      name: `setparams`,
      authorization: [
        {
          actor: CONTRACT_ACCOUNT,
          permission: `active`,
        },
      ],
      data: {
        params: {
          fee: 30,
          amplifier: 1,
        },
      },
    });
    process.exit(0);
  } catch (error) {
    // ignore
    process.exit(1);
  }
}

action();
