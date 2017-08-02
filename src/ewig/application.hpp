//
// ewig - an immutable text editor
// Copyright (C) 2017 Juan Pedro Bolivar Puente
//
// This file is part of ewig.
//
// ewig is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ewig is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ewig.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <ewig/keys.hpp>
#include <ewig/buffer.hpp>
#include <ewig/store.hpp>

#include <ctime>
#include <variant>

namespace ewig {

struct terminal_action
{
    key_code key;
    coord size;
};

using action = std::variant<
    terminal_action,
    buffer_action>;

struct message
{
    std::time_t time_stamp;
    immer::box<std::string> content;
};

struct application
{
    buffer current;
    key_map keys;
    key_seq input;
    immer::vector<text> clipboard;
    immer::vector<message> messages;
};

using command = std::function<result<application, action>(application, coord)>;

application paste(application app, coord size);
application put_message(application state, std::string str);
application put_clipboard(application state, text content);
application clear_input(application state);

result<application, action> save(application app, coord);
result<application, action> eval_command(application state,
                                        const std::string& cmd,
                                        coord editor_size);
result<application, action> update(application state, action ev);

application apply_edit(application state, coord size, buffer edit);
application apply_edit(application state, coord size, std::pair<buffer, text> edit);

template <typename Fn>
command edit_command(Fn fn)
{
    return [=] (application state, coord size) {
        return apply_edit(state, size, fn(state.current));
    };
}

template <typename Fn>
command paste_command(Fn fn)
{
    return [=] (application state, coord size) {
        return apply_edit(state, size, fn(state.current, state.clipboard.back()));
    };
}

template <typename Fn>
command scroll_command(Fn fn)
{
    return [=] (application state, coord size) {
        state.current = fn(state.current, editor_size(size));
        return state;
    };
}

template <typename Fn>
command key_command(Fn fn)
{
    return [=] (application state, coord size) {
        auto key = std::get<1>(state.input.back());
        return apply_edit(state, size, fn(state.current, key));
    };
}

} // namespace ewig
