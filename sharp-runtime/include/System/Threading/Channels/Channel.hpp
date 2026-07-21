// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Threading/Channels/ChannelClosedException.hpp"
#include "System/Threading/Channels/ChannelOptions.hpp"
#include "System/Threading/Tasks/Task.hpp"

namespace System::Threading::Channels {

    /**
     * @brief Provides a base class for reading from a channel.
     *
     * C++ counterpart of .NET System.Threading.Channels.ChannelReader<T>.
     *
     * @note No `CancellationToken`/`IAsyncEnumerable<T>` support (`ReadAllAsync` isn't
     * reproduced) — this runtime's Task type has no async-enumerable idiom; drain a channel with
     * a `while (WaitToReadAsync) { while (TryRead) { ... } }` loop instead, matching
     * `ReadAllAsync`'s own documented implementation.
     *
     * @note Inherits `std::enable_shared_from_this` so the default `ReadAsync()` can keep `this`
     * alive for the lifetime of its background task (see that method's doc comment) — instances
     * must therefore be held via `std::shared_ptr` (true of every factory in this file, and of
     * any custom subclass expected to use the default `ReadAsync()`).
     */
    template<typename T>
    class ChannelReader : public std::enable_shared_from_this<ChannelReader<T>> {
    public:
        virtual ~ChannelReader() = default;

        /** @brief Attempts to read an item from the channel without blocking. */
        virtual bool TryRead(T& item) = 0;

        /** @brief Attempts to peek at the next item without removing it. Returns false if unsupported. */
        virtual bool TryPeek(T& /*item*/) { return false; }

        /** @return true if TryPeek() is supported by this instance. */
        [[nodiscard]] virtual bool getCanPeekProperty() const { return false; }

        /** @return true if getCountProperty() is supported by this instance. */
        [[nodiscard]] virtual bool getCanCountProperty() const { return false; }

        /** @return The current number of items available. @throws System::NotSupportedException if unsupported. */
        [[nodiscard]] virtual SharpRuntime::intcs getCountProperty() const {
            throw System::NotSupportedException("Counting is not supported on this channel reader.");
        }

        /** @brief Completes with true when data is available to read, or false when the channel is closed and drained. */
        virtual System::Threading::Tasks::TaskT<bool> WaitToReadAsync() = 0;

        /** @brief A Task that completes once the channel is closed and fully drained. */
        [[nodiscard]] virtual System::Threading::Tasks::Task getCompletionProperty() const = 0;

        /**
         * @brief Reads an item, waiting for one to become available if necessary.
         *
         * The returned Task runs on a background thread (see Task's constructor) that may
         * still be executing after this call returns; it keeps `this` alive via
         * `shared_from_this()` for its own duration, so it's safe to call even if every other
         * reference to this reader is dropped immediately after issuing the call.
         */
        virtual System::Threading::Tasks::TaskT<T> ReadAsync() {
            auto self = this->shared_from_this();
            return System::Threading::Tasks::TaskT<T>([self]() {
                T item{};
                while (true) {
                    if (self->TryRead(item)) return item;
                    if (!self->WaitToReadAsync().getResultProperty()) {
                        throw ChannelClosedException();
                    }
                }
            });
        }
    };

    /**
     * @brief Provides a base class for writing to a channel.
     *
     * C++ counterpart of .NET System.Threading.Channels.ChannelWriter<T>.
     *
     * @note Inherits `std::enable_shared_from_this` so the default `WriteAsync()` can keep
     * `this` alive for the lifetime of its background task (see that method's doc comment) —
     * instances must therefore be held via `std::shared_ptr` (true of every factory in this
     * file, and of any custom subclass expected to use the default `WriteAsync()`).
     */
    template<typename T>
    class ChannelWriter : public std::enable_shared_from_this<ChannelWriter<T>> {
    public:
        virtual ~ChannelWriter() = default;

        /** @brief Attempts to write an item to the channel without blocking. */
        virtual bool TryWrite(const T& item) = 0;

        /** @brief Completes with true when space is available to write, or false when the channel will accept no more writes. */
        virtual System::Threading::Tasks::TaskT<bool> WaitToWriteAsync() = 0;

