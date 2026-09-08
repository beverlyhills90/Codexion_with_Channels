
# Codexion

## Description

Codexion is a concurrency simulation. A number of coders sit around a shared
Quantum Compiler. Each coder cycles endlessly through three phases — compiling,
debugging, refactoring — and compiling requires holding **two USB dongles at the
same time**, the one on their left and the one on their right. There are exactly
as many dongles as coders, so neighbours compete for the same hardware.

A coder who does not *start* a new compilation within `time_to_burnout`
milliseconds of the start of their previous one burns out, and the simulation
stops. The simulation also stops, successfully, once every coder has compiled at
least `number_of_compiles_required` times.

The goal of the exercise is the synchronisation, not the arithmetic: sharing a
scarce resource between threads without deadlocking, without starving anyone,
detecting a missed deadline within 10 ms of it happening, and keeping the log
readable while several threads produce it concurrently.

### Overview of the design

Every coder is a POSIX thread. A separate monitor thread owns the decision to
stop the simulation. Coders never inspect each other's state and never write to
the log directly — they **send messages** to the monitor over a small
multi-producer / single-consumer channel written for this project
(`C-Channels/`). The monitor is the only consumer and the only thread that
prints.

```
 coder 1 ─┐
 coder 2 ─┼─► MPSC channel ─► monitor ─► stdout
 coder N ─┘                     │
                                └─► deadlines[], compiles[]  (private, no mutex)

 dongle i : mutex + condvar + 2-slot ordered request queue
```

That choice removes most of the shared mutable state: the deadline table and the
compile counters belong to the monitor alone, each coder's own timestamp belongs
to that coder alone, and the only genuinely shared objects left are the dongles
and the run/stop flag.

## Instructions

### Build

```sh
make            # builds ./codexion
make clean      # removes object files
make fclean     # removes object files and the binary
make re         # fclean + all
```

Compiles with `cc -Wall -Wextra -Werror -pthread`. No external dependencies.

### Run

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| argument | meaning |
|---|---|
| `number_of_coders` | number of coders, and of dongles |
| `time_to_burnout` | ms since the start of the last compilation before a coder burns out |
| `time_to_compile` | ms spent compiling, holding both dongles |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | simulation stops when every coder reached this count |
| `dongle_cooldown` | ms a released dongle stays unavailable |
| `scheduler` | `fifo` or `edf` |

All arguments are mandatory and must be non-negative integers; `scheduler` must
be exactly `fifo` or `edf`.

### Usage examples

```sh
# nobody can survive: one cycle (600 ms) is longer than the burnout window
./codexion 4 300 200 200 200 5 0 fifo
300 1 burned out

# a single coder can never hold two dongles, so they always burn out
./codexion 1 800 200 200 200 5 0 fifo
0 1 has taken a dongle
800 1 burned out

# comfortable parameters: the run ends once everyone compiled 3 times
./codexion 4 100000 50 50 50 3 0 fifo

# earliest-deadline-first arbitration with a 10 ms dongle cooldown
./codexion 5 100000 50 50 50 2 10 edf
```

### Log format

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

Timestamps are milliseconds since the start of the simulation. Every
`is compiling` line is preceded by exactly two `has taken a dongle` lines for
the same coder.

## Blocking cases handled

### Deadlock — breaking Coffman's conditions

Four conditions must hold simultaneously for a deadlock. Three of them are
inherent to the problem and cannot be removed:

- **mutual exclusion** — a dongle is physically held by one coder at a time;
- **hold and wait** — a coder holds the first dongle while waiting for the second;
- **no preemption** — a dongle is never taken back by force.

So the implementation attacks the fourth, **circular wait**, by ordering the
acquisitions asymmetrically: coders with an even id take their right dongle
first, coders with an odd id take their left one first. A cycle in the
wait-for graph would require every coder to reach for the same rotational
direction, which the asymmetry makes impossible. The classic single-coder case
is handled by the same code path: with one coder the left and right dongle are
the same object, the second acquisition can never succeed, and the coder
correctly burns out instead of hanging.

### Starvation

Each dongle keeps an ordered queue of pending requests instead of letting
whoever wakes up first win. A waiting coder proceeds only when it is at the head
of that queue, so a coder cannot be repeatedly overtaken by a luckier neighbour.

- **`fifo`** — requests are served in arrival order.
- **`edf`** — requests are served by earliest deadline, the deadline being
  `last_compile_start + time_to_burnout`. Since `time_to_burnout` is the same
  constant for everybody, ordering by deadline is equivalent to ordering by the
  timestamp of the last compilation start, which is what the code compares.
  Ties are broken deterministically in favour of the request that arrived first,
  so the policy is fully defined even when two deadlines are identical.

The deadline travels *inside the request*: a coder copies its own timestamp into
the queue entry at the moment it enqueues, under the dongle's mutex. The
scheduler therefore compares immutable values that belong to the queue, and
never dereferences another thread's coder structure.

Because a dongle sits between exactly two neighbours, at most two requests can
ever be pending on it, and the queue is sized accordingly.

