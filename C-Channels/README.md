# C-Channels

A small Go/Rust-style **MPSC (multi-producer, single-consumer) channel** implemented in C
on top of `pthread` mutexes and condition variables.

Multiple threads hold their own `t_sender` handle and push pointers into an unbounded
FIFO queue; one thread owns the single `t_reciver` and blocks on `reciv()` until a message
arrives. When the last sender is dropped, the channel closes and the receiver is woken up
with `CH_CLOSED` instead of hanging forever.

---

## Layout

```
channels.h          public API of the library
mpsc/cmpsc.h        internal MPSC types (channel, node, sender/receiver fwd decls)
mpsc/channel.c      channel lifecycle: create / close / node helpers
mpsc/send.c         sender side: create, clone, push, drop (refcount)
mpsc/recive.c       receiver side: create, pop (blocking), teardown
mpsc_pub_funcs.c    public wrappers: MPSCNew() / closeMPSC()
helpers.c/.h        ft_calloc()
test_main.c         demo: 5 producer threads -> 1 consumer
Makefile            builds libmpsc.a and the demo binary
```

## Build

```sh
make            # -> libmpsc.a
make test_app   # -> ./test_app (demo binary)
make clean      # remove .o
make fclean     # remove .o, libmpsc.a
make fun        # cross-compile the demo for x86_64 linux (needs x86_64-linux-gnu-gcc)
```

Flags: `cc -g -Wall -Wextra -Werror`.

Link it into your own program:

```sh
cc -Wall -Wextra -Werror main.c -L. -lmpsc -o app
```

---

## API

Everything you need is in `channels.h`.

### Types

```c
typedef struct s_mpscRes {
    t_reciver *reciver;
    t_sender  *sender;
} t_mpscRes;

typedef enum e_chanStatus { CH_OK, CH_ERR, CH_CLOSED } t_chanStatus;

typedef struct s_chanResult {
    t_chanStatus status;
    void        *data;
} t_chanResult;
```

`t_sender` and `t_reciver` are opaque — their definitions live in `mpsc/send.c` and
`mpsc/recive.c`, so callers can only pass them around by pointer.

### Functions

| Function | Description |
|---|---|
| `t_mpscRes *MPSCNew(void)` | Creates a channel plus its first sender and its only receiver. Returns `NULL` on allocation failure. |
| `t_sender *cloneSender(t_sender *origin)` | Makes another sender handle for the same channel and bumps the refcount. Give one clone to each producer thread. Declared in `mpsc/cmpsc.h`. |
| `void send_data(t_sender *sender, void *data)` | Appends `data` to the queue and signals a waiting receiver. The channel stores the **pointer only** — it never copies or owns the pointee. |
| `t_chanResult reciv(t_reciver *reciver)` | Blocks until a message is available or the channel is closed. `status == CH_OK` → `data` is valid; `status == CH_CLOSED` → queue drained and all senders gone. |
| `void dropSender(t_sender *sender)` | Releases one sender handle. When the refcount reaches 0 the channel is marked closed and every blocked receiver is woken. |
| `void closeMPSC(t_mpscRes *MPSC)` | Tears down the receiver, the channel (mutex, cond, leftover nodes) and the `t_mpscRes` box itself. |

### Ownership rules

* One `cloneSender()` per producer thread; each thread calls `dropSender()` exactly once
  when it is done.
* The original sender returned by `MPSCNew()` must also be dropped, otherwise the
  refcount never reaches 0 and the receiver blocks forever.
* Message payloads are **not** owned by the channel: if you send heap pointers, the
  consumer frees them.

---

## Example

```c
#include "channels.h"
#include "mpsc/cmpsc.h"
#include <pthread.h>
#include <stdio.h>

void *worker(void *arg)
{
    t_sender *sender = arg;
    static int  value = 42;

    send_data(sender, &value);
    dropSender(sender);
    return (NULL);
}

int main(void)
{
    t_mpscRes *mpsc = MPSCNew();
    pthread_t  th[4];

    for (int i = 0; i < 4; i++)
        pthread_create(&th[i], NULL, worker, cloneSender(mpsc->sender));
    dropSender(mpsc->sender);          /* drop the original handle */

    while (1) {
        t_chanResult res = reciv(mpsc->reciver);
        if (res.status == CH_CLOSED)
            break ;
        printf("got %d\n", *(int *)res.data);
    }
    for (int i = 0; i < 4; i++)
        pthread_join(th[i], NULL);
    closeMPSC(mpsc);
}
```

`test_main.c` contains a similar demo with randomized sleeps.

---

## How it works

* `t_channel` holds the mutex `mu`, the condition variable `state`, the head/tail of a
  singly linked queue, a sender refcount `RC`, and the `closed` flag.
* `pushToChan()` locks, links the node at the tail, `pthread_cond_signal()`s, unlocks.
* `pop()` locks and waits on `state` while the queue is empty **and** the channel is
  open; it returns `CH_OK` with the node's data or `CH_CLOSED` once no senders remain.
* `singleReciver` is a one-shot flag: a second `createReciver()` on the same channel
  fails, which is what makes it *single*-consumer.

## Notes / status

This is a learning project, so a few rough edges are known and intentional for now:

* The queue is unbounded — there is no backpressure (no bounded/rendezvous mode yet).
* `CH_ERR` is declared but never returned; `send_data()` has no way to report a failed
  allocation to the caller.
* A closed channel silently accepts further `send_data()` calls.
* `create_node()` / `add_node_back()` in `mpsc/channel.c` are leftovers not used by the
  send path.