        /** @brief Attempts to mark the channel complete. @return false if already completed. */
        virtual bool TryComplete(std::exception_ptr error = nullptr) {
            (void)error;
            return false;
        }

        /** @brief Marks the channel complete. @throws ChannelClosedException if already completed. */
        void Complete(std::exception_ptr error = nullptr) {
            if (!TryComplete(error)) {
                throw ChannelClosedException("The channel has already been completed.");
            }
        }

        /**
         * @brief Writes an item, waiting for space if necessary.
         *
         * The returned Task runs on a background thread (see Task's constructor) that may
         * still be executing after this call returns; it keeps `this` alive via
         * `shared_from_this()` for its own duration, so it's safe to call even if every other
         * reference to this writer is dropped immediately after issuing the call.
         */
        virtual System::Threading::Tasks::Task WriteAsync(T item) {
            auto self = this->shared_from_this();
            return System::Threading::Tasks::Task([self, item = std::move(item)]() mutable {
                while (!self->TryWrite(item)) {
                    if (!self->WaitToWriteAsync().getResultProperty()) {
                        throw ChannelClosedException();
                    }
                }
            });
        }
    };

    namespace detail {

        template<typename T>
        struct ChannelState {
            std::mutex mutex;
            std::condition_variable notEmpty;
            std::condition_variable notFull;
            std::deque<T> queue;
            bool closed = false;
            std::exception_ptr closeError;
            SharpRuntime::intcs capacity = -1; // -1 == unbounded
            BoundedChannelFullMode fullMode = BoundedChannelFullMode::Wait;

            // Verified against BoundedChannel.cs's TryWrite: a configured capacity of 0 (a
            // documented legal "rendezvous channel") is handled there by a queue-transition-from-
            // 0-to-1 special case that still buffers one item when no reader is synchronously
            // blocked waiting -- i.e. a capacity-0 channel is observably equivalent to a
            // capacity-1 channel for every publicly-visible TryWrite/WaitToWriteAsync outcome (the
            // only difference is an internal direct-handoff-to-a-blocked-reader optimization that
            // doesn't change any return value this port's simpler mutex/condvar model produces).
            // Previously this port used `capacity` directly in its "is full" comparisons, so a
            // capacity-0 channel had queue.size() (0) >= capacity (0) true immediately -- every
            // write blocked forever, even with a reader concurrently waiting to receive.
            [[nodiscard]] SharpRuntime::intcs effectiveCapacity() const { return capacity == 0 ? 1 : capacity; }
        };

        template<typename T>
        class ChannelReaderImpl : public ChannelReader<T> {
            std::shared_ptr<ChannelState<T>> state_;

        public:
            explicit ChannelReaderImpl(std::shared_ptr<ChannelState<T>> state) : state_(std::move(state)) {}

            bool TryRead(T& item) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->queue.empty()) return false;
                item = std::move(state_->queue.front());
                state_->queue.pop_front();
                // notify_all(), not notify_one(): multiple writers can be concurrently blocked
                // in WaitToWriteAsync on a full bounded channel, and freeing one slot doesn't
                // mean only one of them should get a chance to re-check -- notify_one() would
                // wake at most one specific waiter (implementation-defined which), permanently
                // starving the others if no further read/complete ever happens to notify them
                // again. Matches the identical notEmpty fix below (TryWrite) and real .NET's
                // WaitToWriteAsync contract of notifying every currently-pending registration on
                // any relevant state change, not just one. Waking extra threads that re-check
                // their predicate and safely go back to sleep is always correct, just a few
                // futile wakeups -- never a correctness risk.
                state_->notFull.notify_all();
                // Also wake any Completion waiter: it's blocked on notEmpty until closed &&
                // queue.empty(), and draining the last item is exactly the transition it's
                // waiting for.
                state_->notEmpty.notify_all();
                return true;
            }

            [[nodiscard]] bool getCanCountProperty() const override { return true; }