### Cooldown

A released dongle records `now + dongle_cooldown` and stays unavailable until
that instant. Waiters cannot be woken into taking it early: the wait predicate
includes the cooldown check, and a waiter whose only obstacle is the cooldown
sleeps on a timed wait that expires exactly when the cooldown does, rather than
polling.

### Precise burnout detection

Burnout is the *absence* of an event: nobody signals "I have burned out", because
by definition the coder failed to do something. A monitor that only wakes on
incoming messages would therefore detect it late — or never, in the worst case
where every coder is blocked and nothing is being logged at all.

The monitor keeps, for each coder, the timestamp of their last compilation start.
On every iteration it computes the earliest deadline among all coders and then
waits on the channel **until that instant**:

- a message arrives first → it is processed, the deadline table is refreshed,
  and the wait is recomputed;
- the deadline arrives first → the wait times out, the monitor finds the coder
  whose deadline elapsed and prints `burned out` with the exact deadline value,
  not the time it happened to notice.

The wait is a `pthread_cond_timedwait` on the channel's condition variable, so
the monitor consumes no CPU while idle and still wakes at the right millisecond.
The 10 ms requirement is met by construction rather than by polling frequency.

### Log serialization

There is no mutex around `printf`, because there is nothing to protect: the
monitor is the only thread that writes to stdout. Coders produce log records as
messages; the channel serialises them; the monitor prints them one at a time in
the order it dequeues them. Two lines cannot interleave because there is only
ever one writer.

The same property gives the "nothing after `burned out`" guarantee for free: the
monitor prints the burnout line and returns, so no further line can be emitted.

### Shutdown without hanging

Stopping the simulation has to reach threads that are asleep in three different
places, and each needs its own wake-up:

- coders sleeping in `usleep` — they check the run flag at every phase boundary;
- coders blocked on a dongle's condition variable — `world_stop` broadcasts on
  every dongle after setting the flag;
- the monitor blocked on the channel — it is woken by *closing the channel*,
  which happens when the last sender is dropped.

Sender ownership is therefore released at one precise point: after every coder
thread has been joined and before the monitor is joined. At that moment all
producers are provably dead, so nothing can be sent into a closed channel, and
the monitor is still alive to observe the closure and exit. Any remaining queued
message is drained and freed before the channel is destroyed.

Lock ordering is one-directional: a coder may take a dongle mutex and then the
world mutex, so `world_stop` releases the world mutex *before* touching any
dongle mutex. Without that rule the two paths would form a classic lock-order
inversion.

## Thread synchronization mechanisms

### `pthread_mutex_t`

**One per dongle** (`t_dongle.mutex`). Protects everything about that dongle:
its `is_occupied` flag, its cooldown deadline and its request queue. Every
acquisition, release and queue operation happens inside it.

**One for the run/stop flag** (`t_world_data.world_mutex`). `is_running` is
written by whichever thread decides to stop and read by every coder several
times per cycle, so it is never touched outside `safe_world_state_check` and
`world_stop`, both of which lock it.

**One inside the channel** (`t_channel.mu`). Guards the queue of messages, the
sender count and the closed flag.

### `pthread_cond_t`

**One per dongle** (`t_dongle.state`). A coder that cannot proceed waits here
instead of spinning. It is signalled by `giveup_dongle` when a dongle is
released and broadcast by `world_stop` when the simulation ends. The wait is
guarded by a predicate loop — the dongle is free, the cooldown has expired, and
this coder is at the head of the queue — so spurious wake-ups are harmless.

**One inside the channel** (`t_channel.not_empty`). Signalled by a producer
after pushing a message, broadcast when the channel closes. This is the
condition variable the monitor sleeps on, either indefinitely (`mpsc_recv`) or
until an absolute deadline (`mpsc_recv_until`).

### The channel as a custom event mechanism

`C-Channels/` implements a small multi-producer / single-consumer channel on top
of the two primitives above. Its public surface is:

| function | role |
|---|---|
| `mpsc_new` | create channel, receiver and first sender |
| `mpsc_sender_clone` | give a new producer its own handle |
| `mpsc_send` | hand a payload to the channel; returns non-zero on failure |
| `mpsc_recv` | block until a message arrives or the channel closes |
| `mpsc_recv_until` | same, but give up at an absolute deadline |
| `mpsc_sender_drop` | release a producer; the last one closes the channel |

Ownership is explicit: on success the channel owns the payload and the consumer
frees it; on failure the payload stays with the caller, which frees it and
reports the error upwards. The channel closes on its own once the sender count
reaches zero, which is what turns "all producers are gone" into a value the
consumer can act on (`CH_CLOSED`).

`mpsc_recv_until` is what makes precise burnout detection possible: it collapses
"wake me when something happens" and "wake me at this instant" into a single
wait, so the monitor never has to choose between reacting quickly and sleeping
efficiently.

### How race conditions are prevented

**By not sharing, where possible.** The deadline table and the compile counters
are allocated once and touched only by the monitor thread. A coder's own
`last_compile_timestamp` is written and read only by that coder. Neither needs a
mutex, because neither is shared — the values reach the monitor as message
copies, not as pointers into somebody else's memory.

