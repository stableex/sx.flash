<h1 class="contract">borrow</h1>

---
spec_version: "0.2.0"
title: borrow
summary: 'Borrow from flashloan provider'
icon: https://avatars1.githubusercontent.com/u/60660770#d6a1df4bbf2942f23c3a4485eb9942cb37c5348945e84be8c53e2ef9254ed8da
---

Flashloan borrower {{to}} agrees to repay {{contract}}@{{quantity}} within the same transaction timeframe.

The borrow transfer action will be using {{memo}} memo and notify {{notifier}} account.

<h1 class="contract">checkbalance</h1>

---
spec_version: "0.2.0"
title: checkbalance
summary: 'Check account balance if lower than quantity'
icon: https://avatars1.githubusercontent.com/u/60660770#d6a1df4bbf2942f23c3a4485eb9942cb37c5348945e84be8c53e2ef9254ed8da
---

Throws error if {{account}} as a balance of lower than {{contract}}@{{quantity}}.

<h1 class="contract">callback</h1>

---
spec_version: "0.2.0"
title: callback
summary: 'Notifies recipient from flashloan'
icon: https://avatars1.githubusercontent.com/u/60660770#d6a1df4bbf2942f23c3a4485eb9942cb37c5348945e84be8c53e2ef9254ed8da
---

Allows {{recipient}} recipient to receive notification once {{quantity}} transfer is deposited from flashloan.

<h1 class="contract">savebalance</h1>

---
spec_version: "0.2.0"
title: savebalance
summary: 'Save balance of account'
icon: https://avatars1.githubusercontent.com/u/60660770#d6a1df4bbf2942f23c3a4485eb9942cb37c5348945e84be8c53e2ef9254ed8da
---

Save balance of {{account}} of {{contract}}::{{symcode}}.