            [[nodiscard]] SharpRuntime::intcs getCountProperty() const override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                return static_cast<SharpRuntime::intcs>(state_->queue.size());
            }

            // Verified against UnboundedChannel.cs's WaitToReadAsync: when there are no items
            // left and writing is done, the returned task faults with the completion exception
            // (if any) rather than resolving to false. This previously always returned false
            // once closed, silently discarding a real completion error -- it was only
            // observable via the separate getCompletionProperty() task, not through the
            // primary WaitToReadAsync/ReadAsync read path.
            System::Threading::Tasks::TaskT<bool> WaitToReadAsync() override {
                auto state = state_;
                return System::Threading::Tasks::TaskT<bool>([state]() -> bool {
                    std::unique_lock<std::mutex> lock(state->mutex);
                    state->notEmpty.wait(lock, [&] { return !state->queue.empty() || state->closed; });
                    if (!state->queue.empty()) return true;
                    if (state->closeError) std::rethrow_exception(state->closeError);
                    return false;
                });
            }

            [[nodiscard]] System::Threading::Tasks::Task getCompletionProperty() const override {
                auto state = state_;
                return System::Threading::Tasks::Task([state]() {
                    std::unique_lock<std::mutex> lock(state->mutex);
                    state->notEmpty.wait(lock, [&] { return state->closed && state->queue.empty(); });
                    if (state->closeError) std::rethrow_exception(state->closeError);
                });
            }
        };

        template<typename T>
        class ChannelWriterImpl : public ChannelWriter<T> {
            std::shared_ptr<ChannelState<T>> state_;

        public:
            explicit ChannelWriterImpl(std::shared_ptr<ChannelState<T>> state) : state_(std::move(state)) {}

            bool TryWrite(const T& item) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->closed) return false;

                if (state_->capacity >= 0 && static_cast<SharpRuntime::intcs>(state_->queue.size()) >= state_->effectiveCapacity()) {
                    switch (state_->fullMode) {
                        case BoundedChannelFullMode::Wait:
                            return false;
                        case BoundedChannelFullMode::DropWrite:
                            return true; // item is dropped, but the write is considered handled
                        case BoundedChannelFullMode::DropNewest:
                            if (!state_->queue.empty()) state_->queue.pop_back();
                            break;
                        case BoundedChannelFullMode::DropOldest:
                            if (!state_->queue.empty()) state_->queue.pop_front();
                            break;
                    }
                }

                state_->queue.push_back(item);
                // notify_all(), not notify_one(): multiple readers can be concurrently blocked
                // in WaitToReadAsync/ReadAsync on an empty channel (confirmed via a standalone
                // repro, 2026-07-14: two readers blocked, one TryWrite, one reader hangs forever
                // since notify_one() only ever wakes one of them and no further write/complete
                // ever comes along to notify the other). Real .NET's WaitToReadAsync contract
                // ("completes with true when data is available to read") notifies every
                // currently-pending registration on a write, not just one -- readers that lose
                // the race for the single new item simply re-check TryRead, see it's gone, and
                // go back to waiting, which is always safe.
                state_->notEmpty.notify_all();
                return true;
            }

            // Verified against UnboundedChannel.cs's WaitToWriteAsync: once the channel is
            // closed, the returned task faults with the completion exception (if any) rather
            // than resolving to false. Same rationale as WaitToReadAsync's fix above.
            System::Threading::Tasks::TaskT<bool> WaitToWriteAsync() override {
                auto state = state_;
                return System::Threading::Tasks::TaskT<bool>([state]() -> bool {
                    std::unique_lock<std::mutex> lock(state->mutex);
                    state->notFull.wait(lock, [&] {
                        return state->closed || state->capacity < 0 ||
                               static_cast<SharpRuntime::intcs>(state->queue.size()) < state->effectiveCapacity() ||
                               state->fullMode != BoundedChannelFullMode::Wait;
                    });
                    if (!state->closed) return true;
                    if (state->closeError) std::rethrow_exception(state->closeError);
                    return false;
                });
            }

            bool TryComplete(std::exception_ptr error) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->closed) return false;
                state_->closed = true;
                state_->closeError = error;
                state_->notEmpty.notify_all();
                state_->notFull.notify_all();
                return true;
            }
        };

        // Backing state/reader/writer for Channel<T>::CreateUnboundedPrioritized(), kept
        // deliberately separate from ChannelState<T>/ChannelReaderImpl<T>/ChannelWriterImpl<T>
        // above rather than generalizing those over a swappable container: the FIFO types'
        // bounded-specific fields (capacity, fullMode/drop policies) don't apply to an unbounded
        // priority queue, and keeping this fully separate means the existing, tested FIFO channel
        // code path is untouched by this addition. Verified against real .NET's
        // UnboundedPrioritizedChannel<T> (backed by PriorityQueue<bool, T>) -- see
        // ChannelOptions.hpp's UnboundedPrioritizedChannelOptions<T> doc-comment.
        template<typename T>
        struct PrioritizedChannelState {
            std::mutex mutex;
            std::condition_variable notEmpty;
            // std::multiset dequeues its smallest element first under `comparer` (ascending
            // order), matching real .NET's default Comparer<T>.Default (min-first) semantics.
            std::multiset<T, std::function<bool(const T&, const T&)>> items;
            bool closed = false;
            std::exception_ptr closeError;

            explicit PrioritizedChannelState(std::function<bool(const T&, const T&)> comparer)
                : items(std::move(comparer)) {}
        };

        template<typename T>
        class PrioritizedChannelReaderImpl : public ChannelReader<T> {
            std::shared_ptr<PrioritizedChannelState<T>> state_;

        public:
            explicit PrioritizedChannelReaderImpl(std::shared_ptr<PrioritizedChannelState<T>> state)
                : state_(std::move(state)) {}

            bool TryRead(T& item) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->items.empty()) return false;
                auto node = state_->items.extract(state_->items.begin());
                item = std::move(node.value());
                // Wake any Completion waiter: it's blocked on notEmpty until closed &&
                // items.empty(), and draining the last item is exactly that transition. Missing
                // this call is a genuine lost-wakeup bug -- confirmed via a real hang under
                // ThreadSanitizer (2026-07-14): PrioritizedChannelTests.
                // Completion_CompletesOnceClosedAndDrained deadlocked reliably under TSan's
                // instrumented scheduling, though it happened to pass under normal execution
                // because the Completion task's background thread usually didn't start checking
                // its predicate until after TryRead had already run -- a timing coincidence, not
                // a correctness guarantee. Matches the identical, already-correct pattern in the
                // sibling FIFO ChannelReaderImpl::TryRead just above in this file.
                state_->notEmpty.notify_all();
                return true;
            }

            bool TryPeek(T& item) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->items.empty()) return false;
                item = *state_->items.begin();
                return true;
            }

            [[nodiscard]] bool getCanPeekProperty() const override { return true; }
            [[nodiscard]] bool getCanCountProperty() const override { return true; }

            [[nodiscard]] SharpRuntime::intcs getCountProperty() const override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                return static_cast<SharpRuntime::intcs>(state_->items.size());
            }

            System::Threading::Tasks::TaskT<bool> WaitToReadAsync() override {
                auto state = state_;
                return System::Threading::Tasks::TaskT<bool>([state]() -> bool {
                    std::unique_lock<std::mutex> lock(state->mutex);
                    state->notEmpty.wait(lock, [&] { return !state->items.empty() || state->closed; });
                    if (!state->items.empty()) return true;
                    if (state->closeError) std::rethrow_exception(state->closeError);
                    return false;
                });
            }

            [[nodiscard]] System::Threading::Tasks::Task getCompletionProperty() const override {
                auto state = state_;
                return System::Threading::Tasks::Task([state]() {
                    std::unique_lock<std::mutex> lock(state->mutex);
                    state->notEmpty.wait(lock, [&] { return state->closed && state->items.empty(); });
                    if (state->closeError) std::rethrow_exception(state->closeError);
                });
            }
        };

        template<typename T>
        class PrioritizedChannelWriterImpl : public ChannelWriter<T> {
            std::shared_ptr<PrioritizedChannelState<T>> state_;

        public:
            explicit PrioritizedChannelWriterImpl(std::shared_ptr<PrioritizedChannelState<T>> state)
                : state_(std::move(state)) {}

            bool TryWrite(const T& item) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->closed) return false;
                state_->items.insert(item);
                // notify_all(), not notify_one() -- same lost-wakeup class as the FIFO
                // ChannelWriterImpl::TryWrite fix above (confirmed via the same kind of
                // standalone repro: 2026-07-14), and for the identical reason: multiple readers
                // can be concurrently blocked in WaitToReadAsync/ReadAsync, and notify_one()
                // would only ever wake one of them.
                state_->notEmpty.notify_all();
                return true;
            }

            // Unbounded: writing can always proceed unless the channel is closed -- matches real
            // .NET's own comment on WaitToWriteAsync ("unbounded writing can always be done if
            // we haven't completed").
            System::Threading::Tasks::TaskT<bool> WaitToWriteAsync() override {
                auto state = state_;
                return System::Threading::Tasks::TaskT<bool>([state]() -> bool {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    if (!state->closed) return true;
                    if (state->closeError) std::rethrow_exception(state->closeError);
                    return false;
                });
            }

            bool TryComplete(std::exception_ptr error) override {
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->closed) return false;
                state_->closed = true;
                state_->closeError = error;
                state_->notEmpty.notify_all();
                return true;
            }
        };

    } // namespace detail

    /**
     * @brief Provides static methods for creating channels.
     *
     * C++ counterpart of .NET System.Threading.Channels.Channel (the static factory) — this
     * class's instances model .NET's `Channel<T>` (the Reader/Writer pair), since C++ has no
     * separate non-generic/generic overload of the same type name the way C# does.
     *
     * @note Backed by `std::mutex`/`std::condition_variable`, not a lock-free algorithm — simpler
     * and adequate for typical game producer/consumer workloads, but not tuned for extreme
     * contention the way .NET's real implementation is.
     */
    template<typename T>
    class Channel {
    public:
        std::shared_ptr<ChannelReader<T>> Reader;
        std::shared_ptr<ChannelWriter<T>> Writer;

        /** @brief Creates an unbounded channel. */
        [[nodiscard]] static Channel<T> CreateUnbounded(const UnboundedChannelOptions& = UnboundedChannelOptions()) {
            auto state = std::make_shared<detail::ChannelState<T>>();
            Channel<T> channel;
            channel.Reader = std::make_shared<detail::ChannelReaderImpl<T>>(state);
            channel.Writer = std::make_shared<detail::ChannelWriterImpl<T>>(state);
            return channel;
        }

        /** @brief Creates a bounded channel with the given capacity and BoundedChannelFullMode::Wait. */
        [[nodiscard]] static Channel<T> CreateBounded(SharpRuntime::intcs capacity) {
            return CreateBounded(BoundedChannelOptions(capacity));
        }

        /** @brief Creates a bounded channel with the given options. */
        [[nodiscard]] static Channel<T> CreateBounded(const BoundedChannelOptions& options) {
            auto state = std::make_shared<detail::ChannelState<T>>();
            state->capacity = options.getCapacityProperty();
            state->fullMode = options.FullMode;
            Channel<T> channel;
            channel.Reader = std::make_shared<detail::ChannelReaderImpl<T>>(state);
            channel.Writer = std::make_shared<detail::ChannelWriterImpl<T>>(state);
            return channel;
        }

        /**
         * @brief Creates an unbounded channel that dequeues items in priority order rather than
         * insertion order.
         *
         * C++ counterpart of .NET Channel.CreateUnboundedPrioritized<T>(options). Items dequeue
         * in ascending order per @p options's Comparer (the smallest item first); if @p options
         * doesn't supply a Comparer, `operator<` is used, matching
         * UnboundedPrioritizedChannelOptions<T>::Comparer's own documented default. Writing never
         * blocks (unbounded) and always succeeds until the channel is completed, exactly like
         * CreateUnbounded().
         */
        [[nodiscard]] static Channel<T> CreateUnboundedPrioritized(
            const UnboundedPrioritizedChannelOptions<T>& options = UnboundedPrioritizedChannelOptions<T>()) {
            std::function<bool(const T&, const T&)> comparer = options.Comparer
                ? options.Comparer
                : std::function<bool(const T&, const T&)>([](const T& a, const T& b) { return a < b; });
            auto state = std::make_shared<detail::PrioritizedChannelState<T>>(std::move(comparer));
            Channel<T> channel;
            channel.Reader = std::make_shared<detail::PrioritizedChannelReaderImpl<T>>(state);
            channel.Writer = std::make_shared<detail::PrioritizedChannelWriterImpl<T>>(state);
            return channel;
        }
    };

} // namespace System::Threading::Channels