**By copying at the synchronisation point, where sharing is unavoidable.** The
dongle scheduler needs another coder's deadline. Rather than reading that
coder's structure, the requesting thread copies its own timestamp into the queue
entry while holding the dongle mutex. What the scheduler later compares is an
immutable snapshot living in memory the mutex already protects.

**By making the mutex the single source of order.** A dongle's occupancy, its
cooldown and its queue are all read and written under the same mutex, so a
waiter can never observe a half-updated dongle — for example a dongle marked
free whose cooldown has not been written yet.

**By having exactly one writer.** stdout is written by the monitor only, so
interleaved output is impossible without any locking on the print path.

### Thread-safe communication between coders and the monitor

A coder allocates a small record — coder id, timestamp, event type — and hands
it to the channel. From that point the record belongs to the monitor, which
prints it, updates its own tables, and frees it. No object is ever written by
one thread and read by another; ownership is transferred, not shared.

The event types are:

| message | sent when | what the monitor does |
|---|---|---|
| `MSG_TOOK_DONGLE` | a dongle has been acquired | prints the line |
| `MSG_COMPILING` | compilation starts | prints, and refreshes that coder's deadline |
| `MSG_DEBUGGING` | debugging starts | prints |
| `MSG_REFACTORING` | refactoring starts | prints |
| `MSG_COMPILE_DONE` | compilation ends | increments that coder's compile counter (not printed) |

The timestamp is taken by the coder at the instant of the event, so it reflects
when the event happened rather than when the monitor got around to it. The
distinction between `MSG_COMPILING` and `MSG_COMPILE_DONE` matters: the burnout
deadline is measured from the *start* of a compilation, while the
`number_of_compiles_required` counter is only satisfied by a *finished* one, and
using either message for both purposes would break one of the two stop
conditions.

## Technical choices

- **Message passing instead of shared state.** Sharing the deadline table would
  have meant a mutex on the hot path of every burnout check. Passing timestamps
  as messages removes both the mutex and the possibility of reading a value that
  another thread is mid-way through writing.
- **Two-slot ordered queues instead of a general priority queue.** A dongle sits
  between exactly two neighbours, so at most two requests can be pending on it.
  A heap over at most two elements degenerates to a comparison, which is what the
  code does.
- **The deadline lives in the request, not in the coder.** A coder queues on two
  different dongles, each with its own mutex; a field on the coder would have
  been protected by two different locks, which is no protection at all.
- **`gettimeofday` for timestamps.** Millisecond resolution, one clock for
  everything, and the same base the condition variables use, so a deadline can be
  handed to `pthread_cond_timedwait` without conversion between clocks.

## Resources

### Documentation and references

- POSIX manual pages: `pthread_create`, `pthread_join`, `pthread_mutex_lock`,
  `pthread_cond_wait`, `pthread_cond_timedwait`, `pthread_cond_broadcast`,
  `gettimeofday`, `usleep`.
- *The Linux Programming Interface*, Michael Kerrisk — chapters 29-30 (threads,
  mutexes, condition variables).
- *Operating Systems: Three Easy Pieces*, Arpaci-Dusseau — chapters on locks,
  condition variables and deadlock; the source of Coffman's four conditions as
  used above.
- Dijkstra's dining philosophers problem, the classic formulation of this
  resource-sharing pattern, and the standard asymmetric-ordering solution.
- Earliest Deadline First scheduling — Liu & Layland, *Scheduling Algorithms for
  Multiprogramming in a Hard-Real-Time Environment* (1973).
- Go's channel model and Rust's `std::sync::mpsc`, as design references for the
  channel abstraction, though the implementation here is written from scratch on
  top of POSIX primitives.

### Use of AI

AI (Claude, via Claude Code) was used as a reviewer and a discussion partner,
not as a code generator.

What it was used for:

- **Design discussion.** Working out why burnout detection cannot be purely
  event-driven, why the monitor needs a wait with a deadline, where sender
  ownership has to be released so that shutdown cannot hang, and why the request
  deadline belongs in the queue entry rather than in the coder structure. The
  conclusions were reached in discussion; the code implementing them is mine.
- **Code review.** Finding concrete defects and explaining them: an inverted
  burnout comparison, a mixed absolute/relative time base in the deadline check,
  a `pthread_cond_timedwait` given a deadline in the past, a use-after-free from
  releasing a sender before joining its thread, a double free from two owners
  releasing the same sender, and several missing cleanup paths.
- **Test tooling.** A fault-injection harness that makes the N-th call to
  `ft_calloc`, `pthread_create` or `usleep` fail, used to exercise the error
  paths systematically. It lives outside the submitted sources.
- **Formatting and Norm.** Mechanical conversion of indentation to tabs,
  declaration and prototype alignment, line wrapping, and `camelCase` to
  `snake_case` renaming. No logic was changed by these passes.

Everything in this repository was reviewed and is understood by the author; the
reasoning behind each design decision is written out in the sections above.
