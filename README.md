# SX `Flashloans` ⚡️ - EOSIO Smart Contract

> Borrow any amount of liquidity instantly for $0 fees & $0 collateral required.

## Table of Content

- [`concept`](#concept)
- [`examples`](#examples)
- [`borrow`](#action-borrow)
- [`checkbalance`](#action-checkbalance)
- [`callback`](#action-callback)
- [`withdraw`](#action-withdraw)

## Concept

1. account calls `borrow` action from `flash.sx` with a desired `quantity`.
  - contract gets **initial** balance of asset.
  - contract sends quantity `to` account.
2. account recieves notifications via `on_notify` and/or `callback` of incoming transfer.
  - account is free to use received quantity for any purposes.
  - account returns loan back to contract.
3. contract gets **final** balance of asset
  - contract throws an error if **initial** balance is lower than **final** balance.

## Examples

- [**basic.cpp**](/examples/basic.sx.cpp) ([example transaction](https://eosq.app/tx/10f95b6c3927a2d506bad403499a415cb471cb515d52aac8914d5e1b2f3c957a))
- [**callback.cpp**](/examples/callback.sx.cpp) ([example transaction](https://eosq.app/tx/e93c55af119abb8f90c4adf7a00a05fca8072c41483615d0df7e33a6f53d8e3f))

## ACTION `borrow`

Account requests to borrow quantity

- **authority**: `any`

### params

- `{name} to` - receiver of flash loan
- `{name} contract` - token contract account
- `{asset} quantity` - flash loan request amount
- `{string} memo` - used for outgoing transfer
- `{set<name>} notifiers` - notify accounts via `callback` action after transfer has been sent

### Example 1

```c++
const asset quantity = asset{10000, symbol{"EOS", 4}};
flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
borrow.send( get_self(), "eosio.token"_n, quantity, "my memo", set<name>() );
```

### Example 2

```bash
$ cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "my memo", []]' -p myaccount
```

## ACTION `checkbalance`

Throws error if account does not have equal or above desired asset quantity

- **authority**: `any`

### params

- `{name} account` - account to check if minimum balance is available
- `{name} contract` - token contract account
- `{asset} quantity` - minimum required balance amount

### returns

- `{asset}` - current balance of account

### Example 1

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const asset quantity = asset{10000, symbol{"EOS", 4}};

flash::checkbalance_action checkbalance( "flash.sx"_n, { account, "active"_n });
checkbalance.send( account, contract, quantity );
```

### Example 2

```bash
$ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "1.0000 EOS"] -p myaccount
```

## ACTION `callback`

Notifies recipient account via `callback` action after transfer has been sent from `borrow` action

- **authority**: `get_self()`

### params

- `{name} recipient` - callback recipient
- `{name} to` - receiver of flash loan
- `{name} contract` - token contract account
- `{asset} quantity` - flash loan request amount
- `{string} memo` - used for outgoing transfer

### Example

```c++
[[eosio::on_notify("flash.sx::callback")]]
void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
{
	token::transfer_action transfer( contract, { get_self(), "active"_n });
	transfer.send( get_self(), "flash.sx"_n, quantity, memo );
}
```

## ACTION `withdraw`

Account can withdraw available balance from contract

- **authority**: `account`

### params

- `{name} account` - account to withdraw from
- `{name} contract` - token contract account
- `{asset} quantity` - withdraw amount

### Example

```bash
cleos push action flash.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount
```

## Build

```bash
$ eosio-cpp flash.sx.cpp -o flash.sx.wasm -I external
```
