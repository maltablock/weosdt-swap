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
    // await sendTransaction({
    //   account: `eosio.token`,
    //   name: `transfer`,
    //   authorization: [
    //     {
    //       actor: RECEIVER,
    //       permission: `active`,
    //     },
    //   ],
    //   data: {
    //     from: RECEIVER,
    //     to: CONTRACT_ACCOUNT,
    //     quantity: `40.00000000 WAX`,
    //     memo: `WEOSDT`,
    //   },
    // });

    await sendTransaction({
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
        quantity: `1.000000000 WEOSDT`,
        memo: `WAX`,
      },
    });
    process.exit(0);
  } catch (error) {
    console.error(error)
    // ignore
    process.exit(1);
  }
}

action();
