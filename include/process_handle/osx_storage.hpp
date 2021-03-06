/*
* Copyright 2017 Justas Masiulis
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef JM_PROCESS_HANDLE_STORAGE_APPLE_HPP
#define JM_PROCESS_HANDLE_STORAGE_APPLE_HPP

#include <system_error>
#include <mach/mach.h>
#include <sys/types.h>
#include <unistd.h>

namespace jm { namespace detail {

    using pid_t           = ::pid_t;
    using native_handle_t = ::mach_port_t;


    class handle_storage {
        native_handle_t _handle;

    public:
        explicit handle_storage() noexcept
                : _handle(::mach_task_self()) {}

        explicit handle_storage(native_handle_t handle) noexcept
                : _handle(handle) {}

        explicit handle_storage(pid_t pid)
        {
            const auto kr = ::task_for_pid(::mach_task_self(), pid, &_handle);
            if (kr != KERN_SUCCESS)
                throw std::system_error(std::error_code(kr, std::system_category()), "task_for_pid() failed");
        }

        explicit handle_storage(pid_t pid, std::error_code& ec) noexcept
        {
            const auto kr = ::task_for_pid(::mach_task_self(), pid, &_handle);
            if (kr != KERN_SUCCESS) {
                reset();
                ec = std::error_code(kr, std::system_category());
            }
        }

        handle_storage(const handle_storage& other) noexcept = default;
        handle_storage& operator=(const handle_storage& other) noexcept = default;

        handle_storage(handle_storage&& other) noexcept
                : _handle(other._handle)
        {
            other.reset();
        }

        handle_storage& operator=(handle_storage&& other) noexcept
        {
            _handle = other._handle;
            other.reset();
            return *this;
        }

        bool valid() const noexcept { return static_cast<bool>(_handle); }

        void reset() noexcept { _handle = static_cast<native_handle_t>(0); }

        void reset(native_handle_t new_handle) noexcept { _handle = new_handle; }

        native_handle_t native() const noexcept { return _handle; }

        pid_t pid() const
        {
            pid_t      pid;
            const auto kr = ::pid_for_task(_handle, &pid);
            if (kr != KERN_SUCCESS)
                throw std::system_error(std::error_code(kr, std::system_category()), "pid_for_task() failed");

            return pid;
        }

        pid_t pid(std::error_code& ec) const noexcept
        {
            pid_t      pid;
            const auto kr = ::pid_for_task(_handle, &pid);
            if (kr != KERN_SUCCESS)
                ec = ec = std::error_code(kr, std::system_category());

            return pid;
        }
    }; // handle_storage

}} // namespace jm::detail

#endif // include guard
